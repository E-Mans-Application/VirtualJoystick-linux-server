#ifndef _COMMAND_HPP_
#define _COMMAND_HPP_

#include <linux/input-event-codes.h>
#include <string>

#define ACTION_UP 0
#define ACTION_DOWN 1
#define ACTION_PRESS_RELEASE 2

class Command {
  public:
    enum CommandType {
        Invalid,

        X,
        Y,
        RX,
        RY,

        BSELECT,
        BSTART,
        // Other buttons not implemented yet (because they are unused)
    };

    CommandType type;
    /// An arbitrary, unchecked integer value associated with the command. The interpretation
    /// depends on the command type.
    int value = 0;
    /// If this field is a non-empty string, it contains the raw, unparsed
    /// representation of the command type used for parsing (without the integer
    /// value). Note that there is no warranty that this field is actually populated,
    /// even if this command was parsed from a string.
    std::string raw_type;

    Command(CommandType type, int value = 0, const std::string &raw_type = "");

    /// Parses a command.
    /// @param cmd The string representation of the command.
    /// The format should be 'CMD' or 'CMD value' (with 'value' being an
    /// integer), without a command-termination char.
    /// @throws This function does not throw. Should a parse error occur, this
    /// function would return a command with type 'Invalid' (and unspecified
    /// 'value' and 'raw_type' instead)
    static const Command parse(const std::string &cmd) noexcept;
};

#endif