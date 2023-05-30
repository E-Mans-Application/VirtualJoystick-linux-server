#include "Gamepad.hpp"
#include "Socket.hpp"
#include "CommandLineParser.hpp"

#include <iostream>
#include <signal.h>

bool running = true;

void interrupt(sig_atomic_t _s) { running = false; }

int main(int argc, char *argv[]) {
    signal(SIGINT, interrupt);

    CommandLineParser parser;
    parser.parse(argc, argv);

    VirtualGamePad gamePad;
    std::cout << "Created virtual device " << gamePad.getRawDeviceName()
              << "\nBound to /dev/input/" << gamePad.getEventFile()
              << "\n";
    
    int port = parser.optValue("port", 58324);
    GamepadServer server(port);
    std::cout << "Server waiting for connection on TCP port " << port << "\n";

    std::cout << "Press 'Ctrl + C' to remove virtual device and exit\n";

    while (running) {
      auto cmd = server.next_command();
      if (cmd.has_value()) {
        gamePad.process_command(*cmd);
      }
    }

    std::cout << "\nExiting...\n";
}