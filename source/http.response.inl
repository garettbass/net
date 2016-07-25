#pragma once


namespace net {
namespace http {


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
        buffer.append(to_string<size_t>(status));
        buffer.append(" ");
        buffer.append(to_string(status));
        buffer.append("\r\n");

        for (auto& pair : headers) {
            buffer.append(pair.first);
            buffer.append(": ");
            buffer.append(pair.second);
            buffer.append("\r\n");
        }

        if (not headers.has("Content-Length")) {
            buffer.append("Content-Length: ");
            buffer.append(to_string(content.length()));
            buffer.append("\r\n");
        }

        buffer.append("\r\n");
        buffer.append(content);
    }

    string
    response::write() const {
        string buffer;
        write(buffer);
        return buffer;
    }


}} // namespace net::http