#include "Gamepad.hpp"
#include "IOException.hpp"

#include <fcntl.h> //open
#include <fstream>
#include <iostream>       // std::cout
#include <linux/uinput.h> //ioctl
#include <math.h>         //cos
#include <string.h>       //strcpy
#include <unistd.h>       //close, symlink

#define SYMLINK_PATH                                                           \
    "/dev/input/by-path/usb-EMan_Virtual_GamePad-event-joystick"
#define REMOVE_SYMLINK try_opt(std::remove(SYMLINK_PATH), "remove symlink")

VirtualGamePad::VirtualGamePad() {
    file = check(open("/dev/uinput", O_WRONLY | O_NONBLOCK));

    check(ioctl(file, UI_SET_EVBIT, EV_ABS));
    check(ioctl(file, UI_SET_ABSBIT, ABS_X));
    check(ioctl(file, UI_SET_ABSBIT, ABS_Y));
    check(ioctl(file, UI_SET_ABSBIT, ABS_RX));
    check(ioctl(file, UI_SET_ABSBIT, ABS_RY));

    check(ioctl(file, UI_SET_EVBIT, EV_KEY));
    check(ioctl(file, UI_SET_KEYBIT, BTN_SELECT));
    check(ioctl(file, UI_SET_KEYBIT, BTN_START));

    /*
        check(ioctl(file, UI_SET_KEYBIT, BTN_SOUTH));
        check(ioctl(file, UI_SET_KEYBIT, BTN_EAST));
        check(ioctl(file, UI_SET_KEYBIT, BTN_NORTH));
        check(ioctl(file, UI_SET_KEYBIT, BTN_WEST));

        check(ioctl(file, UI_SET_KEYBIT, BTN_DPAD_UP));
        check(ioctl(file, UI_SET_KEYBIT, BTN_DPAD_DOWN));
        check(ioctl(file, UI_SET_KEYBIT, BTN_DPAD_LEFT));
        check(ioctl(file, UI_SET_KEYBIT, BTN_DPAD_RIGHT));

        check(ioctl(file, UI_SET_KEYBIT, BTN_TL));
        check(ioctl(file, UI_SET_KEYBIT, BTN_TR));
        check(ioctl(file, UI_SET_KEYBIT, BTN_TL2));
        check(ioctl(file, UI_SET_KEYBIT, BTN_TR2));
    */

    uinput_setup dev_settings = {0};
    dev_settings.id.bustype = BUS_USB;
    strcpy(dev_settings.name, "E__Man virtual gamepad");

    check(ioctl(file, UI_DEV_SETUP, &dev_settings));
    check(ioctl(file, UI_DEV_CREATE));

    char device_name[16];
    check(ioctl(file, UI_GET_SYSNAME(sizeof(device_name)), device_name));
    _raw_name = std::string(device_name);

    _event_file = parseEventFile();
    REMOVE_SYMLINK;
    try_opt(symlink(("/dev/input/" + _event_file).c_str(), SYMLINK_PATH),
            "create symlink");
}

VirtualGamePad::~VirtualGamePad() {
    if (file > -1) {
        try_opt(ioctl(file, UI_DEV_DESTROY), "destroy device");
        try_opt(close(file), "close file");
        REMOVE_SYMLINK;
    }
}

const std::string VirtualGamePad::getEventFile() const { return _event_file; }
const std::string VirtualGamePad::getRawDeviceName() const { return _raw_name; }

std::string VirtualGamePad::parseEventFile() const {
    std::ifstream devices;
    devices.exceptions(std::ifstream::badbit);
    devices.open("/proc/bus/input/devices", std::ios::in);

    std::string line;
    std::string target("S: Sysfs=/devices/virtual/input/" + _raw_name);

    while (std::getline(devices, line) && line != target)
        ;
    while (std::getline(devices, line) &&
           line.rfind("H: Handlers=", 0) == std::string::npos)
        ;

    line = line.substr(12);
    line = line.substr(0, line.find(" "));

    return line;
}

inline void VirtualGamePad::emit(int type, int code, int val) {
    input_event event;
    event.type = type;
    event.code = code;
    event.value = val;

    check(write(file, &event, sizeof(event)));
}

inline void VirtualGamePad::report_sync() { emit(EV_SYN, SYN_REPORT, 0); }
inline void VirtualGamePad::button_press(int code) {
    button_simple_action(code, ACTION_DOWN);
    button_simple_action(code, ACTION_UP);
}
inline void VirtualGamePad::button_simple_action(int code, int action) {
    emit(EV_KEY, code, action);
    report_sync();
}
inline void VirtualGamePad::button_action(int code, int action) {
    if (action == ACTION_PRESS_RELEASE) {
        button_press(code);
    } else if (action == ACTION_DOWN || action == ACTION_UP) {
        button_simple_action(code, action);
    }
}

void VirtualGamePad::process_command(const Command &command) {
    switch (command.type) {
    case Command::CommandType::X:
        emit(EV_ABS, ABS_X, command.value);
        break;
    case Command::CommandType::Y:
        emit(EV_ABS, ABS_Y, command.value);
        report_sync();
        break;
    case Command::CommandType::RX:
        emit(EV_ABS, ABS_RX, command.value);
        break;
    case Command::CommandType::RY:
        emit(EV_ABS, ABS_RY, command.value);
        report_sync();
        break;
    case Command::CommandType::BSELECT:
        button_action(BTN_SELECT, command.value);
        break;
    case Command::CommandType::BSTART:
        button_action(BTN_START, command.value);
        break;
    default:
        break;
    }
}