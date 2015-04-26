#pragma once

#include <string>
#include <functional>
#include <list>

#include <account.h>
#include <connection.h>
#include <util.h>

#include "http.h"

enum class HTTPFlag {
    NONE =  0,
    AUTH =  1 << 0,
};

inline constexpr HTTPFlag operator|(HTTPFlag a, HTTPFlag b) {
    return (HTTPFlag)((int)a | (int)b);
};

inline constexpr bool operator&(HTTPFlag a, HTTPFlag b) {
    return ((int)a & (int)b) != 0;
}

class HTTPClient {
    PurpleAccount *acct;
    PurpleConnection *conn;

    PurpleHttpConnectionSet *connection_set;
    PurpleHttpKeepalivePool *keepalive_pool;

    static void purple_cb(PurpleHttpConnection *http_conn, PurpleHttpResponse *response,
        gpointer user_data);

public:
    using CompleteFunc = std::function<void(int, const gchar *, gsize)>;

    HTTPClient(PurpleAccount *acct, PurpleConnection *conn);
    ~HTTPClient();

    void request(std::string url, CompleteFunc callback);
    void request(std::string url, HTTPFlag flags, CompleteFunc callback);
    void request(std::string url, HTTPFlag flags,
        std::string content_type, std::string body,
        CompleteFunc callback);

};
