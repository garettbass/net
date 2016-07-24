#pragma once


namespace net {
namespace ip {


    socket
    socket::accept() const {
        sockaddr_in a; socklen_t size = sizeof(a);
        return socket(::accept(id, (sockaddr*)&a, &size));
    }


    error
    socket::bind(address a) {
        sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_port        = htons(a.port);
        sa.sin_addr.s_addr = htonl(a.host);
        return
            ok(::bind(id, (sockaddr*)&sa, sizeof(sa)))
            ? error::none()
            : error();
    }


    error
    socket::close() {
        const int32_t old_id = id;
        new(this)socket();
        return
            ok(::close(old_id))
            ? error::none()
            : error();
    }


    error
    socket::connect(address a) {
        sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_port        = htons(a.port);
        sa.sin_addr.s_addr = htonl(a.host);
        return
            ok(::connect(id, (sockaddr*)&sa, sizeof(sa)))
            ? error::none()
            : error();
    }


    error
    socket::listen() const {
        return listen(SOMAXCONN);
    }


    error
    socket::listen(int32_t backlog) const {
        return
            ok(::listen(id, backlog))
            ? error::none()
            : error();
    }


    error
    socket::open(protocol p) {
        new(this)socket(::socket(AF_INET, p, 0));
        return ok() ? error::none() : error();
    }


    #ifndef MSG_NOSIGNAL
    enum { MSG_NOSIGNAL = 0 };
    #endif


    transfer
    socket::recv(target data) const {
        const ssize_t rcvd = ::recv(id, data.head, data.size, MSG_NOSIGNAL);
        return ~rcvd ? transfer(rcvd) : transfer(error());
    }


    transfer
    socket::recvall(target data) const {
        size_t size = 0;
        while (data) {
            const transfer tx = recv(data);
            if (tx.error) return { size, tx.error };
            data.advance(tx.size);
            size += tx.size;
        }
        return transfer(size);
    }


    transfer
    socket::send(source data) const {
        const ssize_t sent = ::send(id, data.head, data.size, MSG_NOSIGNAL);
        return ~sent ? transfer(sent) : transfer(error());
    }


    transfer
    socket::sendall(source data) const {
        size_t size = 0;
        while (data) {
            const transfer tx = send(data);
            if (tx.error) return { size, tx.error };
            data.advance(tx.size);
            size += tx.size;
        }
        return transfer(size);
    }


    error
    socket::shutdown(operation o) {
        return
            ok(::shutdown(id, o))
            ? error::none()
            : error();
    }


    #ifndef SO_NOSIGPIPE
        enum { SO_NOSIGPIPE = 0x1022 };
        #define handle_SO_NOSIGPIPE(level, key) do { \
            if (level == SOL_SOCKET && key == SO_NOSIGPIPE) return 0; \
        } while(0)
    #else
        #define handle_SO_NOSIGPIPE(level, key) ((void)0)
    #endif


    error
    socket::setsockopt(int level, int key, source value) {
        handle_SO_NOSIGPIPE(level, key);
        const socklen_t size = socklen_t(value.size);
        return
            ok(::setsockopt(id, level, key, value.head, size))
            ? error::none()
            : error();
    }


    error
    socket::setsockopt(int level, int key, int value) {
        handle_SO_NOSIGPIPE(level, key);
        return setsockopt(level, key, source(value));
    }


}} // namespace net::ip