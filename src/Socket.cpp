#include "Socket.hpp"

#include <arpa/inet.h> //inet_addr
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h> //close

#define QUIT_STR "QUIT"

RemoteGamepadSocket::RemoteGamepadSocket(int socket) : _socket(socket) {
    int flags = fcntl(_socket, F_GETFL);
    try_opt(fcntl(_socket, F_SETFL, flags | O_NONBLOCK),
            "activate non-blocking mode");
}

RemoteGamepadSocket::~RemoteGamepadSocket() {
    if (_socket > -1) {
        write(_socket, QUIT_STR, strlen(QUIT_STR));
        write(_socket, "\n", 1);

        close(_socket);
    }
}

const std::optional<const Command>
RemoteGamepadSocket::next_buffered_command(size_t start_pos) {
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
    ssize_t r = read(_socket, buffer, MAX_BUFFER_SIZE - _buffer.length());
    if (r == 0 || (r == -1 && (errno == EWOULDBLOCK || errno == EAGAIN))) {
        return std::optional<const Command>();
    }
    check(r);

    int prev_length = _buffer.length();
    _buffer.append(buffer, r);
    return next_buffered_command(prev_length);
}

GamepadServer::GamepadServer(int listening_port) {
    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listening_port);

    _socket = check(socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0));
    int value = 1;
    try_opt(
        setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)),
        "set reuseaddr");

    check(bind(_socket, (sockaddr *)&addr, sizeof(addr)));
    check(listen(_socket, 1));
}

GamepadServer::~GamepadServer() {
    if (_socket > -1) {
        try_opt(close(_socket), "close socket");
    }
}

bool GamepadServer::accept_client() {
    sockaddr_in addr = {0};
    socklen_t socklen = sizeof(addr);

    int client = accept4(_socket, (sockaddr *)&addr, &socklen, SOCK_NONBLOCK);
    if (client == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
        return false;
    }
    check(client);
    std::cout << "Remote gamepad connected: " << inet_ntoa(addr.sin_addr)
              << " on port " << htons(addr.sin_port) << "\n";

    _client.emplace(client);
    time(&last_activity);

    return true;
}

void GamepadServer::dismiss_client() {
    _client.reset();
    _client_ready = false;
    std::cout << "Remote gamepad disconnected\n";
}

const std::optional<const Command> GamepadServer::next_command() {
    if (!_client.has_value() && !accept_client()) {
        return std::optional<const Command>();
    }
    try {
        auto cmd = _client->next_command();   
        if (cmd.has_value()) {
            time(&last_activity);

            if (cmd->raw_type == "READY") {
                if (!_client_ready) {
                    _client_ready = true;
                    std::cout << "--Remote gamepad READY to send commands--\n";
                }
            } else if (cmd->raw_type == QUIT_STR) {
                dismiss_client();
            } else {
                return cmd;
            }
        } else if(time(NULL) - last_activity > CLIENT_TIMEOUT) {                 
            dismiss_client();        
        }
    } catch (IOException &ex) {
        dismiss_client();
    }
    return std::optional<const Command>();
}