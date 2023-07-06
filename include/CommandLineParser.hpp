// Extracted from https://gitlab.crans.org/phenixceleste/vatt/
// (one my other school projects)

// -*- lsst-c++ -*-

#ifndef _COMMAND_LINE_PARSER_HPP_
#define _COMMAND_LINE_PARSER_HPP_

#include <map>
#include <optional>
#include <string>

/**
 * A minimalist command-line arguments parser.
 *
 */
class CommandLineParser {
  private:
    std::map<std::string, std::optional<std::string>> values;

  public:
    /**
     * Parse the command-line arguments.
     * Assuming 'arg1' takes no value and 'arg2' takes a value, the expected
     * format is:
     * ./vatt arg1 arg2=value2
     *
     * @param argc Number of arguments
     * @param argv Low-level array of arguments
     */
    void parse(int argc, char *argv[]);

    /**
     * Indicates whether the parameter with the given name has been set.
     * This does not imply that the parameter has a value.
     */
    bool isSet(const std::string &name) const;

    /**
     * Get the value associated to the parameter with the given name.
     *
     * @tparam T The expected type of the value. Only int and std::string are
     * supported.
     * @throw If the parameter has not been set, has no value or T = int and its
     * value cannot be interpreted as an integer.
     */
    template <typename T> const T getValue(const std::string &name) const;

    /**
     * Try to get the value associated to the parameter with the given name.
     * Return defaultValue if the parameter is not set, has no value or T = int
     * and its value cannot be interpreted as an integer.
     *
     * @tparam T The expected type of the value. Only int and std::string are
     * supported.
     */
    template <typename T> const T optValue(const std::string &name, const T &defaultValue) const;
};

#endif