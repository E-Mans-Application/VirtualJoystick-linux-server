#include "Socket.hpp"

RemoteGamepadSocket::RemoteGamepadSocket(const Socket &socket) : _socket(socket) {}

RemoteGamepadSocket::~RemoteGamepadSocket() {
    _socket.send(QUIT_STR);
    _socket.send("\n");

    _socket.close();
}

const std::optional<const Command> RemoteGamepadSocket::next_buffered_command(size_t start_pos) {
    size_t i = _buffer.find('\n', start_pos);
    if (i == std::string::npos) {
        return std::optional<const Command>();
    }
    std::string cmd = _buffer.substr(0, i);
    _buffer = _buffer.substr(i + 1);
    return std::optional(Command::parse(cmd));
}

const std::optional<const Command> RemoteGamepadSocket::next_command() {
    if (_buffer.length() >= MAX_BUFFER_SIZE) {
        throw IOException(EOVERFLOW);
    }
    std::optional<const Command> buffered = next_buffered_command();
    if (buffered.has_value()) {
        return buffered;
    }

    char buffer[MAX_BUFFER_SIZE];
    try {
        size_t r = _socket.receive(buffer, MAX_BUFFER_SIZE - _buffer.length());
        int prev_length = _buffer.length();
        _buffer.append(buffer, r);
        return next_buffered_command(prev_length);
    } catch (IOException &e) {
        if (errno == EWOULDBLOCK || errno == ENODATA) {
            return std::optional<const Command>();
        } else {
            throw e;
        }
    }
}
