#include <qsox/Util.hpp>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>

namespace qsox {

bool systemSupportsIp(bool v6) {
    return true;
    // struct ifaddrs* ifaddr, *ifa;

    // auto needed = v6 ? AF_INET6 : AF_INET;

    // if (getifaddrs(&ifaddr) == -1) {
    //     assert(false && "getifaddrs failed");
    //     return false;
    // }

    // for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    //     if (ifa->ifa_addr == nullptr) {
    //         continue;
    //     }

    //     if (ifa->ifa_addr->sa_family != needed) {
    //         continue;
    //     }


    // }
}

} // namespace qsox