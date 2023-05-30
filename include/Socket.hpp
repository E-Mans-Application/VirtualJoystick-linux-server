#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_

#include "Command.hpp"
#include "Fallible.hpp"

#include <netinet/in.h>
#include <optional>
#include <string.h>
#include <ctime>

/// A socket that receives remote commands to emulate a gamepad.
class RemoteGamepadSocket : Fallible {
  private:
    int _socket = -1;
    std::string _buffer;
    static const size_t MAX_BUFFER_SIZE = 1024;

    /// If the buffer contains a command, returns that command.
    /// Otherwise, returns an empty optional.
    /// @param start_pos The start position to search the command-termination
    /// char (i.e. \\n) from. Defaults to 0.
    const std::optional<const Command>
    next_buffered_command(size_t start_pos = 0);

  public:
    /// Creates a new RemoteGamepadSocket that will communicate with the socket.
    /// @param socket The file descriptor of the socket. Important: a 'QUIT'
    /// command is sent and  the socket is closed when the object is destroyed.
    RemoteGamepadSocket(int socket);
    ~RemoteGamepadSocket();

    /// Returns the next command received from the socket.
    ///
    /// If no command was received, this returns an empty optional instead.
    /// If multiple commands are received at once, only the first one is
    /// returned; the others are buffered and shall be returned one at a time on
    /// subsequent calls. The socket is not read again until there is no more
    /// buffered, pending commands.
    ///
    /// This method does not process the command, even in case of a 'QUIT'
    /// command.
    ///
    /// @throw This throws an `IOException` if the socket cannot be read, or if
    /// the buffer overflows before a command-termination char (i.e. \\n) is
    /// encoutered. If this methods throws, subsequent calls are also very
    /// likely to throw, so that this object should be destroyed and re-created
    /// with a new valid socket.
    const std::optional<const Command> next_command();
};

/// A TCP server responsible for accepting a single client.
/// This client is then supposed to send remote gamepad commands.
///
/// When the client disconnects, the server should be able to accept another
/// client again.
/// When a client is connected, the behaviour as to future connection attempts
/// is unspecified, but the server should only process commands from the first
/// connected client.
class GamepadServer : Fallible {
  private:
    int _socket = -1;
    std::optional<RemoteGamepadSocket> _client;
    bool _client_ready = false;
    time_t last_activity = 0;

    static const int CLIENT_TIMEOUT = 5;

    /// Tries to accept a connection to this server, if a
    /// connection attempt is pending. Non-blocking.
    /// @return If a connection is accepted, this returns true and
    /// sets field '_client' appropriately,
    /// Otherwise, this returns false.
    bool accept_client();
    /// Destroys the value in field '_client', if any. See destructor of
    /// 'RemoteGamepadSocket' for more details.
    void dismiss_client();

  public:
    /// Creates a new TCP server.
    /// @param listening_port The TCP port the server shall be bound to.
    GamepadServer(int listening_port);
    ~GamepadServer();
    /// Returns the next command received from a connected client.
    /// Returns an empty optional if no client is currently connected, if
    /// the connected client has not sent any command yet.
    /// If the client happens to have disconnected or to let their buffer to overflow,
    /// the client is silently dismissed and an empty optional is also returned.
    const std::optional<const Command> next_command();
};

#endif