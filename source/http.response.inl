#pragma once


namespace net {
namespace http {


    void
    response::reset(http::status s) {
        status = s;
        headers.clear();
        content.clear();
    }


    void
    response::write(string& buffer) {
        buffer.append("HTTP/1.1 ");
        buffer.append(to_string<size_t>(status));
        buffer.append(" ");
        buffer.append(to_string(status));
        buffer.append("\r\n");

        if (const size_t content_length = content.length()) {
            headers.set("Content-Length", content_length);
        }
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
    response::write() {
        string buffer;
        write(buffer);
        return buffer;
    }


}} // namespace net::http