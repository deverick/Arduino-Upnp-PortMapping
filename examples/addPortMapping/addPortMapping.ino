/*
   Add Port Mapping to UPNP device port mapping table
 
 This example sketch uses the UPNP functionality in most modern commercial routers to add a port mapping.  
 The port mapping allows an external service to contact the LAN based device via the mapped port. 
 Logic Steps:
 1) Send an M-SEARCH broadcast request for a UPNP compliant Internet Gateway Device.
 2) Parse any device IGD request response for control URL and port number for the IGD
 3) Send an Add Port Mapping request to the IGD
 4) Optionally request the IGD for the external (public) IP address of the IGD
 
  
 created 08 Aug 2012
 by Deverick McIntyre
 version 1.0
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include "portMapping.h"

//declare the client object here so it is global in scope so we can call it in functions in order to print
EthernetClient client;

PortMapClass portmap;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network, only used if DHCP fails
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0, 177);


  /*
  core Arduino functions
  */

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
 
  //send first message after serial port is connected
  Serial.println(F("Initializing... "));
  
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    // DHCP failed, so use a fixed IP address:
    Ethernet.begin(mac, ip);
    Serial.println(F("Fixed IP initialization complete"));
  }
  else
  {
    Serial.println(F("Configured Ethernet using DHCP"));
  }

  Serial.print(F("device IP is: "));
  Serial.println(Ethernet.localIP());
  
  Serial.print(F("gateway IP is: "));
  Serial.println(Ethernet.gatewayIP());
  
  Serial.print(F("subnet mask is: "));
  Serial.println(Ethernet.subnetMask());

  Serial.print(F("DNS is: "));
  Serial.println(Ethernet.dnsServerIP());

  Serial.print(F("External IP is: "));
  Serial.println(portmap.externalIp());


   if (portmap.addPortMap(Ethernet.localIP(),80, 2900))  // add port mapping for user defined port values
   //if (portmap.addPortMap(Ethernet.localIP()))  // add port mapping for random port values
 {
   Serial.println(F("Port Mapped successfully!"));
   Serial.print(F("Internal Port: "));
   Serial.println(portmap.internalPort());
   Serial.print(F("External Port: "));
   Serial.println(portmap.externalPort());
 } else {
   Serial.println(F("unable to map port.  Check your router UPNP function is turned on"));
 }

}


void loop() {
  
 delay(4000);
 
}
