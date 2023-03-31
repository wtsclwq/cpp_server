//
// Created by wtsclwq on 23-3-30.
//

#include "../include/socket/ip_address.h"

#include <libnet.h>

#include "../include/log/log_manager.h"

namespace wtsclwq {
static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

auto IPAddress::Create(const char *address, uint16_t port) -> IPAddress::ptr {
    addrinfo hints{};
    addrinfo *results;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;

    int error = getaddrinfo(address, nullptr, &hints, &results);
    if (error != 0) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "IPAddress::Create(%s, %d) error = %d, errstr = %s",
                         address, port, error, strerror(errno));
        return nullptr;
    }
    try {
        IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
            Address::Create(results->ai_addr, results->ai_addrlen));
        if (result != nullptr) {
            result->SetPort(port);
        }
        freeaddrinfo(results);
        return result;
    } catch (...) {
        freeaddrinfo(results);
        return nullptr;
    }
}

auto IPAddress::LookupAnyAddress(const std::string &host, int family, int type,
                                 int protocol) -> IPAddress::ptr {
    std::vector<Address::ptr> result;
    if (Address::Lookup(result, host, family, type, protocol)) {
        for (auto &item : result) {
            IPAddress::ptr res = std::dynamic_pointer_cast<IPAddress>(item);
            if (res != nullptr) {
                return res;
            }
        }
    }
    return nullptr;
}
}  // namespace wtsclwq