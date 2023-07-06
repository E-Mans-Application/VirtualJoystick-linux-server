#include "CommandLineParser.hpp"
#include <iterator>

void CommandLineParser::parse(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        std::string::size_type e = arg.find("=");
        if (e == std::string::npos) {
            values[arg] = std::optional<std::string>();
        } else {
            std::string name = arg.substr(0, e);
            std::string value = arg.substr(e + 1);
            values[name] = std::optional<std::string>(value);
        }
    }
}

bool CommandLineParser::isSet(const std::string &name) const { return values.count(name) == 1; }

template <typename T> const T CommandLineParser::getValue(const std::string &name) const {
    throw std::logic_error("Invalid type parameter.");
}
template <> const std::string CommandLineParser::getValue(const std::string &name) const {
    return values.at(name).value();
}
template <> const int CommandLineParser::getValue(const std::string &name) const {
    return std::stoi(getValue<std::string>(name));
}

template <typename T>
const T CommandLineParser::optValue(const std::string &name, const T &defaultValue) const {
    throw std::logic_error("Invalid type parameter.");
}

template <>
const std::string CommandLineParser::optValue(const std::string &name,
                                              const std::string &defaultValue) const {
    auto it = values.find(name);
    if (it != values.end() && it->second) {
        return *it->second;
    } else {
        return defaultValue;
    }
}
template <>
const int CommandLineParser::optValue(const std::string &name, const int &defaultValue) const {
    auto it = values.find(name);
    if (it != values.end() && it->second) {
        try {
            return std::stoi(*it->second);
        } catch (std::logic_error &e) {
            return defaultValue;
        }
    } else {
        return defaultValue;
    }
}
