#include <chrono>
#include <thread>
#include <net/http.h>
#include "tests.h"


using namespace net::http;


TEST("net::http::request::read() - without query") {
    const request req =
        "GET /hello/world HTTP/1.1\r\n"
        "Host: localhost:7200\r\n"
        "\r\n";
    CHECK(req.method == GET);
    CHECK(req.uri == "/hello/world");
    CHECK(req.headers["Host"] == "localhost:7200");
}


TEST("net::http::request::read() - with query") {
    const request req =
        "GET /hello/world?a=b&c=d&e=3 HTTP/1.1\r\n"
        "Host: localhost:7200\r\n"
        "\r\n";
    CHECK(req.method == GET);
    CHECK(req.uri == "/hello/world");
    CHECK(req.query["a"] == "b");
    CHECK(req.query["c"] == "d");
    CHECK(req.query["e"] == "3");
    CHECK(req.headers["Host"] == "localhost:7200");
}


TEST("net::http::response::read() - explicit Content-Length") {
    const response res =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Hello World\n";
    CHECK(res.status == OK);
    CHECK(res.headers["Content-Length"] == "12");
    CHECK(res.headers.get<int>("Content-Length") == 12);
    CHECK(res.content == "Hello World\n");
}


TEST("net::http::response::read() - implicit Content-Length") {
    const char expected[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Hello World\n";
    response res;
    res.status = OK;
    res.content = "Hello World\n";
    CHECK(res.write() == expected);
}


int main(int, const char**) {
    return tests::run();
}