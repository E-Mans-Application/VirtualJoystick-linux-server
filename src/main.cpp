#include "CommandLineParser.hpp"
#include "Gamepad.hpp"
#include "Socket.hpp"

#include <iostream>
#include <signal.h>
#include <memory> // unique_ptr

bool running = true;

void interrupt(sig_atomic_t _s) { running = false; }

int main(int argc, char *argv[]) {
    signal(SIGINT, interrupt);

    CommandLineParser parser;
    parser.parse(argc, argv);

    VirtualGamePad gamePad;
    std::cout << "Created virtual device " << gamePad.getRawDeviceName() << "\nBound to /dev/input/"
              << gamePad.getEventFile() << "\n";

    GamepadDelegate *receiver;

    // TODO: define DEFAULT_PORT or replace with the actual default port.
    int port = parser.optValue("--port", DEFAULT_PORT);
    bool delegate = parser.isSet("--delegate");
    bool bind = parser.isSet("--bind");
    
    if (delegate) {
        // TODO: define DEFAULT_DELEGATE_SERVER or replace with the actual default
        // server, or parser.getValue("--delegate").
        std::string delegate_server = parser.optValue<std::string>("--ping", DEFAULT_DELEGATE_SERVER);
        receiver = new GamepadClient(delegate_server, bind, port);
    } else {
        receiver = new GamepadServer(port);
        std::cout << "Server waiting for connection on TCP port " << port << "\n";
    } 
   
    std::cout << "Press 'Ctrl + C' to remove virtual device and exit\n";

    while (running) {
        auto cmd = receiver->next_command();
        if (cmd.has_value()) {
            gamePad.process_command(*cmd);
        }
    }

    std::cout << "\nExiting...\n";
}