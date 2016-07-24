#pragma once


namespace net {
namespace ip {


    address::address(ip::protocol p, const char* s) {
        addresses(p, s, [this](address a) { (*this) = a; });
    }


    //--------------------------------------------------------------------------

    bool
    addresses(
        ip::protocol p,
        const char* s,
        void* context,
        void (*callback)(void*, ip::address)
    ) {
        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family   = AF_INET;
        hints.ai_socktype = p;
        hints.ai_flags    = AI_PASSIVE | AI_CANONNAME;

        addrinfo* infos = nullptr;
        const int gotaddrinfo = ::getaddrinfo(s, nullptr, &hints, &infos);
        if (not ~gotaddrinfo) {
            if (infos) freeaddrinfo(infos);
            return false;
        }

        for (auto info = infos; info; info = info->ai_next) {
            const sockaddr_in* const sa = (sockaddr_in*)info->ai_addr;
            address a;
            a.host = ntohl(sa->sin_addr.s_addr);
            a.port = ntohs(sa->sin_port);
            callback(context, a);
        }

        freeaddrinfo(infos);
        return true;
    }


}} // namespace net::ip