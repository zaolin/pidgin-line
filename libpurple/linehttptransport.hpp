#pragma once

#include <functional>
#include <string>
#include <sstream>
#include <queue>

#include <stdint.h>

#include <account.h>
#include <http.h>
#include <sslconn.h>

#include <thrift/transport/TTransport.h>

#include "wrapper.hpp"

class LineHttpTransport : public apache::thrift::transport::TTransport {

    struct Request {
        LineHttpTransport *transport;
        std::function<void()> callback;
    };

    bool destroying;

    PurpleAccount *acct;
    PurpleConnection *conn;

    guint connection_limit;
    int timeout;

    std::string url;
    std::string x_ls;

    PurpleHttpConnectionSet *connection_set;
    PurpleHttpKeepalivePool *keepalive_pool;

    std::stringbuf request_buf;

    std::stringbuf response_buf;
    int status_code_;

    static void purple_cb(PurpleHttpConnection *http_conn, PurpleHttpResponse *response,
        gpointer user_data);

    void handle_response(PurpleHttpResponse *response, Request *req);

public:

    LineHttpTransport(PurpleAccount *acct, PurpleConnection *conn, std::string url);
    ~LineHttpTransport();

    void set_url(std::string url);
    void set_timeout(int timeout);
    void set_connection_limit(int limit);

    virtual void open();
    virtual void close();

    virtual uint32_t read_virt(uint8_t *buf, uint32_t len);
    void write_virt(const uint8_t *buf, uint32_t len);

    void request(std::function<void()> callback);
    int status_code();
};
