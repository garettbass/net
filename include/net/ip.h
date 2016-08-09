#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <string>
#include "platform.h"
#include "assign.h"


namespace net {
namespace ip {


    using string = std::string;


    string gethostname();


    //--------------------------------------------------------------------------


    enum operation { READ, WRITE, READ_WRITE };

    enum protocol { ANY, TCP, UDP };

    enum iterate { BREAK, CONTINUE };


    //--------------------------------------------------------------------------


    struct address {

        union {
            struct { uint32_t host; uint16_t port; ip::protocol protocol:16; };
        #if NET_ENDIAN_LE
            struct { uint8_t d,c,b,a; uint16_t p; ip::protocol pr:16; };
        #elif NET_ENDIAN_BE
            struct { uint8_t a,b,c,d; uint16_t p; ip::protocol pr:16; };
        #else
            #error "undefined endianness"
        #endif
            uint64_t bits;
        };

    public: // structors

        address() : bits(0) {}

        address(
            uint8_t a, uint8_t b, uint8_t c, uint8_t d,
            uint16_t port = 0, ip::protocol protocol = ANY)
        #if NET_ENDIAN_LE
        : d(d), c(c), b(b), a(a), p(port), pr(protocol)
        #elif NET_ENDIAN_BE
        : a(a), b(b), c(c), d(d), p(port), pr(protocol)
        #endif
        {}

        address(uint16_t port)
        : host(0), port(port)
        {}

        address(ip::protocol, const char* uri);

        address(ip::protocol p, const string& uri)
        : address(p, uri.c_str()) {}

    public: // operators

        explicit operator bool() const { return bits != 0; }

    };


    //--------------------------------------------------------------------------


    bool addresses(
        ip::protocol, const char* url,
        void*, iterate(*)(void*, ip::address)
    );


    bool addresses(
        ip::protocol, const char* url,
        void*, void(*)(void*, ip::address)
    );


    template<typename Callback>
    bool addresses(ip::protocol p, const char* url, Callback&& callback) {
        return addresses(p, url, &callback, [](void* c, ip::address a) {
            return (*(Callback*)c)(a);
        });
    }


    template<typename Callback>
    bool addresses(const char* url, Callback&& callback) {
        return addresses(ANY, url, &callback, [](void* c, ip::address a) {
            return (*(Callback*)c)(a);
        });
    }


    //--------------------------------------------------------------------------


    struct error {

        const int id;

    public: // structors

        static error none() { return error(0); }

        error();

        explicit
        error(int id) : id(id) {}

        error(const error&) = default;

    public: // operators

        error& operator=(const error& lv) { return assign(this, lv); }

        explicit operator bool() const { return (id != 0); }

        operator const char*() const { return message(); }

    public: // properties

        const char* message() const;
    };


    //--------------------------------------------------------------------------


    struct target {

        void* const  head = nullptr;
        const size_t size = 0;

    public: // structors

        target() = default;

        target(target& lv) : target((const target&)lv) {}
        target(const target&) = default;
        target& operator=(const target&) = default;

        target(void* head, size_t size)
        : head(head), size(size) { clear(); }

        template <size_t LENGTH>
        target(char (&str)[LENGTH])
        : head(str), size(LENGTH - 1) { str[LENGTH - 1] = '\0'; clear(); }

        template <typename T, size_t LENGTH>
        target(T (&array)[LENGTH])
        : head(array), size(sizeof(T) * LENGTH) { clear(); }

        template <typename T>
        target(T& value)
        : head(&value), size(sizeof(T)) { clear(); }

    public: // operators

        explicit operator bool() const { return ok(); }

    public: // properties

        bool ok() const { return head and size; }

    public: // methods

        void advance(size_t bytes) {
            if (bytes >= size) { new(this)target(); return; }
            void* const new_head = (void*)(size_t(head) + bytes);
            const size_t new_size = size - bytes;
            new(this)target(new_head, new_size);
        }

        void clear();
    };


    //--------------------------------------------------------------------------


    struct source {

        const void* const head = nullptr;
        const size_t      size = 0;

    public: // structors

        source() = default;

        source(source& lv) : source((const source&)lv) {}
        source(const source&) = default;
        source& operator=(const source&) = default;

        source(const void* head, size_t size)
        : head(head), size(size) {}

        source(const char* str)
        : head(str), size(strlen(str)) {}

        source(const string& str)
        : head(str.data()), size(str.size()) {}

        template <typename T, size_t LENGTH>
        source(const T (&array)[LENGTH])
        : head(array), size(sizeof(T) * LENGTH) {}

        template <typename T>
        source(const T& value)
        : head(&value), size(sizeof(T)) {}

    public: // operators

        explicit operator bool() const { return ok(); }

    public: // properties

        bool ok() const { return head and size; }

    public: // methods

        void advance(size_t bytes) {
            if (bytes >= size) { new(this)target(); return; }
            void* const  new_head = (void*)(size_t(head) + bytes);
            const size_t new_size = size - bytes;
            new(this)target(new_head, new_size);
        }
    };


    //--------------------------------------------------------------------------


    struct transfer {

        const size_t    size = 0;
        const ip::error error;

    public: // structors

        transfer() = default;

        explicit
        transfer(size_t size) : size(size) {}

        explicit
        transfer(ip::error error) : error(error) {}

        transfer(size_t size, ip::error error) : size(size), error(error) {}

        transfer(const transfer&) = default;

    public: // operators

        transfer& operator=(const transfer& lv) { return assign(this, lv); }

        explicit operator bool() const { return ok(); }

    public: // properties

        bool ok() const { return not error; }
    };


    //--------------------------------------------------------------------------


    struct socket {

        enum : int { INVALID = ~int(0) };

        const volatile int id = INVALID;

    public: // structors

        socket() = default;

        socket(ip::protocol p) { open(p); }

        explicit
        socket(int id) : id(id) {}

        socket(socket&& rv) : id(rv.id) { new(&rv)socket(); }

       ~socket() { close(); }

    public: // operators

        socket& operator=(socket&& rv) { return assign(this, std::move(rv)); }

        explicit operator bool() { return ok(); }

    public: // properties

        ip::address address() const;

        static
        bool ok(int id) { return id != INVALID; }
        bool ok() const { return id != INVALID; }

        uint16_t port() const { return address().port; }

    public: // core api

        socket accept() const;

        error bind(ip::address);

        error close();

        error connect(ip::address);

        error listen(int backlog = 0);
        error listen(ip::address, int backlog = 0); // open(),bind(),listen()

        error open(ip::protocol);

        transfer recv(ip::target) const;
        transfer recvall(ip::target) const;

        transfer send(ip::source) const;
        transfer sendall(ip::source) const;

        error shutdown(ip::operation = READ_WRITE);

        error setsockopt(int level, int key, ip::source);
        error setsockopt(int level, int key, int value);
        error setsockopt(int level, int key, bool value);

    };


}} // namespace net::ip
