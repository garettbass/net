#include <algorithm>
#include <cassert>
#include <cstring>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>
#include <errno.h>
#include <net/ip.h>
#include <net/http.h>
#include "substr.h"


#if NET_COMPILER_MSVC

    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #undef DELETE
    #undef min
    #undef max
    inline int close(int socket) { return closesocket(socket); }

    #define NET_SOCKET_SYSTEM_INITIALIZATION \
        static struct NET_SOCKET_SYSTEM_INITIALIZATION { \
            NET_SOCKET_SYSTEM_INITIALIZATION() { \
                WSADATA data; \
                const int err = WSAStartup(0x202, &data); \
                assert(not err); \
                (void)err; \
            } \
           ~NET_SOCKET_SYSTEM_INITIALIZATION() { \
                const int err = WSACleanup(); \
                assert(not err); \
                (void)err; \
            } \
        } _NET_SOCKET_SYSTEM_INITIALIZATION

#else

    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <netdb.h>
    #include <unistd.h>

    #define NET_SOCKET_SYSTEM_INITIALIZATION ((void)0)

#endif


//==============================================================================


namespace net {
namespace ip {


    string gethostname() {
        string hostname(255, '\0');
        char* const out = &hostname.front();
        int const size = int(hostname.size());
        ::gethostname(out, size);
        hostname.resize(strlen(hostname.c_str()));
        return hostname;
    }


    // address =================================================================


    address::address(ip::protocol p, const char* s)
    : address() {
        addresses(p, s, [this](address addr) { (*this) = addr; return BREAK; });
    }


    std::ostream& operator<<(std::ostream& out, const ip::address& a) {
        out << int(a.a) << '.'
            << int(a.b) << '.'
            << int(a.c) << '.'
            << int(a.d);
        if (a.port) {
            out << ":" << a.port;
        }
        switch (a.protocol) {
            case ANY : out << " (ANY)"; break;
            case TCP : out << " (TCP)"; break;
            case UDP : out << " (UDP)"; break;
            default: break;
        }
        return out;
    }


    struct host {
        char addr[120] {};
        char port[8] = "http";

        host() = default;

        host(substr url) {
            if (url.seek("://")) {
                substr protocol = url.before("://");
                const size_t count = std::min(sizeof(port), protocol.length());
                strncpy(port, protocol.begin(), count);
                url = url.after("://");
            }
            if (url.seek('/')) {
                url = url.before('/');
            }
            if (url.seek(':')) {
                substr portnum = url.after(':');
                const size_t count = std::min(sizeof(port), portnum.length());
                strncpy(port, portnum.begin(), count);
                url = url.before(':');
            }
            const size_t count = std::min(sizeof(addr), url.length());
            strncpy(addr, url.begin(), count);
        }
    };


    bool
    addresses(
        ip::protocol protocol, const char* url,
        void* context, iterate (*callback)(void*, ip::address)
    ) {
        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family   = AF_INET;
        hints.ai_socktype = protocol;
        hints.ai_flags    = AI_PASSIVE | AI_CANONNAME;

        ip::host host(url);

        addrinfo* infos = nullptr;
        if (getaddrinfo(host.addr, host.port, &hints, &infos)) {
            if (infos) freeaddrinfo(infos);
            return false;
        }

        for (auto info = infos; info; info = info->ai_next) {
            const sockaddr_in* const sa = (sockaddr_in*)info->ai_addr;
            address a;
            a.host = ntohl(sa->sin_addr.s_addr);
            a.port = ntohs(sa->sin_port);
            a.protocol = ip::protocol(info->ai_socktype);
            if (not callback(context, a)) break;
        }

        freeaddrinfo(infos);
        return true;
    }


    bool
    addresses(
        ip::protocol protocol, const char* url,
        void* context, void (*callback)(void*, ip::address)
    ) {
        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family   = AF_INET;
        hints.ai_socktype = protocol;
        hints.ai_flags    = AI_PASSIVE | AI_CANONNAME;

        ip::host host(url);

        addrinfo* infos = nullptr;
        if (getaddrinfo(host.addr, host.port, &hints, &infos)) {
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


    // error ===================================================================


    error::error() : id(errno) {}


    const char*
    error::message() const { return id ? strerror(id) : "OK"; }


    std::ostream& operator<<(std::ostream& out, const error& e) {
        return out << "error(" << e.id << "): '" << e.message() << "'";
    }


    // target ==================================================================


    void
    target::clear() { memset(head, 0, size); }


    // socket ==================================================================


    address
    socket::address() const {
        sockaddr_in a; socklen_t size = sizeof(a);
        if (ok(getsockname(id, (sockaddr*)&a, &size))) {
            ip::address address;
            address.host = ntohl(a.sin_addr.s_addr);
            address.port = ntohs(a.sin_port);
            return address;
        }
        return {};
    }


    socket
    socket::accept() const {
        sockaddr_in a; socklen_t size = sizeof(a);
        return socket(::accept(id, (sockaddr*)&a, &size));
    }


    error
    socket::bind(ip::address address) {
        if (not ok()) {
            if (auto err = open(address.protocol)) {
                return err;
            }
        }
        sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_port        = htons(address.port);
        sa.sin_addr.s_addr = htonl(address.host);
        return
            ok(::bind(id, (sockaddr*)&sa, sizeof(sa)))
            ? error::none()
            : error();
    }


    error
    socket::close() {
        const int old_id = id;
        new(this)socket();
        return
            ok(::close(old_id))
            ? error::none()
            : error();
    }


    error
    socket::connect(ip::address address) {
        if (not ok()) {
            if (auto err = open(address.protocol)) {
                return err;
            }
        }
        sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_port        = htons(address.port);
        sa.sin_addr.s_addr = htonl(address.host);
        return
            ok(::connect(id, (sockaddr*)&sa, sizeof(sa)))
            ? error::none()
            : error();
    }


    error
    socket::listen(int backlog) {
        return
            ok(::listen(id, (backlog > 0) ? backlog : SOMAXCONN))
            ? error::none()
            : error();
    }


    error
    socket::listen(ip::address address, int backlog) {
        if (ok()) close();
        if (auto err = open(address.protocol)) return err;
        if (auto err = bind(address)) return err;
        return listen(backlog);
    }


    #ifndef MSG_NOSIGNAL
        enum { MSG_NOSIGNAL = 0 };
    #endif


    #ifndef SO_NOSIGPIPE
        enum { SO_NOSIGPIPE = 0x1022 };
    #endif


    error
    socket::open(protocol p) {
        NET_SOCKET_SYSTEM_INITIALIZATION;
        if (id > INVALID) close();
        const int ipproto =
            (p == SOCK_STREAM) ? IPPROTO_TCP :
            (p == SOCK_DGRAM)  ? IPPROTO_UDP : 0;
        const int new_id = ::socket(AF_INET, p, ipproto);
        new(this)socket(new_id);
        if (not ok()) return error();
        return setsockopt(SOL_SOCKET, SO_NOSIGPIPE, true);
    }


    transfer
    socket::recv(target data) const {
        char* const head = (char*)data.head;
        int   const size = int(data.size);
        const size_t rcvd = ::recv(id, head, size, MSG_NOSIGNAL);
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
        const char* head = (const char*)data.head;
        const int   size = int(data.size);
        const size_t sent = ::send(id, head, size, MSG_NOSIGNAL);
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


    error
    socket::setsockopt(int level, int key, source value) {
        #ifndef SO_NOSIGPIPE
        if (level == SOL_SOCKET && key == SO_NOSIGPIPE) {
            return error::none();
        }
        #endif // SO_NOSIGPIPE
        const char* head = (const char*)value.head;
        const int   size = int(value.size);
        return
            ok(::setsockopt(id, level, key, head, size))
            ? error::none()
            : error();
    }


    error
    socket::setsockopt(int level, int key, int value) {
        return setsockopt(level, key, source(value));
    }


    error
    socket::setsockopt(int level, int key, bool value) {
        return setsockopt(level, key, int(value));
    }


}} // namespace net::ip


//==============================================================================


namespace net {
namespace http {


    // method ==================================================================


    const char*
    method_to_string(http::method m) {
        switch (m) {
            default:      return "UNKNOWN";
            case CONNECT: return "CONNECT";
            case DELETE:  return "DELETE";
            case GET:     return "GET";
            case HEAD:    return "HEAD";
            case OPTIONS: return "OPTIONS";
            case PATCH:   return "PATCH";
            case POST:    return "POST";
            case PUT:     return "PUT";
            case TRACE:   return "TRACE";
        }
    }

    http::method
    string_to_method(const char* str) {
        substr sub(str);
        if (sub.has_prefix("CONNECT")) return CONNECT;
        if (sub.has_prefix("DELETE"))  return DELETE;
        if (sub.has_prefix("GET"))     return GET;
        if (sub.has_prefix("HEAD"))    return HEAD;
        if (sub.has_prefix("OPTIONS")) return OPTIONS;
        if (sub.has_prefix("PATCH"))   return PATCH;
        if (sub.has_prefix("POST"))    return POST;
        if (sub.has_prefix("PUT"))     return PUT;
        if (sub.has_prefix("TRACE"))   return TRACE;
        return METHOD_UNKNOWN;
    }


    // status ==================================================================


    http::status
    string_to_status(const char* str) {
        substr sub(str);
        if (sub.has_prefix("100") or sub.has_prefix("Continue"))                        return CONTINUE;
        if (sub.has_prefix("101") or sub.has_prefix("Switching Protocols"))             return SWITCHING_PROTOCOLS;
        if (sub.has_prefix("102") or sub.has_prefix("Processing"))                      return PROCESSING;
        if (sub.has_prefix("200") or sub.has_prefix("OK"))                              return OK;
        if (sub.has_prefix("201") or sub.has_prefix("Created"))                         return CREATED;
        if (sub.has_prefix("202") or sub.has_prefix("Accepted"))                        return ACCEPTED;
        if (sub.has_prefix("203") or sub.has_prefix("Non-Authoritative Information"))   return NON_AUTHORITATIVE_INFORMATION;
        if (sub.has_prefix("204") or sub.has_prefix("No Content"))                      return NO_CONTENT;
        if (sub.has_prefix("205") or sub.has_prefix("Reset Content"))                   return RESET_CONTENT;
        if (sub.has_prefix("206") or sub.has_prefix("Partial Content"))                 return PARTIAL_CONTENT;
        if (sub.has_prefix("300") or sub.has_prefix("Multiple Choices"))                return MULTIPLE_CHOICES;
        if (sub.has_prefix("301") or sub.has_prefix("Moved Permanently"))               return MOVED_PERMANENTLY;
        if (sub.has_prefix("302") or sub.has_prefix("Found"))                           return FOUND;
        if (sub.has_prefix("303") or sub.has_prefix("See Other"))                       return SEE_OTHER;
        if (sub.has_prefix("304") or sub.has_prefix("Not Modified"))                    return NOT_MODIFIED;
        if (sub.has_prefix("305") or sub.has_prefix("Use Proxy"))                       return USE_PROXY;
        if (sub.has_prefix("306") or sub.has_prefix("Switch Proxy"))                    return SWITCH_PROXY;
        if (sub.has_prefix("307") or sub.has_prefix("Temporary Redirect"))              return TEMPORARY_REDIRECT;
        if (sub.has_prefix("400") or sub.has_prefix("Bad Request"))                     return BAD_REQUEST;
        if (sub.has_prefix("401") or sub.has_prefix("Unauthorized"))                    return UNAUTHORIZED;
        if (sub.has_prefix("402") or sub.has_prefix("Payment Required"))                return PAYMENT_REQUIRED;
        if (sub.has_prefix("403") or sub.has_prefix("Forbidden"))                       return FORBIDDEN;
        if (sub.has_prefix("404") or sub.has_prefix("Not Found"))                       return NOT_FOUND;
        if (sub.has_prefix("405") or sub.has_prefix("Method Not Allowed"))              return METHOD_NOT_ALLOWED;
        if (sub.has_prefix("406") or sub.has_prefix("Not Acceptable"))                  return NOT_ACCEPTABLE;
        if (sub.has_prefix("407") or sub.has_prefix("Proxy Authentication Failed"))     return PROXY_AUTHENTICATION_FAILED;
        if (sub.has_prefix("408") or sub.has_prefix("Request Time-out"))                return REQUEST_TIMEOUT;
        if (sub.has_prefix("409") or sub.has_prefix("Conflict"))                        return CONFLICT;
        if (sub.has_prefix("410") or sub.has_prefix("Gone"))                            return GONE;
        if (sub.has_prefix("411") or sub.has_prefix("Length Required"))                 return LENGTH_REQUIRED;
        if (sub.has_prefix("412") or sub.has_prefix("Precondition Failed"))             return PRECONDITION_FAILED;
        if (sub.has_prefix("413") or sub.has_prefix("Request Entity Too Large"))        return REQUEST_ENTITY_TOO_LARGE;
        if (sub.has_prefix("414") or sub.has_prefix("Request-URI Too Long"))            return REQUEST_URI_TOO_LONG;
        if (sub.has_prefix("415") or sub.has_prefix("Unsupported Media Type"))          return UNSUPPORTED_MEDIA_TYPE;
        if (sub.has_prefix("416") or sub.has_prefix("Requested Range Not Satisfiable")) return REQUESTED_RANGE_NOT_SATISFIABLE;
        if (sub.has_prefix("417") or sub.has_prefix("Expectation Failed"))              return EXPECTATION_FAILED;
        if (sub.has_prefix("500") or sub.has_prefix("Internal Server Error"))           return INTERNAL_SERVER_ERROR;
        if (sub.has_prefix("501") or sub.has_prefix("Not Implemented"))                 return NOT_IMPLEMENTED;
        if (sub.has_prefix("502") or sub.has_prefix("Bad Gateway"))                     return BAD_GATEWAY;
        if (sub.has_prefix("503") or sub.has_prefix("Service Unavailable"))             return SERVICE_UNAVAILABLE;
        if (sub.has_prefix("504") or sub.has_prefix("Gateway Timeout"))                 return GATEWAY_TIMEOUT;
        if (sub.has_prefix("505") or sub.has_prefix("HTTP Version Not Supported"))      return HTTP_VERSION_NOT_SUPPORTED;
        return STATUS_UNKNOWN;
    }


    const char*
    status_to_string(http::status s) {
        switch (s) {
            default: return "UNKNOWN";

            // 1XX - Informational
            case CONTINUE:                        return "Continue";
            case SWITCHING_PROTOCOLS:             return "Switching Protocols";
            case PROCESSING:                      return "Processing";

            // 2XX - Success
            case OK:                              return "OK";
            case CREATED:                         return "Created";
            case ACCEPTED:                        return "Accepted";
            case NON_AUTHORITATIVE_INFORMATION:   return "Non-Authoritative Information";
            case NO_CONTENT:                      return "No Content";
            case RESET_CONTENT:                   return "Reset Content";
            case PARTIAL_CONTENT:                 return "Partial Content";

            // 3XX - Redirection
            case MULTIPLE_CHOICES:                return "Multiple Choices";
            case MOVED_PERMANENTLY:               return "Moved Permanently";
            case FOUND:                           return "Found";
            case SEE_OTHER:                       return "See Other";
            case NOT_MODIFIED:                    return "Not Modified";
            case USE_PROXY:                       return "Use Proxy";
            case SWITCH_PROXY:                    return "Switch Proxy";
            case TEMPORARY_REDIRECT:              return "Temporary Redirect";

            // 4XX - Client Error
            case BAD_REQUEST:                     return "Bad Request";
            case UNAUTHORIZED:                    return "Unauthorized";
            case PAYMENT_REQUIRED:                return "Payment Required";
            case FORBIDDEN:                       return "Forbidden";
            case NOT_FOUND:                       return "Not Found";
            case METHOD_NOT_ALLOWED:              return "Method Not Allowed";
            case NOT_ACCEPTABLE:                  return "Not Acceptable";
            case PROXY_AUTHENTICATION_FAILED:     return "Proxy Authentication Failed";
            case REQUEST_TIMEOUT:                 return "Request Time-out";
            case CONFLICT:                        return "Conflict";
            case GONE:                            return "Gone";
            case LENGTH_REQUIRED:                 return "Length Required";
            case PRECONDITION_FAILED:             return "Precondition Failed";
            case REQUEST_ENTITY_TOO_LARGE:        return "Request Entity Too Large";
            case REQUEST_URI_TOO_LONG:            return "Request-URI Too Long";
            case UNSUPPORTED_MEDIA_TYPE:          return "Unsupported Media Type";
            case REQUESTED_RANGE_NOT_SATISFIABLE: return "Requested Range Not Satisfiable";
            case EXPECTATION_FAILED:              return "Expectation Failed";

            // 5XX - Server Error
            case INTERNAL_SERVER_ERROR:           return "Internal Server Error";
            case NOT_IMPLEMENTED:                 return "Not Implemented";
            case BAD_GATEWAY:                     return "Bad Gateway";
            case SERVICE_UNAVAILABLE:             return "Service Unavailable";
            case GATEWAY_TIMEOUT:                 return "Gateway Timeout";
            case HTTP_VERSION_NOT_SUPPORTED:      return "HTTP Version Not Supported";
        }
    }


    // request =================================================================


    static
    substr
    read_head(const substr& head, const substr& name) {
        return
            head
            .after(name).after(":").skip(isspace)
            .before("\r\n").truncate(isspace);
    }

    template<typename T>
    static
    T
    read_head(const substr& headers, const substr& name, T fallback = {}) {
        substr value = read_head(headers, name);
        return (value.empty()) ? fallback : string_to<T>(value);
    }


    void
    request::reset() {
        method = METHOD_UNKNOWN;
        uri.clear();
        query.clear();
        headers.clear();
        content.clear();
    }


    bool
    request::read(string& buffer) {
        reset();

        const substr data = buffer;

        substr head = data.including("\r\n\r\n");
        if (not head) return false;

        const size_t heading_length = head.size();
        const size_t content_length = read_head(head,"Content-Length",0);
        const size_t message_length = heading_length + content_length;

        const substr message = data.prefix(message_length);
        if (not message) return false;

        // <method> <uri> HTTP/1.1\r\n
        method = string_to_method(head.begin());
        if (not method) {
            reset();
            return false;
        }

        substr url = 
            head
            .after(method_to_string(method))
            .skip(isspace)
            .before("HTTP/1.1\r\n")
            .truncate(isspace);

        uri = url;

        if (url.seek('?')) {
            uri = url.before('?');
            url = url.after('?');
            while (url) {
                if (const substr key = url.before('=')) {
                    const substr value =
                        (url.seek('&'))
                        ? url.after('=').before('&')
                        : url.after('=');
                    query.set(string(key), string(value));
                }
                url = url.after('&');
            }
        }

        head = head.after("HTTP/1.1\r\n");
        while (head) {
            if (const substr key = head.before(':')) {
                const substr value =
                    head
                    .after(':')
                    .skip(isspace)
                    .before("\r\n")
                    .truncate(isspace);
                headers.set(string(key), string(value));
            }
            head = head.after("\r\n");
        }

        content = message.after("\r\n\r\n");

        buffer.erase(0, message.length());
        return true;
    }


    void
    request::write(string& buffer) const {
        buffer.append(method_to_string(method));
        buffer.append(" ");
        buffer.append(uri);
        if (query.any()) {
            buffer.append("?");
            for (auto& pair : query) {
                buffer.append(pair.first);
                buffer.append("=");
                buffer.append(pair.second);
                buffer.append("&");
            }
            buffer.pop_back();
        }
        buffer.append(" HTTP/1.1\r\n");
        for (auto& pair : headers) {
            buffer.append(pair.first);
            buffer.append(": ");
            buffer.append(pair.second);
            buffer.append("\r\n");
        }
        buffer.append("\r\n");
        buffer.append(content);
    }

    string
    request::write() const {
        string buffer;
        write(buffer);
        return buffer;
    }


    response
    request::send() const {
        ip::address address(ip::TCP, uri);
        if (not address.ok()) return {};

        ip::socket socket;
        if (auto err = socket.connect(address)) {
            std::cout << "socket.connect(" << address << ") " << err << '\n';
            return {};
        }

        std::string  message = write();
        ip::transfer tx = socket.sendall(message);
        if (tx.error) {
            std::cout << "socket.sendall(message) " << tx.error << '\n';
            return {};
        }

        response response; string response_buffer;

        char block[4096];
        while ((tx = socket.recv(block)) and tx.size) {
            response_buffer.append(block, tx.size);
            if (response.read(response_buffer)) {
                std::cout << "received!\n";
                return response;
            }
        }

        if (tx.error) {
            std::cout << "socket.recv(block) " << tx.error << '\n';
        }
        return {};
    }


    std::ostream& operator<<(std::ostream& out, const request& req) {
        return out << req.write();
    }


    // response ================================================================


    void
    response::reset() {
        status = STATUS_UNKNOWN;
        headers.clear();
        content.clear();
    }


    bool
    response::read(string& buffer) {
        reset();

        const substr data = buffer;

        substr head = data.including("\r\n\r\n");
        if (not head) return false;

        const size_t heading_length = head.size();
        const size_t content_length = read_head(head,"Content-Length",0);
        const size_t message_length = heading_length + content_length;

        const substr message = data.prefix(message_length);
        if (not message) return false;

        // HTTP/1.1 <status-id> <status-name>\r\n
        status = string_to_status(head.after("HTTP/1.1").skip(isspace));
        if (not status) {
            reset();
            return false;
        }

        head = head.after("\r\n");
        while (head) {
            if (const substr key = head.before(':')) {
                const substr value =
                    head
                    .after(':')
                    .skip(isspace)
                    .before("\r\n")
                    .truncate(isspace);
                headers.set(string(key), string(value));
            }
            head = head.after("\r\n");
        }

        content = message.after("\r\n\r\n");

        buffer.erase(0, message.length());
        return true;
    }


    void
    response::write(string& buffer) const {
        buffer.append("HTTP/1.1 ");
        buffer.append(to_string(int(status)));
        buffer.append(" ");
        buffer.append(to_string(status));
        buffer.append("\r\n");

        for (auto& pair : headers) {
            buffer.append(pair.first);
            buffer.append(": ");
            buffer.append(pair.second);
            buffer.append("\r\n");
        }

        const size_t content_length = content.length();
        if (not headers.has("Content-Length")) {
            buffer.append("Content-Length: ");
            buffer.append(to_string(content_length));
            buffer.append("\r\n");
        }

        buffer.append("\r\n");
        buffer.append(content.data(), content_length);
    }


    string
    response::write() const {
        string buffer;
        write(buffer);
        return buffer;
    }


    std::ostream& operator<<(std::ostream& out, const response& res) {
        return out << res.write();
    }


    // get =====================================================================


    response
    get(string uri, pairs query, pairs headers) {
        request req { GET, uri, {}, std::move(headers) };
        return req.send();
    }


    response
    getJSON(string uri, pairs query) {
        pairs headers {{"Accept", "application/json"}};
        return get(uri, std::move(query), std::move(headers));
    }


    response
    getHTML(string uri, pairs query) {
        pairs headers {{"Accept", "text/html"}};
        return get(uri, std::move(query), std::move(headers));
    }


    response
    getText(string uri, pairs query) {
        pairs headers {{"Accept", "text/plain"}};
        return get(uri, std::move(query), std::move(headers));
    }


    // server ==================================================================


    server::client::client(ip::socket&& s)
    : socket(std::move(s))
    , id(socket.id) {
        //printf("client::client(%i)\n", id);
    }


    server::client::~client() {
        //printf("client::~client(%i)\n", id);
    }


    //--------------------------------------------------------------------------


    uint16_t
    server::port() const {
        return listener.port();
    }


    //--------------------------------------------------------------------------


    ip::error
    server::start(uint16_t port) {
        if (listener.ok()) stop();

        if (auto err = listener.open(ip::TCP))
            return err;

        if (auto err = listener.bind(port))
            return err;

        if (auto err = listener.listen())
            return err;

        std::thread([this]{ listen(); }).detach();
        return ip::error::none();
    }


    void
    server::stop() {
        listener.close();

        // wait for listen thread to stop
        lock listen_lock(listen_mutex);

        // now that listen thread has stopped,
        // no additional clients can be added.

        {
            // close all client sockets
            lock clients_lock(clients_mutex);
            for (auto& c : clients) { c->socket.close(); }

        }

        for (;;) {
            // wait for all client threads to stop
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            lock clients_lock(clients_mutex);
            if (clients.empty()) break;
            //puts("server::stop() waiting for client threads...");
        }

        //puts("server::stop() DONE");
    }


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


    void
    server::listen() {
        lock listen_lock(listen_mutex);
        //printf("server::listen() on port %u\n", listener.port());
        while (listener.ok()) {
            if (auto err = listener.listen()) {
                printf("server::listen() error: '%s'\n", err.message());
                continue;
            }
            if (ip::socket socket = listener.accept()) {
                lock clients_lock(clients_mutex);
                auto result = clients.emplace(new client(std::move(socket)));
                assert(result.second); // successfully emplaced
                std::thread([this,result]{ serve(*result.first); }).detach();
            }
        }
        //puts("server::listen() DONE");
    }


    void
    server::serve(const client_ptr& client_ptr) {
        client& client = *client_ptr;
        socket& socket = client.socket;

        //const int client_id = client.id;
        //printf("client(%i) connected on port %u\n", client_id, socket.port());

        request  request;  string request_buffer;
        response response; string response_buffer;

        ip::transfer rcvd; char block[4096];

        while ((rcvd = socket.recv(block)) and rcvd.size) {
            request_buffer.append(block, rcvd.size);
            while (request.read(request_buffer)) {
                service(request, response);

                if (not response.ok()) {
                    response.status = NOT_IMPLEMENTED;
                }
                response.write(response_buffer);
                socket.sendall(response_buffer);
                response_buffer.clear();
                response.reset();

                if (request.headers.get("connection") == "close") {
                    //printf("client(%i) requested disconnection\n", client_id);
                    goto disconnect;
                }
            }
        }

    disconnect:

        socket.close();

        if (rcvd.error) {
            //printf("client(%i) error: %s\n", client_id, rcvd.error.message());
        }
        else {
            //printf("client(%i) disconnected\n", client_id);
        }

        lock clients_lock(clients_mutex);
        clients.erase(client_ptr);
        //printf("server::serve(client(%i)) DONE\n", client_id);
    }


}} // namespace net::http
