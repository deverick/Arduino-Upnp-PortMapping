Arduino-Upnp-PortMapping
========================

UPNP based portmapping function for the Arduino

## Installation

The functionality has been created as a C++ library for Arduino.  This library can be added to the Arduino libraries as follows:
1) Open the 'libraries' folder under your main Arduino folder.
2) Create a new folder 'PortMapping'.
3) Clone this repository into that folder
4) Restart the Arduino IDE so that you can see the example sketch in the examples menu 'Examples > PortMapping > addPortMapping'.

## Known Issues

The UPNP standard is based on the SOAP/XML protocol.  XML messages are very verbose, and not ideal for a simple MCU which the Arduino uses. Creating, sending, receiving and parsing SOAP/XML messages uses up most of the Arduino's extremely limited resources.

This code has been designed and tested with 3 different routers.  UPNP implementation across different hardware appears to have slight variances.  Due to the simplistic way XML parsing is implemented in this code it is quite possible that this code will not work correctly with other routers.  If this is case I am extremely interested in getting feedback and/or code updates to improve the code

## Tested Router Models

1. TP-LINK TL-WR541G
2. D-LINK DI-624+A
3. D-LINK DIR-605

** Please send feedback about other models.

## Usage example

        #the blink program

        #import