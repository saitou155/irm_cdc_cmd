# irm_cdc_cmd

irMagician commands that run on a general-purpose PC

## Getting Started

This is program for Windows(MinGW)/Ubuntu/OSX.

### Prerequisites

install libserialport and libjson-c for build. 

```
packman -S mingw-w64-i686-json-c (32bit)
packman -S mingw-w64-x86_64-json-c (64bit)
     or
sudo apt-get install libserialport
sudo apt-get install libjson-c3 libjson-c-dev
     or
brew install libserialport
brew install json-c
```

## How to use

### Receive infrared remote control

When the reception of the infrared remote control succeeds, it is output to the standard output in JSON format.

```
saitou155@Endeavor-AT991E:~/irm_cdc_cmd$ ./irm_cdc_cmd -r
Capturing IR...
{ "format": "raw", "freq": 38, "data": [ 122, 1, 203, 50, 42, 4, 23, 4, 6, 6, 7, 4, 9, 4, 8, 4, 7, 4, 9, 5, 8, 4, 8, 5, 9, 4, 5, 7, 9, 4, 21, 4, 35, 4, 21, 4, 7, 5, 21, 4, 8, 5, 22, 4, 8, 5, 6, 5, 8, 5, 7, 5, 9, 4, 8, 4, 8, 5, 8, 4, 21, 5, 9, 4, 20, 4, 20, 5, 20, 5, 21, 5, 8, 4, 9, 4, 21, 5, 7, 5, 20, 5, 20, 5, 20, 5, 21, 5, 6, 6, 21, 5, 214, 52, 25, 6, 7, 5, 20, 6, 6, 6, 7, 5, 6, 6, 7, 6, 5, 7, 6, 6, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 6, 5, 7, 7, 5, 7, 6, 5, 7, 7, 6, 6, 6, 6, 6, 7, 5, 20, 6, 7, 5, 20, 5, 20, 6, 20, 6, 20, 6, 7, 5, 7, 6, 19, 6, 6, 6, 20, 5, 20, 6, 20, 6, 20, 6, 7, 5, 20, 5, 213, 52, 26, 6, 6, 6, 19, 6, 7, 5, 6, 6, 7, 6, 7, 5, 7, 6, 7, 5, 7, 6, 6, 5, 6, 6, 6, 6, 6, 6, 20, 6, 7, 5, 6, 6, 7, 5, 6, 6, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 6, 7, 5, 7, 6, 7, 5, 7, 5, 7, 5, 7, 6, 6, 6, 7, 5, 20, 6, 7, 6, 19, 6, 20, 6, 20, 6, 20, 6, 6, 5, 7, 5, 20, 6, 7, 5, 20, 6, 20, 6, 19, 6, 20, 6, 7, 5, 20, 6 ], "postscale": 100 }
```

To save the standard output JSON output to a file, do as follows.

```
./irm_cdc_cmd -r | tee panasonic_tv_power.json
```

### Transmit infrared remote control

For infrared remote control transmission, put JSON as an argument.

```
saitou155@Endeavor-AT991E:~/irm_cdc_cmd$ ./irm_cdc_cmd -t '{ "format": "raw", "freq": 38, "data": [ 122, 1, 203, 50, 42, 4, 23, 4, 6, 6, 7, 4, 9, 4, 8, 4, 7, 4, 9, 5, 8, 4, 8, 5, 9, 4, 5, 7, 9, 4, 21, 4, 35, 4, 21, 4, 7, 5, 21, 4, 8, 5, 22, 4, 8, 5, 6, 5, 8, 5, 7, 5, 9, 4, 8, 4, 8, 5, 8, 4, 21, 5, 9, 4, 20, 4, 20, 5, 20, 5, 21, 5, 8, 4, 9, 4, 21, 5, 7, 5, 20, 5, 20, 5, 20, 5, 21, 5, 6, 6, 21, 5, 214, 52, 25, 6, 7, 5, 20, 6, 6, 6, 7, 5, 6, 6, 7, 6, 5, 7, 6, 6, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 6, 5, 7, 7, 5, 7, 6, 5, 7, 7, 6, 6, 6, 6, 6, 7, 5, 20, 6, 7, 5, 20, 5, 20, 6, 20, 6, 20, 6, 7, 5, 7, 6, 19, 6, 6, 6, 20, 5, 20, 6, 20, 6, 20, 6, 7, 5, 20, 5, 213, 52, 26, 6, 6, 6, 19, 6, 7, 5, 6, 6, 7, 6, 7, 5, 7, 6, 7, 5, 7, 6, 6, 5, 6, 6, 6, 6, 6, 6, 20, 6, 7, 5, 6, 6, 7, 5, 6, 6, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 6, 7, 5, 7, 6, 7, 5, 7, 5, 7, 5, 7, 6, 6, 6, 7, 5, 20, 6, 7, 6, 19, 6, 20, 6, 20, 6, 20, 6, 6, 5, 7, 5, 20, 6, 7, 5, 20, 6, 20, 6, 19, 6, 20, 6, 7, 5, 20, 6 ], "postscale": 100 }'
Transfer IR...
```

To output the JSON file with this command, use the shell function as follows.

```
./irm_cdc_cmd -t "$(cat panasonic_tv_power.json)"
      or
./irm_cdc_cmd -t "`cat panasonic_tv_power.json`"
```

### Linux permission setting

Add a rule to udev In LINUX as follows:
```
# /etc/udev/rules.d/99-irusb.rules
#	bto_ir
SUBSYSTEMS=="usb", ATTRS{idVendor}=="22ea", ATTRS{idProduct}=="001e", GROUP="users", MODE="0666"
```

Update is as follows:
```
sudo udevadm control --reload-rules
```

## License

This project is Free software.


