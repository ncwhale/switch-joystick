# switch-joystick

## Build & Flash

```
git clone https://github.com/ncwhale/switch-joystick
git clone https://github.com/abcminiuser/lufa LUFA
cd switch-joystick
make # build
# Press the reset button on your borad.
# Replace the tty port to your chip port. use `dmesg` to show it.
AVRDUDE_PORT=/dev/ttyACM0 make avrdude # Flash
```

# Refer

* [shinyquagsire23/Switch-Fightstick](https://github.com/shinyquagsire23/Switch-Fightstick)