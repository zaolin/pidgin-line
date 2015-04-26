#pragma once

#include <string>
#include <deque>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <debug.h>
#include <plugin.h>
#include <prpl.h>

#include "thrift_line/TalkService.h"

#include "linehttptransport.hpp"

class ThriftClient : public line::TalkServiceClient {

    boost::shared_ptr<LineHttpTransport> http;

public:

    ThriftClient(PurpleAccount *acct, PurpleConnection *conn, std::string path);

    void set_url(std::string url);
    void set_timeout(int timeout);
    void set_connection_limit(int limit);

    void send(std::function<void()> callback);

    int status_code();
    void close();

};
