#include "Socket.hpp"

#define CLIENT_TIMEOUT 5

GamepadDelegate::~GamepadDelegate() { dismiss_delegate(); }

const std::optional<const Command> GamepadDelegate::next_command() {
    if (!_delegate.has_value() && !connect_to_delegate()) {
        return std::optional<const Command>();
    }
    try {
        auto cmd = _delegate->next_command();
        if (cmd.has_value()) {
            time(&_last_activity);

            if (cmd->raw_type == QUIT_STR) {
                dismiss_delegate();
            } else {
                return cmd;
            }
        } else if (time(NULL) - _last_activity > CLIENT_TIMEOUT) {
            dismiss_delegate();
        }
    } catch (IOException &ex) {
        dismiss_delegate();
    }
    return std::optional<const Command>();
}

void GamepadDelegate::dismiss_delegate() noexcept { _delegate.reset(); }

GamepadServer::GamepadServer(uint16_t listening_port) : _local_addr(AF_INET6, listening_port) {
    _socket.bind(_local_addr);
    _socket.listen(1);
}

GamepadServer::~GamepadServer() {}

bool GamepadServer::connect_to_delegate() {
    auto client_opt = _socket.accept();
    if (client_opt.has_value()) {
        auto [client, addr] = *client_opt;

        std::cout << "Remote gamepad connected: " << addr.ip_repr() << " on port " << addr.port()
                  << "\n";

        _delegate.emplace(client);

        time(&_last_activity);

        return true;
    } else {
        return false;
    }
}

void GamepadServer::dismiss_delegate() noexcept {
    GamepadDelegate::dismiss_delegate();
    std::cout << "Remote gamepad disconnected\n";
}

GamepadClient::GamepadClient(const std::string &addr, bool bind, int binding_port)
    : GamepadClient(SockAddr(addr), bind, binding_port) {}

GamepadClient::GamepadClient(const SockAddr &addr, bool bind, int binding_port)
    : _server_addr(addr) {
    if (bind) {
        _local_addr.emplace(AF_INET6, binding_port);
    };
}

bool GamepadClient::connect_to_delegate() {
    try {
        if (!tmp_socket.has_value()) {
            tmp_socket.emplace();
            if (_local_addr.has_value()) {
                tmp_socket->bind(*_local_addr);
            }
        }
        if (tmp_socket->connect(_server_addr)) {
            _delegate.emplace(*tmp_socket);
            tmp_socket.reset();
            time(&_last_activity);
            std::cout << "Connected to server.\n";
            return true;
        }
    } catch (IOException e) {
        dismiss_delegate();
        if (time(NULL) - _last_activity > CLIENT_TIMEOUT) {
            time(&_last_activity);
            std::cout << "Cannot connect to server: " << e.what() << "\n";
        }
    }
    return false;
}

void GamepadClient::dismiss_delegate() noexcept {
    if (!tmp_socket.has_value()) {
        GamepadDelegate::dismiss_delegate();
        std::cout << "Lost connection with server\n";
    } else {
        tmp_socket->close();
        tmp_socket.reset();
    }
}