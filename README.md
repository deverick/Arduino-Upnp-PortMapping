Arduino-Upnp-PortMapping
========================

UPNP based portmapping function for the Arduino

## Implementation

		void setup() {
			
			/* Ethernet and Serial setup code goes here ... */

			Serial.println(portmap.externalIp());

			portmap.addPortMap(192.168.0.50, 80, 2900);

		}

## Background

For those of use who create Arduino server type programs using the Ethernet shield this project may help you.  Generally there is a limitation when connecting to your Arduino that you need to be on the same LAN.  You are not able to connect to the device via the internet.  This is because the Arduino is given a private IP address on your internal network, it has no public IP address.

I had this problem and found there was really only one solution which will allow me to connect to the Arduino remotely.  The solution involves using the UPNP functionality which is built into most modern wireless routers.  UPNP, and more specifically a sub-protocol called Internet-Gateway-Device allows any device on the local network to add a port mapping to the router, thereby allowing internet resources to access the device by calling the router's WAN (public) IP address and the mapped external port number.  The router will pass the packet to the internal IP address and the mapped internal port number.

The library allows you to: (a) query the external (public) IP address, and (b) add a port mapping entry on the router.  Once you have these you can pass them to an external resource which thereafter can connect to the Arduino on this socket (IP address + port number).  ISPs dynamically assign public addressses so be aware yours could change anytime.  Therefore you could implement this solution with a DDNS solution in order to fix the domain name.

Unfortunately including this port mapping library uses most of the resources of the Arduino, limiting your ability to do complex sketches.

## Installation

The functionality has been created as a C++ library for Arduino.  This library can be added to the Arduino libraries as follows:
1. Open the 'libraries' folder under your main Arduino folder.
2. Create a new folder 'PortMapping'.
3. Clone this repository into that folder
4. Restart the Arduino IDE so that you can see the example sketch in the examples menu 'Examples > PortMapping > addPortMapping'.

## Known Issues

The UPNP standard is based on the SOAP/XML protocol.  XML messages are very verbose, and not ideal for a simple MCU which the Arduino uses. Creating, sending, receiving and parsing SOAP/XML messages uses up most of the Arduino's extremely limited resources.

This code has been designed and tested with 3 different routers.  UPNP implementation across different hardware appears to have slight variances.  Due to the simplistic way XML parsing is implemented in this code it is quite possible that this code will not work correctly with other routers.  If this is case I am extremely interested in getting feedback and/or code updates to improve the code

## Tested Router Models

1. TP-LINK TL-WR541G
2. D-LINK DI-624+A
3. D-LINK DIR-605

** Please send feedback about other models.

## Usage example

See Arduino IDE Example