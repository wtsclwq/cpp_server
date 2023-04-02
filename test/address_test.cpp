//
// Created by wtsclwq on 23-4-1.
//
#include "../src/include/socket/address.h"

#include "../src/include/log/log_manager.h"
#include "../src/include/socket/ip_address.h"
#include "../src/include/socket/ipv4_address.h"
#include "../src/include/socket/ipv6_address.h"
#include "../src/include/socket/unix_address.h"
#include "../src/include/socket/unknow_address.h"

wtsclwq::Logger::ptr logger = ROOT_LOGGER;

void TestHost() {
    std::vector<wtsclwq::Address::ptr> addrs;
    auto v = wtsclwq::Address::Lookup(addrs, "www.baidu.com:ftp");
    if (!v) {
        LOG_ERROR(logger, "lookup error");
        return;
    }

    for (auto i = 0; i < addrs.size(); ++i) {
        LOG_CUSTOM_INFO(logger, "%d -- %s", i, addrs[i]->ToString().c_str())
    }
}

void TestInterface() {
    std::multimap<std::string, std::pair<wtsclwq::Address::ptr, uint32_t>>
        results;
    auto res = wtsclwq::Address::GetAllInterFaceAddress(results, AF_UNSPEC);
    if (!res) {
        LOG_ERROR(logger, "TestInterface error");
    }
    for (auto &i : results) {
        LOG_CUSTOM_INFO(logger, "name = %s, addr = %s, prefix_len = %u",
                        i.first.c_str(), i.second.first->ToString().c_str(),
                        i.second.second);
    }
}

void TestIpv4() {
    auto addr = wtsclwq::IPAddress::Create("127.0.0.9");
    if (addr) {
        LOG_INFO(logger, addr->ToString().c_str());
    }
}
int main() { TestIpv4(); }
