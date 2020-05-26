# XKeys HID on Mac

Just messing around with [IOKit][url-iokit] in an attempt to write a command-line program that can toggle individual LEDs on my [X-keys 16-key programmable keypad][url-xkeys-xk-16].

Why?

Programming the keypad in hardware mode is the most portable since the logic lives on the keypad itself. While there *are* hareware-level functions to set the state of a specific LED to either **on** or **off**, there's no hardware-level function to toggle an individual LED. That means the state of an individual LED has to live in software.

As for software, the X-keys website recommends [ControllerMate][url-controllermate] on macOS. ControllerMate *can* toggle individual LEDs, but doesn't recognize all of the keys. Other programs like [Keyboard Maestro][url-keyboard-maestro] *do* recognize all of the keys, but don't have the ability to toggle individual LEDs.

So the plan is to write a command-line utility that can toggle individual LEDs and then tell a program like *Keyboard Maestro* to invokve the utility when a key is pressed.

## ControllerMate

ControllerMate isn't maintained and doesn't officially support macOS Catalina, which has an entirely new permissions/privacy framework. The program *does* appear to run, but for some reason it only recognizes every other pair of keys.

## Installing

1. Clone this repo and run `make` inside the project directory
1. Plug in your XK-16

Assuming `make` worked, there are two programs in `bin/`:

- `bin/read-xkeys` will display the raw bytes sent by your X-keys keypad (useful for debugging)
- `bin/write-keys` will send raw bytes to your XK-16

See the [XK-16 data report][url-xk-16-data-report] for details on the different commands the XK-16 understands (these commands are called "reports" in USB-ese). For example, to turn the third LED on, you'd run:

```console
bin/write-xkeys 181 2 1
```

Why? Because...

- The `181` / `0xB5` byte corresponds to the **Index Based Set Backlights (Flash)** command
- The `2` byte corresponds to the 0-based index of the LED whose state you want to modify
- The `1` byte corresponds to setting the LED to **on**

You could then turn it off with:

```console
bin/write-keys 181 2 0
```

[url-iokit]: https://developer.apple.com/documentation/iokit
[url-xkeys-xk-16]: https://xkeys.com/xk16.html
[url-controllermate]: https://www.orderedbytes.com/controllermate/
[url-keyboard-maestro]: https://www.keyboardmaestro.com/main/
[url-xk-16-data-report]: https://www.collem.nl/x-keys/pages/X-keys%20XK-16%20XK-8%20XK-4%20Stick%20Data%20Report.htm
