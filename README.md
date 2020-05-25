# XKeys HID on Mac

Just messing around with [IOKit][url-iokit] in an attempt to write a command-line program that can toggle individual LEDs on my [X-keys 16-key programmable keypad][url-xkeys-xk-16].

Why?

Programming the keypad in hardware mode is the most portable since the logic lives on the keypad itself. While there *are* hareware-level functions to set the state of a specific LED to either **on** or **off**, there's no hardware-level function to toggle an individual LED. That means the state of an individual LED has to live in software.

As for software, the X-keys website recommends [ControllerMate][url-controllermate] on macOS. ControllerMate *can* toggle individual LEDs, but doesn't recognize all of the keys. Other programs like [Keyboard Maestro][url-keyboard-maestro] *do* recognize all of the keys, but don't have the ability to toggle individual LEDs.

So the plan is to write a command-line utility that can toggle individual LEDs and then tell a program like *Keyboard Maestro* to invokve the utility when a key is pressed.

## ControllerMate

ControllerMate isn't maintained and doesn't officially support macOS Catalina, which has an entirely new permissions/privacy framework. The program *does* appear to run, but for some reason it only recognizes every other pair of keys.

[url-iokit]: https://developer.apple.com/documentation/iokit
[url-xkeys-xk-16]: https://xkeys.com/xk16.html
[url-controllermate]: https://www.orderedbytes.com/controllermate/
[url-keyboard-maestro]: https://www.keyboardmaestro.com/main/
