#include "Command.hpp"

#include <stdexcept>

Command::Command(CommandType type, int value, const std::string &raw_type)
    : type(type), value(value), raw_type(raw_type) {}

const Command Command::parse(const std::string &cmd) noexcept {
    size_t i = cmd.find(" ");
    std::string type_str = (i == std::string::npos) ? cmd : cmd.substr(0, i);

    int val = 0;
    if (i != std::string::npos) {
        std::string val_str = cmd.substr(i + 1);
        try {
            val = std::stoi(val_str);
        } catch (std::logic_error &ex) {
            return Command(CommandType::Invalid);
        }
    }

    CommandType type = CommandType::Invalid;
    if (type_str == "X") {
        type = CommandType::X;
    } else if (type_str == "Y") {
        type = CommandType::Y;
    } else if (type_str == "RX") {
        type = CommandType::RX;
    } else if (type_str == "RY") {
        type = CommandType::RY;
    } else if (type_str == "BSTART") {
        type = CommandType::BSTART;
    } else if (type_str == "BSELECT") {
        type = CommandType::BSELECT;
    }

    return Command(type, val, type_str);
}