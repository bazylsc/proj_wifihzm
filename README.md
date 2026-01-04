How to build PlatformIO based project
=====================================

1. [Install PlatformIO Core](https://docs.platformio.org/page/core.html)
2. Download [development platform with examples](https://github.com/platformio/platform-espressif32/archive/develop.zip)
3. Extract ZIP archive
4. Run these commands:

```shell
# Change directory to example
$ cd platform-espressif32/examples/espidf-hello-world

# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Build specific environment
$ pio run -e esp32dev

# Upload firmware for the specific environment
$ pio run -e esp32dev --target upload

# Clean build files
$ pio run --target clean
```


;
; Please visit documentation for the other options and examples
; https://docs.platformio.org/page/projectconf.html

; [env]
; platform = espressif32
; framework = espidf
; monitor_speed = 115200

[env:esp32s3usbotg]
platform = espressif32
;platform = https://github.com/platformio/platform-espressif32
board = esp32s3usbotg
framework = espidf
debug_tool = esp-builtin
build_type = debug
monitor_speed = 115200
;upload_speed = 921600
;upload_port = /dev/ttyUSB0
monitor_port = /dev/ttyACM1

; [env:esp32dev]
; board = esp32dev

; [env:esp32-s2-kaluga-1]
; board = esp32-s2-kaluga-1

; [env:esp32-c3-devkitm-1]
; board = esp32-c3-devkitm-1

; [env:esp32-c6-devkitc-1]
; board = esp32-c6-devkitc-1
; board_build.cmake_extra_args = 
;     -DSDKCONFIG_DEFAULTS="sdkconfig.defaults.esp32c6"


ota
https://www.youtube.com/watch?v=QhnLKu6tmLg
