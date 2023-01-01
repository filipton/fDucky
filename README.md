# fDucky
"Rubber Ducky" like hid device using Seeed XIAO RP2040.

Written using arduino core bc of weird error with normal pico-c-sdk 
(flash memory was just crashing my board).


## Usage
I'll write better tutorial later, but now you can try it.

### Flashing and preparation of board
 - Download ArduinoIDE (i really recommend v2.0)
 - Run it and install [arduino-pico](https://github.com/earlephilhower/arduino-pico#installing-via-arduino-boards-manager) using Arduino Boards Manager
 - Clone repo and open it
 - Download other required libraries
 - Flash your board selecting proper board config (Change USB Stack to TinyUSB and Flash Size to 1mb Sketch / 1mb fs)
 - Now you can solder little push button from [Pin D10](https://files.seeedstudio.com/wiki/XIAO-RP2040/img/xinpin.jpg) to GND

### Real usage
 - Connect Seeed XIAO RP2040 to your pc while holding new push button (D10-GND)
 - Now your pc should detect USB Storage - you should format it in FAT
 - Create folder named `payloads`
 - Create file named `selected.txt`
 - Now you can create files in your `payloads` folder and tell the board which payload to use writing file name (with ext) in `selected.txt`


## Changing board name
While connected to PC our "fDucky" has "Seeed XIAO RP2040" name in device manager (e.g. lsusb).

To change that we need to reinstall [arduino-pico](https://github.com/earlephilhower/arduino-pico#installing-via-git) now using GIT as our method.

Then you can change boards.txt file which is located in ~/Arduino/hardware/pico

```
seeed_xiao_rp2040.build.usb_manufacturer="Seeed"
seeed_xiao_rp2040.build.usb_product="XIAO RP2040"
```

In that way you can modify "Arduino build behaviour" and set new manafacturer and product name. 


## Error codes
| 1 | 2 | 3 | Explanation |
|---|---|---|---|
| :large_blue_circle: | :large_blue_circle: | :large_blue_circle: | FS not formatted |
| :red_circle: | :red_circle: | :red_circle: | `payloads` folder doesn't exists in fs root  |
| :red_circle: | :red_circle: | :large_blue_circle: | `selected.txt` file doesn't exists in fs root (or there is a problem with reading that file) |
| :red_circle: | :large_blue_circle: | :red_circle: | selected payload doesn't exists in `payloads` directory |