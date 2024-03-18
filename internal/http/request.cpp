#include "request.hpp"

Request::Request() : method_(kMethodUnknown), request_target_("/"), http_version_("HTTP/1.1"), headers_(), body_() {}

Request::Request(HttpMethod method,
                 const std::string &request_target,
                 const std::string &http_version,
                 const std::map<std::string, std::string> &headers,
                 const std::string &body)
    : method_(method), request_target_(request_target), http_version_(http_version), headers_(headers), body_(body) {}

Request::Request(const Request &other)
    : method_(other.method_), request_target_(other.request_target_), http_version_(other.http_version_), headers_(other.headers_), body_(other.body_) {}

Request &Request::operator=(const Request &other) {
    if (this != &other) {
        method_ = other.method_;
        request_target_ = other.request_target_;
        http_version_ = other.http_version_;
        headers_ = other.headers_;
        body_ = other.body_;
    }
    return *this;
}

HttpMethod Request::method() const {
    return method_;
}

const std::string &Request::path() const {
    // TODO: implement
    return request_target_;
}

Option<std::string> Request::query(const std::string &key) const {
    // TODO: implement
    return None;
}

const std::string &Request::httpVersion() const {
    return http_version_;
}

// Returns nullptr when key is not found.
Option<std::string> Request::header(const std::string &key) const {
    std::map<std::string, std::string>::const_iterator value = headers_.find(key);

    if (value == headers_.end())
        return None;
    return Some(value->second);
}

const std::string &Request::body() const {
    return body_;
}
