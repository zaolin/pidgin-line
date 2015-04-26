#include <sstream>
#include <string.h>

#include <debug.h>

#include "constants.hpp"
#include "httpclient.hpp"

HTTPClient::HTTPClient(PurpleAccount *acct, PurpleConnection *conn) :
    acct(acct),
    conn(conn),
    connection_set(purple_http_connection_set_new()),
    keepalive_pool(purple_http_keepalive_pool_new())
{
    purple_http_keepalive_pool_set_limit_per_host(keepalive_pool, 4);
}

HTTPClient::~HTTPClient() {
    purple_http_connection_set_destroy(connection_set);
    purple_http_keepalive_pool_unref(keepalive_pool);
}

struct Request {
    HTTPClient::CompleteFunc callback;
};

void HTTPClient::request(std::string url, HTTPClient::CompleteFunc callback) {
    request(url, HTTPFlag::NONE, callback);
}

void HTTPClient::request(std::string url, HTTPFlag flags, HTTPClient::CompleteFunc callback) {
    request(url, flags, "", "", callback);
}

void HTTPClient::request(std::string url, HTTPFlag flags,
    std::string content_type, std::string body,
    HTTPClient::CompleteFunc callback)
{
    Request *req = new Request();
    req->callback = callback;

    PurpleHttpRequest *preq = purple_http_request_new(url.c_str());
    purple_http_request_set_keepalive_pool(preq, keepalive_pool);

    // TODO: make configurable
    purple_http_request_set_timeout(preq, 3 * 60);

    purple_http_request_header_set(preq, "User-Agent", LINE_USER_AGENT);

    if (flags & HTTPFlag::AUTH) {
        purple_http_request_header_set(preq, "X-Line-Application", LINE_APPLICATION);
        purple_http_request_header_set(preq, "X-Line-Access",
            purple_account_get_string(acct, LINE_ACCOUNT_AUTH_TOKEN, ""));
    }

    if (content_type.size()) {
        purple_http_request_set_method(preq, "POST");
        purple_http_request_header_set(preq, "Content-Type", content_type.c_str());
        purple_http_request_set_contents(preq, body.c_str(), body.size());
    }

    PurpleHttpConnection *hconn = purple_http_request(conn, preq, HTTPClient::purple_cb, req);

    purple_http_connection_set_add(connection_set, hconn);
}

void HTTPClient::purple_cb(PurpleHttpConnection *http_conn, PurpleHttpResponse *response,
    gpointer user_data)
{
    Request *req = (Request *)user_data;

    req->callback(
        purple_http_response_get_code(response),
        purple_http_response_get_data(response, nullptr),
        purple_http_response_get_data_len(response));

    delete req;
}
