#pragma once


namespace net {
namespace ip {


    error::error() : id(errno) {}


    const char*
    error::message() const { return id ? strerror(id) : "OK"; }


}} // namespace net::ip