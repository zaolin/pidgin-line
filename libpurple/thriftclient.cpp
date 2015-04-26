#include <account.h>
#include <connection.h>

#include <thrift/protocol/TCompactProtocol.h>

#include "constants.hpp"
#include "thriftclient.hpp"

ThriftClient::ThriftClient(PurpleAccount *acct, PurpleConnection *conn, std::string url)
    : line::TalkServiceClient(
        boost::make_shared<apache::thrift::protocol::TCompactProtocol>(
            boost::make_shared<LineHttpTransport>(acct, conn, url)))
{
    http = boost::static_pointer_cast<LineHttpTransport>(getInputProtocol()->getTransport());
}

void ThriftClient::set_url(std::string url) {
    http->set_url(url);
}

void ThriftClient::set_timeout(int timeout) {
    http->set_timeout(timeout);
}

void ThriftClient::set_connection_limit(int limit) {
    http->set_connection_limit(limit);
}

void ThriftClient::send(std::function<void()> callback) {
    http->open();
    http->request(callback);
}

int ThriftClient::status_code() {
    return http->status_code();
}

void ThriftClient::close() {
    http->close();
}

// Required for the single set<Contact> in the interface

bool line::Contact::operator<(const Contact &other) const {
    return mid < other.mid;
}
