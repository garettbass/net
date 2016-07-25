#net (https://github.com/garettbass/net)

some simple, composable networking primitives using Berkeley sockets and C++11


## Features

* slim RAII socket: `net::ip::socket`
* slim RAII & multithreaded HTTP server: `net::http::server`

NOTE: `net::http::server` does not internally handle the "Expect: 100-continue" HTTP header


## The Unlicense (see UNLICENSE.txt)

A license with no conditions whatsoever which dedicates works to the public domain. Unlicensed works, modifications, and larger works may be distributed under different terms and without source code.
