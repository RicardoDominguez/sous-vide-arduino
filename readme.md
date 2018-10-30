# Sous Vide Cooker - Arduino + Slow Cooker

Turn a standard slow cooker into a sous vide cooker with precise temperature control!

- [Introduction](#introduction)
- [Hardware](#hardware)
- [Libraries required](#libraries-required)

# Introduction

Sous-vide is a method of cooking in which food is placed in a plastic pouch or a glass jar and cooked in a water bath or steam environment for longer than normal cooking times (usually 1 to 7 hours, up to 48 or more in some cases) at an accurately regulated temperature. The temperature is much lower than normally used for cooking, typically around 55 to 60 °C for meat, higher for vegetables. The intent is to cook the item evenly, ensuring that the inside is properly cooked without overcooking the outside, and to retain moisture.

A standard cheap slow cooker can be converted to a sous vide cooker by adding temperature control to maintain the temperature at a desired setpoint during cooking. Here an Arduino is used to perform PID temperature control by sensing the water temperature inside the slow cooker and using a relay to switch on and off the power to the slow cooker in order to maintain the desired water temperature. A simple 16x2 LCD Display with Buttons is used to interface with the controller and display information such as water temperature or cooking time elapsed.

# Hardware

 * Arduino Uno or equivalent
 * 16x2 LCD Display with Buttons
 * Waterproof temperature sensor ([DS18B20](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf))
 * Power socket relay (I used [this](https://energenie4u.co.uk/index.php/catalogue/product/ENER002-2PI) remote one but others will also work)

# Libraries required

The contents of `libraries\` must be placed on `$YOUR_ARDUINO_PATH$\libraries\`.

Additional libraries required:
  * [`OneWire`](https://www.arduinolibraries.info/libraries/one-wire)
  * [`DallasTemperature`](https://www.arduinolibraries.info/libraries/dallas-temperature)
  * [`LiquidCrystal`](https://www.arduinolibraries.info/libraries/liquid-crystal)
  * [`PID_v1`](https://www.arduinolibraries.info/libraries/pid)
