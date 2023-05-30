#ifndef _JOYSTICK_HPP_
#define _JOYSTICK_HPP_

#include "Fallible.hpp"
#include "Command.hpp"

#include <exception>
#include <string> //strcpy, strerror

/// A virtual GamePad device. Linux only.
/// The device is emulated using '/dev/uinput'.
class VirtualGamePad : Fallible {
  private:
    int file;
    std::string _raw_name;
    std::string _event_file;

    std::string parseEventFile() const;
    /// Emits a single input event for this virtual device. 
    inline void emit(int type, int code, int val);
    /// Emits a 'SYN' event to report an update caused by a previous input event.
    inline void report_sync();
    /// Simulates a button press by emitting a 'button down' then a 'button up' event.
    inline void button_press(int code);
    /// If 'action' is 'ACTION_PRESS_RELEASE', delegates to 'button_press',
    /// otherwise delegates to 'button_simple_action'
    inline void button_action(int code, int action);
    /// Emits a 'button down' or a 'button up', then a 'SYN' event.
    inline void button_simple_action(int code, int action);

  public:
    VirtualGamePad();
    ~VirtualGamePad();

    /// Gets the path of the file that can be read to receive input events from this virtual device.
    const std::string getEventFile() const;
    /// Gets the name that internally represents the virtual device,
    const std::string getRawDeviceName() const;
    /// Processes a command, possibly causing the virtual device to emit gamepad input events.
    /// If the event is invalid or unsupported, it is ignored and no exception is thrown.
    void process_command(const Command &command);

};

#endif