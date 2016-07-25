#pragma once
#include <map>
#include <mutex>
#include <string>
#include <unordered_set>
#include "ip.h"


namespace net {
namespace http {


    using string = std::string;
    using socket = net::ip::socket;


    //--------------------------------------------------------------------------


    enum method {
        METHOD_UNKNOWN,
        CONNECT,
        DELETE,
        GET,
        HEAD,
        OPTIONS,
        PATCH,
        POST,
        PUT,
        TRACE,
    };


    const char*
    method_to_string(http::method);


    http::method
    string_to_method(const char*);


    inline
    http::method
    string_to_method(const string& s) { return string_to_method(s.c_str()); }


    //--------------------------------------------------------------------------


    enum status {
        STATUS_UNKNOWN,

        // 1XX - Informational
        CONTINUE                        = 100,
        SWITCHING_PROTOCOLS             = 101,
        PROCESSING                      = 102,

        // 2XX - Success
        OK                              = 200,
        CREATED                         = 201,
        ACCEPTED                        = 202,
        NON_AUTHORITATIVE_INFORMATION   = 203,
        NO_CONTENT                      = 204,
        RESET_CONTENT                   = 205,
        PARTIAL_CONTENT                 = 206,

        // 3XX - Redirection
        MULTIPLE_CHOICES                = 300,
        MOVED_PERMANENTLY               = 301,
        FOUND                           = 302,
        SEE_OTHER                       = 303,
        NOT_MODIFIED                    = 304,
        USE_PROXY                       = 305,
        SWITCH_PROXY                    = 306,
        TEMPORARY_REDIRECT              = 307,

        // 4XX - Client Error
        BAD_REQUEST                     = 400,
        UNAUTHORIZED                    = 401,
        PAYMENT_REQUIRED                = 402,
        FORBIDDEN                       = 403,
        NOT_FOUND                       = 404,
        METHOD_NOT_ALLOWED              = 405,
        NOT_ACCEPTABLE                  = 406,
        PROXY_AUTHENTICATION_FAILED     = 407,
        REQUEST_TIMEOUT                 = 408,
        CONFLICT                        = 409,
        GONE                            = 410,
        LENGTH_REQUIRED                 = 411,
        PRECONDITION_FAILED             = 412,
        REQUEST_ENTITY_TOO_LARGE        = 413,
        REQUEST_URI_TOO_LONG            = 414,
        UNSUPPORTED_MEDIA_TYPE          = 415,
        REQUESTED_RANGE_NOT_SATISFIABLE = 416,
        EXPECTATION_FAILED              = 417,

        // 5XX - Server Error
        INTERNAL_SERVER_ERROR           = 500,
        NOT_IMPLEMENTED                 = 501,
        BAD_GATEWAY                     = 502,
        SERVICE_UNAVAILABLE             = 503,
        GATEWAY_TIMEOUT                 = 504,
        HTTP_VERSION_NOT_SUPPORTED      = 505,

    };


    const char*
    status_to_string(http::status s);


    http::status
    string_to_status(const char*);


    inline
    http::status
    string_to_status(const string& s) { return string_to_status(s.c_str()); }


    //--------------------------------------------------------------------------


    template<typename T>
    T
    string_to(const string&);


    template<> inline
    int
    string_to<int>(const string& s) {
        return int(std::strtol(s.c_str(),nullptr,0));
    }


    template<> inline
    long
    string_to<long>(const string& s) {
        return std::strtol(s.c_str(),nullptr,0);
    }


    template<> inline
    long long
    string_to<long long>(const string& s) {
        return std::strtoll(s.c_str(),nullptr,0);
    }


    template<> inline
    unsigned
    string_to<unsigned>(const string& s) {
        return unsigned(std::strtoul(s.c_str(),nullptr,0));
    }


    template<> inline
    unsigned long
    string_to<unsigned long>(const string& s) {
        return std::strtoul(s.c_str(),nullptr,0);
    }


    template<> inline
    unsigned long long
    string_to<unsigned long long>(const string& s) {
        return std::strtoull(s.c_str(),nullptr,0);
    }


    template<> inline
    float
    string_to<float>(const string& s) {
        return std::strtof(s.c_str(),nullptr);
    }


    template<> inline
    double
    string_to<double>(const string& s) {
        return std::strtod(s.c_str(),nullptr);
    }


    template<> inline
    long double
    string_to<long double>(const string& s) {
        return std::strtold(s.c_str(),nullptr);
    }


    template<> inline
    http::method
    string_to<http::method>(const string& s) {
        return string_to_method(s);
    }


    template<> inline
    http::status
    string_to<http::status>(const string& s) {
        return string_to_status(s);
    }


    //--------------------------------------------------------------------------


    template<typename T>
    string
    to_string(T&& t) { return std::to_string(t); }


    inline
    string
    to_string(http::method m) { return method_to_string(m); }


    inline
    string
    to_string(http::status s) { return status_to_string(s); }


    //--------------------------------------------------------------------------


    class pairs {
        std::map<string, string> map;

    public: // structors

        pairs() = default;
        pairs(decltype(map) map) : map(std::move(map)) {};

        explicit
        pairs(const pairs&) = default;
        pairs& operator=(const pairs&) = default;

        pairs(pairs&&) = default;
        pairs& operator=(pairs&&) = default;

    public: // operators

        string& operator[](const string& key)       { return map[key]; }
        string  operator[](const string& key) const { return get(key); }

    public: // methods

        void clear() { map.clear(); }

        bool has(const string& key) const {
            auto itr = map.find(key);
            return (itr != map.end());
        }

        string get(const string& key) const {
            auto itr = map.find(key);
            return (itr != map.end()) ? itr->second : string{""};
        }

        template<typename T>
        T get(const string& key, T fallback = {}) const {
            auto itr = map.find(key);
            return (itr != map.end()) ? string_to<T>(itr->second) : fallback;
        }

        void set(const string& key, string value) {
            map[key] = value;
        }

        template<typename T>
        void set(const string& key, T value) {
            set(key, to_string(value));
        }

    public: // iterators

        decltype(map.cbegin()) begin() const { return map.cbegin(); }
        decltype(map.cend())     end() const { return map.cend(); }

    };


    //--------------------------------------------------------------------------


    struct request {
        http::method method = METHOD_UNKNOWN;
        http::string uri;
        http::pairs  query;
        http::pairs  headers;
        http::string content;

    public: // structors

        request() = default;

        request(http::method method) : method(method) {}

        request(string s) { read(s); }

        request(const char* s) : request(string(s)) {}

        explicit
        request(const request&) = default;
        request& operator=(const request&) = default;

        request(request&&) = default;
        request& operator=(request&&) = default;

    public: // operators

        explicit operator bool() const { return ok(); }

    public: // properties

        bool ok() const { return method > METHOD_UNKNOWN; }

    public: // methods

        void reset();

        bool read(string& buffer);

        void write(string& buffer) const;

        string write() const;

    };


    inline
    ip::socket& operator<<(ip::socket& out, const request& req) {
        out.sendall(req.write()); return out;
    }


    //--------------------------------------------------------------------------


    struct response {
        http::status status = STATUS_UNKNOWN;
        http::pairs  headers;
        http::string content;

    public: // structors

        response() = default;

        response(http::status status) : status(status) {}

        response(string s) { read(s); }

        response(const char* s) : response(string(s)) {}

        explicit
        response(const response&) = default;
        response& operator=(const response&) = default;

        response(response&&) = default;
        response& operator=(response&&) = default;

    public: // operators

        explicit operator bool() const { return ok(); }

    public: // properties

        bool ok() const { return status > STATUS_UNKNOWN; }

    public: // methods

        void reset();

        bool read(string& buffer);

        void write(string& buffer) const;

        string write() const;

    };


    inline
    ip::socket& operator<<(ip::socket& out, const response& res) {
        out.sendall(res.write()); return out;
    }


    //--------------------------------------------------------------------------


    using service = std::function<void(const request&, response&)>;


    //--------------------------------------------------------------------------


    class server {

        struct client {
            ip::socket socket;
            std::mutex serve_mutex;
            client(ip::socket&& socket);
           ~client();
        };

        using client_ptr = std::unique_ptr<client>;
        using client_set = std::unordered_set<client_ptr>;

        http::service  service;
        client_set     clients;
        std::mutex     clients_mutex;
        ip::socket     listener;
        std::mutex     listen_mutex;

    public: // structors

        server(http::service service)
        : service(service)
        {}

        server(uint16_t port, http::service service)
        : service(service)
        { start(port); }

       ~server() { stop(); }

        server(const server&) = delete;
        server& operator=(const server&) = delete;

        server(server&&) = delete;
        server& operator=(server&&) = delete;

    public: // operators

        explicit operator bool() const { return ok(); }

    public: // properties

        bool ok() const { return listener.ok(); }

    public: // methods

        ip::error start(uint16_t port);

        void stop();

    private: // implementation

        void listen();
        void accept();
        void serve(const client_ptr&);
        void close(const client_ptr&);

    };


}} // namespace net::http