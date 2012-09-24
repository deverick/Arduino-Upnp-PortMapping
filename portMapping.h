// UPNP IGD Port Mapping	Library v1.0 - August 8th 2012
// Author: Deverick McIntyre

#ifndef PortMapping_h
#define PortMapping_h

#include <Ethernet.h>
#include "EthernetUdp.h"
#include "locateIgd.h"

/* Port Mapping state machine. */
#define STATE_PORTMAP_START 0
#define	STATE_PORTMAP_WAIT_RESPONSE	1
#define	STATE_PORTMAP_CONFIRMED	2

/* Default Port values */
#define RAND_PORT_RANGE_MIN 50000
#define RAND_PORT_RANGE_MAX 59000

const unsigned long PM_TOTAL_TIMEOUT = 300000;
const unsigned long PM_RESPONSE_TIMEOUT = 50000;

/* HTTP add Port Map Request Packet text */
const prog_char PM_HEADER1A[] PROGMEM = "POST ";
const prog_char PM_HEADER1B[] PROGMEM = " HTTP/1.1\r\nSOAPACTION: \"urn:schemas-upnp-org:service:WANIPConnection:1#AddPortMapping\"\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nHOST: ";
const prog_char PM_HEADER1C[] PROGMEM = "\r\nContent-Length: 702\r\n\r\n";

const prog_char PM_BODY1[] PROGMEM = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
const prog_char PM_BODY2[] PROGMEM = "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n";
const prog_char PM_BODY3[] PROGMEM = "   <s:Body>\r\n";
const prog_char PM_BODY4[] PROGMEM = "      <u:AddPortMapping xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\r\n";
const prog_char PM_BODY5[] PROGMEM = "         <NewRemoteHost />\r\n";
const prog_char PM_BODY6A[] PROGMEM = "         <NewExternalPort>";
const prog_char PM_BODY6B[] PROGMEM = "</NewExternalPort>\r\n";
const prog_char PM_BODY7[] PROGMEM = "         <NewProtocol>TCP</NewProtocol>\r\n";
const prog_char PM_BODY8A[] PROGMEM = "         <NewInternalPort>";
const prog_char PM_BODY8B[] PROGMEM = "</NewInternalPort>\r\n";
const prog_char PM_BODY9A[] PROGMEM = "         <NewInternalClient>";
const prog_char PM_BODY9B[] PROGMEM = "</NewInternalClient>\r\n";
const prog_char PM_BODY10[] PROGMEM = "         <NewEnabled>1</NewEnabled>\r\n";
const prog_char PM_BODY11[] PROGMEM = "         <NewPortMappingDescription>Arduino</NewPortMappingDescription>\r\n";
const prog_char PM_BODY12[] PROGMEM = "         <NewLeaseDuration>0</NewLeaseDuration>\r\n";
const prog_char PM_BODY13[] PROGMEM = "      </u:AddPortMapping>\r\n";
const prog_char PM_BODY14[] PROGMEM = "   </s:Body>\r\n";
const prog_char PM_BODY15[] PROGMEM = "</s:Envelope>";


/* HTTP get external IP request Packet text */
const prog_char EXTIP_HEADER1A[] PROGMEM = "POST ";
const prog_char EXTIP_HEADER1B[] PROGMEM = " HTTP/1.1\r\nSOAPACTION: \"urn:schemas-upnp-org:service:WANIPConnection:1#GetExternalIPAddress\"\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nHOST: ";
const prog_char EXTIP_HEADER1C[] PROGMEM = "\r\nContent-Length: 297\r\n\r\n";
const prog_char EXTIP_BODY1[] PROGMEM = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
const prog_char EXTIP_BODY2[] PROGMEM = "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n";
const prog_char EXTIP_BODY3[] PROGMEM = "   <s:Body>\r\n";
const prog_char EXTIP_BODY4[] PROGMEM = "      <u:GetExternalIPAddress xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\" />\r\n";
const prog_char EXTIP_BODY5[] PROGMEM = "   </s:Body>\r\n";
const prog_char EXTIP_BODY6[] PROGMEM = "</s:Envelope>";

// NOTIFY and HTTP packet parsing strings
  const prog_char HTTP_RESPONSE[13] PROGMEM = "HTTP/1.1 200";
  const prog_char XML_TAG_OPEN[2] PROGMEM = "<";
  const prog_char XML_TAG_EXTIP[22] PROGMEM = "NewExternalIPAddress>";

class PortMapClass {
private:
  uint8_t _portmap_state;
  MSearchClass msearch;
  uint16_t _portmapInternalPort;
  uint16_t _portmapExternalPort;
  IPAddress _externalIp;
  uint16_t _portmapRandomPort;
  EthernetClient _apmClient;
  IPAddress _igdIp;
  uint16_t _igdPort;
  String _igdControlUrl;

  void reset();
  int addPortMapRequest(IPAddress myIP, uint16_t requestedIntPort, uint16_t requestedExtPort);
  int parsePortMapResponse(uint16_t requestedIntPort, uint16_t requestedExtPort);
  int getExternalIpRequest();
  int parseExternalIpResponse();
  IPAddress igdIp();
  uint16_t igdPort();
  String igdControlUrl();
  int getIgdParams();

public:
  int addPortMap(IPAddress myIP);
  int addPortMap(IPAddress myIP, uint16_t requestedIntPort, uint16_t requestedExtPort);
  uint16_t internalPort();
  uint16_t externalPort();
  IPAddress externalIp();

};

#endif