# Dice logging

This is the repo containing my project during the 2021 math camp.

The project is built using an ESP8266 and accelerometer.

To upload the code use the ESP8266 package for the arduino IDE. 

Add this to `File > Preferences > Additional Board Manager URLs` (http://arduino.esp8266.com/stable/package_esp8266com_index.json]. Goto `Tools > Board > Board Manager` and install the ESP8266 module. Select `Generic ESP8266 Module` in `Tools > Board`.

Goto `Sketch > Include Library > Manage Libraries` and install the NTPClient. Install the Adafruit ADXL_345 library aswell.

The ClockifyAPI documentation can be found at: (https://clockify.me/developers-api)[!https://clockify.me/developers-api]

## Model
The model can be built using openSCAD. It was sliced with the PrusaSlicer at 0.15mm layer height.

If you wish to only have one model, change the following code in 'dice_cad.scad':
```cpp
for (i = [0:1]) {
    upper = i;
    translate([(-rad -2) * sign(i - 0.5), 0, 0]) {
    translate([0,0,0]) {
        union() {
```

```cpp
                      }
                  }
                  }
              }
          }
```

To this:
```cpp
for (i = [0:0]) { // Or 'for (i = [1:1]) {' to get the other half.
    upper = i;
    //translate([(-rad -2) * sign(i - 0.5), 0, 0]) {
    translate([0,0,0]) {
        union() {
```


```cpp
                      }
                  }
    //              }
              }
          }
```

## Code
In the file './dice-logger/api_code.h' you must fill in your details.

```cpp
String api_code = "Your api code here";
String workspace_id = "Your workspace id here";
```

If something isn't working you probably want to enable DEBUGMODE where it sends a lot of prints over Serial. Set the variable to true and upload the new code.
The `project_GET_page_size` defines how many projects it will request at once.
The `last_sides_length` defines for how many updates the die will have to be in the same position before it updates. It updates about 8 times per second, making this value divided by 8 the number of seconds of the same rotation required.
```cpp
#define last_sides_length 8
#define project_GET_page_size 12
#define DEBUGMODE false
```

The source-code also needs to contain a list of all wifi access-points it can connect to. These can be added with the line:
```cpp
wifiMulti.addAP("SSID", "password");
```

## Connections
I will maybe add a proper schematic here later, for now I will just write it down to document it.

### Between the accelerometer and ESP8266
Accel: GND -> ESP8266: GND
Accel: VCC -> ESP8266: 5V
Accel: SCL -> ESP8266: D1
Accel: SDA -> ESP8266: D2

### Between the ESP8266 and the turn-off button
ESP8266: 3.3V -> Button (side 1) -> Button (side 2) -> ESP8266: D5 and -> 100KOhm -> ESP8266: GND


By: Elias Lundell
