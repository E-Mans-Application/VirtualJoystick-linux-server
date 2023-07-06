#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_

#include "Command.hpp"
#include "Fallible.hpp"
#include "SockAddr.hpp"

#include <ctime>
#include <optional>
#include <string.h>
#include <tuple>

#define QUIT_STR "QUIT"

class Socket;
typedef  std::tuple<Socket, SockAddr> SocketWithAddress;

class Socket : Fallible {
  private:
    int _inner = -1;
    Socket(int fd);

  public:
    Socket();
    void close();
    void bind(const SockAddr &addr);
    void listen(int queue_size);
    std::optional<SocketWithAddress> accept();
    bool connect(const SockAddr &addr);
    void send(const std::string &data);
    size_t receive(char *buffer, int max_len);

};


/// A socket that receives remote commands to emulate a gamepad.
class RemoteGamepadSocket : Fallible {
  private:
    Socket _socket;
    std::string _buffer;
    static const size_t MAX_BUFFER_SIZE = 1024;

    /// If the buffer contains a command, returns that command.
    /// Otherwise, returns an empty optional.
    /// @param start_pos The start position to search the command-termination
    /// char (i.e. \\n) from. Defaults to 0.
    const std::optional<const Command> next_buffered_command(size_t start_pos = 0);

  public:
    /// Creates a new RemoteGamepadSocket that will communicate with the socket.
    /// @param socket The file descriptor of the socket. Important: a 'QUIT'
    /// command is sent and  the socket is closed when the object is destroyed.
    RemoteGamepadSocket(const Socket &socket);
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

/// Base class for objects that delegate their work to a RemoteGamepadSocket.
/// This class must be inherited.
class GamepadDelegate : protected Fallible {
  protected:
    std::optional<RemoteGamepadSocket> _delegate;
    time_t _last_activity = 0;

    /// Tries to connect to an external peer.
    /// If the connection succeeds, this function shall set the fields "_delegate" and
    /// "_last_activity" and return true. Otherwise, the function shall return false. Must not
    /// block. Should return false if the connection is still pending.
    ///
    /// Must be overriden.
    virtual bool connect_to_delegate() = 0;
    /// Dismisses the delegate, if any.
    /// If overriden, the base method should be called as well.
    virtual void dismiss_delegate() noexcept;

    ~GamepadDelegate();

  public:
    /// Returns the next command received from a connected peer.
    /// Returns an empty optional if no peer is currently connected or if
    /// the connected peer has not sent any command yet.
    /// If the peer happens to have disconnected or to let their buffer to overflow,
    /// the peer is silently dismissed and an empty optional is also returned.
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
class GamepadServer : public GamepadDelegate {
  private:
    SockAddr _local_addr;
    Socket _socket;

  protected:
    virtual bool connect_to_delegate() override;
    virtual void dismiss_delegate() noexcept override;

  public:
    /// Creates a new TCP server.
    /// @param listening_port The TCP port the server shall be bound to.
    GamepadServer(uint16_t listening_port);
    ~GamepadServer();
};

/// @brief  A TCP client that receives the commands from an external intermediary server.
class GamepadClient : public GamepadDelegate {
  private:
    SockAddr _server_addr;
    std::optional<SockAddr> _local_addr;
    std::optional<Socket> tmp_socket;

  protected:
    virtual bool connect_to_delegate() override;
    virtual void dismiss_delegate() noexcept override;

  public:
    GamepadClient(const std::string &addr, bool bind = false, int binding_port = 0);
    GamepadClient(const SockAddr &addr, bool bind = false, int binding_port = 0);
};
#endif