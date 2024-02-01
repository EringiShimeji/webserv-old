#include "config.hpp"

Config::Config() : client_max_body_size_(kDefaultClientMaxBodySize) {
    ParseConfigFile(kDefaultPath);
}

Config::Config(const std::string &path) : client_max_body_size_(kDefaultClientMaxBodySize) {
    ParseConfigFile(path);
}

Config::Config(
        const std::vector<VirtualServerConfig> &virtual_servers,
        const std::map<HttpStatusCode, std::string> &error_pages,
        unsigned int client_max_body_size)
    : client_max_body_size_(client_max_body_size),
      virtual_servers_(virtual_servers),
      error_pages_(error_pages) {}

Config::~Config() {}

Config::Config(const Config &other)
    : client_max_body_size_(other.client_max_body_size_),
      virtual_servers_(other.virtual_servers_),
      error_pages_(other.error_pages_) {}

Config &Config::operator=(const Config &other) {
    if (this != &other) {
        client_max_body_size_ = other.client_max_body_size_;
        virtual_servers_ = other.virtual_servers_;
        error_pages_ = other.error_pages_;
    }
    return *this;
}

void Config::ParseConfigFile(const std::string &path) {}

// Set a default error page like nginx
// Do nothing if error page is already set
void Config::SetDefaultErrorPage(HttpStatusCode code) {
    if (error_pages_.find(code) == error_pages_.end()) {
        error_pages_[code] = "";
    }
}

/* getters */
unsigned int Config::GetClientMaxBodySize() const {
    return client_max_body_size_;
}

const std::vector<VirtualServerConfig> &Config::GetVirtualServers() const {
    return virtual_servers_;
}

// If an error page is not set, generate a default one and store it to reduce resource usage
const std::string &Config::GetErrorPage(HttpStatusCode code) {
    if (error_pages_.find(code) != error_pages_.end()) {
        return error_pages_.at(code);
    }
    SetDefaultErrorPage(code);
    return error_pages_.at(code);
}
