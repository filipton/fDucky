# fDucky
"Rubber Ducky" like hid device using Seeed XIAO RP2040. <br>
Using arduino core bc of weird error with normal pico-c-sdk 
(flash memory was just crashing my board).

## Error codes
| 1 | 2 | 3 | Explanation |
|---|---|---|---|
| :large_blue_circle: | :large_blue_circle: | :large_blue_circle: | FS not formatted |
| :red_circle: | :red_circle: | :red_circle: | `payloads` folder doesn't exists in fs root  |
| :red_circle: | :red_circle: | :large_blue_circle: | `selected.txt` file doesn't exists in fs root (or there is a problem with reading that file) |
| :red_circle: | :large_blue_circle: | :red_circle: | selected payload doesn't exists in `payloads` directory |