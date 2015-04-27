#include <sstream>
#include <limits>

#include <debug.h>

#include <thrift/transport/TTransportException.h>

#include "thrift_line/TalkService.h"

#include "constants.hpp"
#include "linehttptransport.hpp"

LineHttpTransport::LineHttpTransport(PurpleAccount *acct, PurpleConnection *conn, std::string url) :
    destroying(false),
    acct(acct),
    conn(conn),
    connection_limit(1),
    timeout(30),
    url(url),
    connection_set(nullptr),
    keepalive_pool(nullptr),
    status_code_(0)
{
}

LineHttpTransport::~LineHttpTransport() {
    destroying = true;

    close();
}

void LineHttpTransport::set_url(std::string url) {
    this->url = url;
}

void LineHttpTransport::set_timeout(int timeout) {
    this->timeout = timeout;
}

void LineHttpTransport::set_connection_limit(int limit) {
    this->connection_limit = limit;

    if (keepalive_pool)
        purple_http_keepalive_pool_set_limit_per_host(keepalive_pool, limit);
}

int LineHttpTransport::status_code() {
    return status_code_;
}

void LineHttpTransport::open() {
    if (connection_set)
        return;

    connection_set = purple_http_connection_set_new();

    keepalive_pool = purple_http_keepalive_pool_new();
    purple_http_keepalive_pool_set_limit_per_host(keepalive_pool, connection_limit);
}

static gboolean destroy_http_connection_set(gpointer user_data) {
    purple_http_connection_set_destroy((PurpleHttpConnectionSet *)user_data);

    return FALSE;
}

void LineHttpTransport::close() {
    if (!connection_set)
        return;

    if (destroying) {
        purple_http_connection_set_destroy(connection_set);
    } else {
        // If close() is called within a request callback, destroying the connection set now causes a
        // double free. Destroy it later via the event loop.
        purple_timeout_add(0, destroy_http_connection_set, connection_set);
    }

    connection_set = nullptr;

    purple_http_keepalive_pool_unref(keepalive_pool);
    keepalive_pool = nullptr;

    x_ls = "";

    request_buf.str("");
    response_buf.str("");
}

uint32_t LineHttpTransport::read_virt(uint8_t *buf, uint32_t len) {
    return (uint32_t )response_buf.sgetn((char *)buf, len);
}

void LineHttpTransport::write_virt(const uint8_t *buf, uint32_t len) {
    request_buf.sputn((const char *)buf, len);
}

void LineHttpTransport::before_send_cb(PurpleHttpConnection *http_conn, guint use_count,
    gpointer user_data)
{
    RequestData *data = (RequestData *)user_data;

    purple_debug_info("line", "before_send_cb %p %d\n", data->transport->keepalive_pool, use_count);

    data->transport->before_send(purple_http_conn_get_request(http_conn), use_count == 1);
}

void LineHttpTransport::request_cb(PurpleHttpConnection *http_conn,
    PurpleHttpResponse *response, gpointer user_data)
{
    RequestData *data = (RequestData *)user_data;

    data->transport->handle_response(response, data->callback);

    delete data;
}

void LineHttpTransport::request(std::function<void()> callback) {
    RequestData *data = new RequestData();
    data->transport = this;
    data->callback = callback;

    PurpleHttpRequest *req = purple_http_request_new(url.c_str());
    purple_http_request_set_keepalive_pool(req, keepalive_pool);
    purple_http_request_set_timeout(req, timeout);

    purple_http_request_set_method(req, "POST");

    purple_http_request_set_before_send_callback(req, before_send_cb, data);

    std::string body = request_buf.str();
    purple_http_request_set_contents(req, body.c_str(), body.size());

    request_buf.str("");

    PurpleHttpConnection *http_conn = purple_http_request(conn, req, request_cb, data);

    purple_http_connection_set_add(connection_set, http_conn);
}

void LineHttpTransport::before_send(PurpleHttpRequest *req, bool first_request) {
    if (first_request) {
        x_ls = "";
    }

    if (x_ls.size()) {
        purple_debug_info("line", "before_send: setting x-ls\n");

        purple_http_request_header_set(req, "X-LS", x_ls.c_str());
    } else {
        purple_debug_info("line", "before_send: setting headers\n");

        purple_http_request_header_set(req, "x-ls", nullptr);

        purple_http_request_header_set(req, "Content-Type", "application/x-thrift");

        purple_http_request_header_set(req, "User-Agent", LINE_USER_AGENT);
        purple_http_request_header_set(req, "X-Line-Application", LINE_APPLICATION);

        const char *auth_token = purple_account_get_string(acct, LINE_ACCOUNT_AUTH_TOKEN, "");
        if (auth_token)
            purple_http_request_header_set(req, "X-Line-Access", auth_token);
    }
}

void LineHttpTransport::handle_response(PurpleHttpResponse *res, std::function<void()> callback) {
    if (destroying)
        return;

    status_code_ = purple_http_response_get_code(res);

    purple_debug_info("line", "Response: %d\n", status_code_);

    if (status_code_ == 0) {
        // Timeout or connection error

        purple_connection_error(conn, "Could not connect to LINE server.");
        return;
    }

    if (status_code_ == 403) {
        // Don't try to reconnect because this usually means the user has logged in from
        // elsewhere.

        // TODO: Check actual reason

        conn->wants_to_die = TRUE;
        purple_connection_error(conn, "Session died.");
        return;
    }

    const gchar *new_x_ls = purple_http_response_get_header(res, "X-LS");
    if (new_x_ls)
        x_ls = new_x_ls;

    response_buf.str(std::string(
        purple_http_response_get_data(res, nullptr),
        purple_http_response_get_data_len(res)));

    try {
        callback();
    } catch (line::TalkException &err) {
        std::string msg = "LINE: TalkException: ";
        msg += err.reason;

        if (err.code == line::ErrorCode::NOT_AUTHORIZED_DEVICE) {
            purple_account_remove_setting(acct, LINE_ACCOUNT_AUTH_TOKEN);

            if (err.reason == "AUTHENTICATION_DIVESTED_BY_OTHER_DEVICE") {
                msg = "LINE: You have been logged out because "
                    "you logged in from another device.";
            } else if (err.reason == "REVOKE") {
                msg = "LINE: This device was logged out via the mobile app.";
            }

            // Don't try to reconnect so we don't fight over the session with another client

            conn->wants_to_die = TRUE;
        }

        purple_connection_error(conn, msg.c_str());
        return;
    } catch (apache::thrift::TApplicationException &err) {
        std::string msg = "LINE: Application error: ";
        msg += err.what();

        purple_connection_error(conn, msg.c_str());
        return;
    } catch (apache::thrift::transport::TTransportException &err) {
        std::string msg = "LINE: Transport error: ";
        msg += err.what();

        purple_connection_error(conn, msg.c_str());
        return;
    }
}
