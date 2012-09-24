// UPNP Internet Gateway Device locator - Library v1.0 - August 8th 2012
// Author: Deverick McIntyre

#ifndef locateIgd_h
#define locateIgd_h

#include <Ethernet.h>
#include "EthernetUdp.h"

/* Locate IGD state machine. */
#define STATE_MSEARCH_START 0
#define	STATE_MSEARCH_WAIT_RESPONSE	1
#define	STATE_MSEARCH_FOUND	2

/* UDP IP and port number for M-SEARCH */
// we have implemented the listener and
// packet send function into a single socket 
// so both ports should be the same 
#define M_SEARCH_CLIENT_SEND_PORT 1900
#define M_SEARCH_CLIENT_RCVE_PORT 1900

/* M-Search IP datagram text */
const prog_char IGD_MSRCH_LINE1[] PROGMEM = "M-SEARCH * HTTP/1.1\r\n"; // 21 chars
const prog_char IGD_MSRCH_LINE2[] PROGMEM = { "HOST: 239.255.255.250:1900\r\n" }; // 28 chars
const prog_char IGD_MSRCH_LINE3[] PROGMEM = { "MAN: \"ssdp:discover\"\r\n" }; // 22 chars
const prog_char IGD_MSRCH_LINE4[] PROGMEM = { "MX: 2\r\n" }; // 7 chars
const prog_char IGD_MSRCH_LINE5a[] PROGMEM = { "ST: urn:schemas-upnp-org:device:" }; //32 chars
const prog_char IGD_MSRCH_LINE5b[] PROGMEM = { "InternetGatewayDevice:1\r\n\r\n" }; // 25 chars

/* HTTP Request Packet text */
const prog_char IGD_RQST_LINE1A[] PROGMEM = { "GET " };
const prog_char IGD_RQST_LINE1B[]  PROGMEM = { " HTTP/1.1\r\n\r\n" };

const unsigned long IGD_LOCATE_TIMEOUT = 600000;
const unsigned long IGD_RESPONSE_TIMEOUT = 100000;

// NOTIFY and HTTP packet parsing strings
//const char NOTIFY_PACKET_STRING[7] = "NOTIFY";
const prog_char NOTIFY_PACKET_STRING[] PROGMEM = "NOTIFY";
const prog_char HTTP_PACKET_STRING[7] PROGMEM = "HTTP/1";
const prog_char LINE_FEED[2] PROGMEM = "\n";
const prog_char CARRIAGE_RETURN[2] PROGMEM = "\r";
const int ASCII_SPACE = 32;
const prog_char NT_TYPE_ID[4] PROGMEM = "NT:";
const prog_char ST_TYPE_ID[4] PROGMEM = "ST:";
const prog_char IGD_STRING_1[32] PROGMEM = "urn:schemas-upnp-org:device:Int";
const prog_char IGD_STRING_2[21] PROGMEM = "ernetGatewayDevice:1";
const prog_char LOC_TYPE_ID[4] PROGMEM = "Loc";
const prog_char LOC_TYPE_ID_2[7] PROGMEM = "ation:";
const prog_char CAPS_LOC_TYPE_ID[4] PROGMEM = "LOC";
const prog_char CAPS_LOC_TYPE_ID_2[7] PROGMEM = "ATION:";

// XML packet parsing strings
const prog_char SERVICE_TYPE[10] PROGMEM = "<serviceT";  //need to match on partial word, as full tag was causing issue on <service> line which was wrapping and skipping the next line
const prog_char SERVICE_STRING[32] PROGMEM = "ype>urn:schemas-upnp-org:servic";
const prog_char SERVICE_STRING_2[20] PROGMEM = "e:WANIPConnection:1";
const prog_char CONTROL_URL[13] PROGMEM = "<controlURL>";
const prog_char END_SERVICE[11] PROGMEM = "</service>";
const prog_char START_XML_TAG[2] PROGMEM = "<";

class MSearchClass {
private:
  uint32_t _msearchInitialTransactionId;
  uint32_t _msearchTransactionId;
  uint8_t _msearch_state;
  
  IPAddress  _msearchIgdIp;
  EthernetUDP _msearchUdpSocket;
  String _msearchIgdControlUrl;
  String _msearchLocationUrl;
  uint16_t _msearchIgdPort;
  EthernetClient _igdClient;

  void reset();
  int find_Igd();
  int send_MSEARCH_MESSAGE();
  int parseMSearchResponse();
  int parseIgdPort(String *locationURL);
  int parseControlUrl(String *locationURL, IPAddress igdIp, uint16_t &igdPort);
  int parseXmlResponse();

public:
  IPAddress igdIp();
  uint16_t igdPort();
  String controlUrl();
  int begin();

};

  #endif