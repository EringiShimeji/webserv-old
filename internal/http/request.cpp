#include "request.hpp"

Request::Request() : method_(kMethodUnknown) {}

Request::Request(const std::string &raw_request) : method_(kMethodUnknown) {
    parseRawRequest(raw_request);
}

Request::Request(const Request &request) : method_(kMethodUnknown) {
    *this = request;
}

Request::~Request() {
}

Request &Request::operator=(const Request &other) {
    if (this != &other) {
        method_ = other.method_;
        route_path_ = other.route_path_;
        headers_ = other.headers_;
        body_ = other.body_;
    }
    return *this;
}

HttpMethod Request::method() const {
    return method_;
}

// Returns requested URI without queries.
const std::string &Request::routePath() const {
    return route_path_;
}

Option<std::string> Request::query(const std::string &key) const {
    std::map<std::string, std::string>::const_iterator value = queries_.find(key);

    if (value == queries_.end())
        return None;
    return Some(value->second);
}

// Returns nullptr when key is not found.
Option<std::string> Request::header(const std::string &key) const {
    std::map<std::string, std::string>::const_iterator value = headers_.find(key);

    if (value == headers_.end())
        return None;
    return Some(value->second);
}

const std::string &Request::text() const {
    return body_;
}