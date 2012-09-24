// UPNP Internet Gateway Device	locator - Library v1.0 - August 8th 2012
// Author: Deverick McIntyre


#include "locateIgd.h"
#include "Arduino.h"

int MSearchClass::begin()
{
    reset();
	return find_Igd();
}

void MSearchClass::reset()
{
    // zero out or null all variables
    _msearch_state = 0;
    _msearchIgdIp = (0, 0, 0, 0);
    _msearchIgdControlUrl = "";
    _msearchIgdPort = 0;

}

IPAddress MSearchClass::igdIp()
{
    return _msearchIgdIp;
}

uint16_t MSearchClass::igdPort()
{
    return _msearchIgdPort;
}

String MSearchClass::controlUrl()
{
    return _msearchIgdControlUrl;
}

int MSearchClass::find_Igd()
{
    // Pick an initial transaction ID
    _msearchTransactionId = random(1UL, 2000UL);
    _msearchInitialTransactionId = _msearchTransactionId;

    if (_msearchUdpSocket.begin(M_SEARCH_CLIENT_RCVE_PORT) == 0)
    {
      // Couldn't get a socket
      return 0;
    }
    int result = 0;

    unsigned long startTime = millis();
    
    while(_msearch_state != STATE_MSEARCH_FOUND)
    {
    	if (_msearch_state == STATE_MSEARCH_START)
    	{
    		if (send_MSEARCH_MESSAGE() == 1)
            {
                _msearchTransactionId++;
                _msearch_state = STATE_MSEARCH_WAIT_RESPONSE;
            }
    	}

        if (_msearch_state == STATE_MSEARCH_WAIT_RESPONSE)
        {
            _msearchTransactionId++;
            if (parseMSearchResponse() == 1)
            {
                _msearch_state == STATE_MSEARCH_FOUND;
                result = 1;
            }
                break;
        } 

		if(result != 1 && ((millis() - startTime) > IGD_LOCATE_TIMEOUT))
            break;
    }
    
    // We're done with the socket now
    _msearchUdpSocket.stop();
    _msearchTransactionId++;

    return result;
}

int MSearchClass::send_MSEARCH_MESSAGE()
{
    static const IPAddress ms_bcast_addr( 239, 255, 255, 250 ); // M-SEARCH Broadcast address
    
    if (-1 == _msearchUdpSocket.beginPacket(ms_bcast_addr, M_SEARCH_CLIENT_SEND_PORT))
    {
        return 0;
    }
    char * buffer = (char *) malloc (32);
    memset(buffer, 0, 32);

    strcpy_P(buffer, IGD_MSRCH_LINE1);
    _msearchUdpSocket.print(buffer);
    memset(buffer, 0, 32); // clear local buffer

    strcpy_P(buffer, IGD_MSRCH_LINE2);
    _msearchUdpSocket.print(buffer);
    memset(buffer, 0, 32); // clear local buffer

    strcpy_P(buffer, IGD_MSRCH_LINE3);
    _msearchUdpSocket.print(buffer);
    memset(buffer, 0, 32); // clear local buffer

    strcpy_P(buffer, IGD_MSRCH_LINE4);
    _msearchUdpSocket.print(buffer);
    memset(buffer, 0, 32); // clear local buffer

    strcpy_P(buffer, IGD_MSRCH_LINE5a);
    _msearchUdpSocket.print(buffer);
    memset(buffer, 0, 32); // clear local buffer

    strcpy_P(buffer, IGD_MSRCH_LINE5b);
    _msearchUdpSocket.print(buffer);


    free(buffer);

    int sendResult = _msearchUdpSocket.endPacket();
    return sendResult;

}
// *** Due to differing implementations of discovery for so called upnp enabled devices, we look for two different packet types to verify
// *** a device is an IGD.  So, strictly speaking we should only accept the NOTIFY datagram, but in this implementation we also accept the HTTP
// *** datagram with the correct payload.
// Below is an example structure for the UDP NOTIFY datagram from a root device which has the 'InternetGatewayDevice' notification type
// Following is the logic for parsing a datagram to confirm it is an IGD NOTIFY datagram
    // first we check first six characters is 'NOTIFY'
    // then we read through characters to each line feed character
    // after each line feed character we look for a 'NT:' or 'Loc' string
    // if 'NT:' string found then remove any trailing white space characters
        // remaining string on that line should be 'urn:schemas-upnp-org:device:InternetGatewayDevice:1'
    // if 'Loc' string found then check if 'ation:' string follows
        // then remove any trailing white spaces after :
        // then move the rest of the line into our string variable
//****************
// NOTIFY * HTTP/1.1
// Host:239.255.255.250:1900
// Cache-Control:max-age=180
// Location:http://192.168.0.1:52869/picsdesc.xml
// Server:OS 1.0 UPnP/1.0 Realtek/V1.3
// NT:urn:schemas-upnp-org:device:InternetGatewayDevice:1
// USN:uuid:12342409-1234-1234-5678-ee1234cc5678::urn:schemas-upnp-org:device:InternetGatewayDevice:1
// NTS:ssdp:alive
//*********************
//
// Below is an example structure for the HTTP response datagram from a root device which has the 'InternetGatewayDevice' notification type
// Following is the logic for parsing a datagram to confirm it is an IGD HTTP response datagram
    // first we check first six characters is 'HTTP/1'
    // then we read through characters to each line feed character
    // after each line feed character we look for a 'ST:' string
    // remove any space characters after the 'ST:' string
    // remaining string on that line should be 'urn:schemas-upnp-org:device:InternetGatewayDevice:1'
//*********************
// HTTP/1.1 200 OK
// Cache-Control: max-age=180
// ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1
// USN: uuid:12342409-1234-1234-5678-ee1234cc5678::urn:schemas-upnp-org:device:InternetGatewayDevice:1
// EXT:
// Server: OS 1.0 UPnP/1.0 Realtek/V1.3
// Location: http://192.168.0.1:52869/picsdesc.xml
//*********************

int MSearchClass::parseMSearchResponse()
{
    // set variable intial states, and create local variables
    char * buffer = (char *) malloc (32);
    memset(buffer, 0, 32);
    char c;
    unsigned long responseStartTime = millis();
    
    int type_flag = 0;
    int result = 0;

    _msearchIgdControlUrl = "";
    _msearchIgdPort = 0;

    while ((millis() - responseStartTime) < IGD_RESPONSE_TIMEOUT)
    {
        // loop will return to here after each packet to wait for next packet.
        while(_msearchUdpSocket.parsePacket() <= 0)
        {
            if((millis() - responseStartTime) > IGD_RESPONSE_TIMEOUT)
            {
                //Serial.println(F("ran out of time!"));
                _msearchUdpSocket.stop();
                free(buffer);
                return 0;
            }
            delay(40);
        }

        while (_msearchUdpSocket.available())
        {
            // first six characters of UPD frame should be 'NOTIFY' or 'HTTP/1'
            memset(buffer, 0, 32);
            _msearchUdpSocket.read(buffer, 6);
            /*
            NOTIFY and HTTP/1 DATAGRAM PROCESSING
            */
            if ((strncmp_P(buffer, NOTIFY_PACKET_STRING, 6) == 0) || (strncmp_P(buffer, HTTP_PACKET_STRING, 6) == 0))
            {
                while (_msearchUdpSocket.available())
                {
                    c = _msearchUdpSocket.read();
                    while (strncmp_P(&c, LINE_FEED, 1) != 0) // iterate through the characters until we hit a LF then drop out
                    {
                        c = _msearchUdpSocket.read();
                    }
                    memset(buffer, 0, 32);
                    _msearchUdpSocket.read(buffer, 3);
                    if ((strncmp_P(buffer, NT_TYPE_ID, 3) == 0) || (strncmp_P(buffer, ST_TYPE_ID, 3) == 0))// if new line starts with 'NT:' then continue checking
                    {
                        // first strip out any spaces
                        while (_msearchUdpSocket.peek() == ASCII_SPACE)
                        {
                            _msearchUdpSocket.read(); //discard white space character
                        }
                        memset(buffer, 0, 32);
                        _msearchUdpSocket.read(buffer, 31);
                        if (strncmp_P(buffer, IGD_STRING_1, 31) == 0) // check the first part of string is correct
                        {
                            memset(buffer, 0, 32);
                            _msearchUdpSocket.read(buffer, 20);
                            if (strncmp_P(buffer, IGD_STRING_2, 20) == 0) // check the second part of string is correct
                            {
                                type_flag = 1;
                            }
                        }
                    // if new line starts with 'Loc' or 'LOC' then continue checking
                    } else if ((strncmp_P(buffer, LOC_TYPE_ID, 3) == 0) || (strncmp_P(buffer, CAPS_LOC_TYPE_ID, 3) == 0)) 
                    {
                        memset(buffer, 0, 32);
                        _msearchUdpSocket.read(buffer, 6);
                        // if rest of word is 'ation:' or 'ATION:' then continue checking
                        if ((strncmp_P(buffer, LOC_TYPE_ID_2, 6) == 0) || (strncmp_P(buffer, CAPS_LOC_TYPE_ID_2, 6) == 0))
                        {
                            // first strip out any spaces
                            while (_msearchUdpSocket.peek() == ASCII_SPACE)
                            {
                                _msearchUdpSocket.read(); //discard white space character
                            }
                            c = _msearchUdpSocket.read(); //read the next character
                            while (strncmp_P(&c, CARRIAGE_RETURN, 1) != 0)  // if we find a carriage return it means the URL and line have ended
                            {
                                _msearchLocationUrl += c;
                                c = _msearchUdpSocket.read(); //read the next character and append to the string
                            }
                        }
                    }

                    // if we have the location URL AND the packet is of the type we need then we can stop processing packets. 
                    if (_msearchLocationUrl.length() && type_flag)
                    {
                        // if we are able to parse out the port number then try to parse out the controlURL
                        if (parseIgdPort(&_msearchLocationUrl)) // if we are able to find 
                        { 
                            _msearchIgdIp = _msearchUdpSocket.remoteIP();
                            // finally if we are able to parse out the control URL then return success
                            if (parseControlUrl(&_msearchLocationUrl, _msearchIgdIp, _msearchIgdPort)) {
                                _msearchUdpSocket.stop();
                                free(buffer);
                                return 1;
                            } else {
                                // unable to parse out the control URL.  return error condition
                                _msearchUdpSocket.stop();
                                free(buffer);
                                return 0;
                            }
                        } else {
                            // unable to parse out the port number.  return error condition
                            _msearchUdpSocket.stop();
                            free(buffer);
                            return 0;
                        }

                    }

                }
  
            } else {
                // if we are here it is because this packet is not what we are looking for
                // need to discard packet

                // skip to the end of the packet
                _msearchUdpSocket.flush();

                //Serial.print(F("packet found but not notify or HTTP type"));
                //while (_msearchUdpSocket.available())
                //{
                //    char c = _msearchUdpSocket.read();
                //    Serial.print(c);
                //}
            }
            // if we are here it is because a packet failed validation.  need to reset variables and wait for next packet.
            _msearchIgdControlUrl   = "";
            _msearchLocationUrl     = "";
            _msearchIgdPort         = 0;
        }
    
    }
    _msearchUdpSocket.stop();
    free(buffer);
    return 0;
}

int MSearchClass::parseIgdPort(String *locationURL)
{
    String tempString = *locationURL;
    int lastColon = tempString.lastIndexOf(':');
    int firstSlash = tempString.indexOf('/', lastColon);
    tempString = tempString.substring(lastColon+1, firstSlash);
    char charArray[tempString.length()+1];
    tempString.toCharArray(charArray, sizeof(charArray));
    _msearchIgdPort = atof(charArray);
    if (_msearchIgdPort) {
        return 1;
    } else {
        return 0;
    }
}

int MSearchClass::parseControlUrl(String *locationURL, IPAddress igdIp, uint16_t &igdPort)
{
    String locationUrlString = *locationURL;
    int lastColon = locationUrlString.lastIndexOf(':');
    int firstSlash = locationUrlString.indexOf('/', lastColon);
    locationUrlString = locationUrlString.substring(firstSlash);

    char locationUrlArray[locationUrlString.length()+1];
    locationUrlString.toCharArray(locationUrlArray, locationUrlString.length()+1);
    
    if (_igdClient.connect(igdIp, igdPort)) {
        
        char * buffer = (char *) malloc (32);
        memset(buffer, 0, 32);
        // Make a HTTP request:
        strcpy_P(buffer, IGD_RQST_LINE1A);
        _igdClient.print(buffer);
        memset(buffer, 0, 32);

        _igdClient.write((uint8_t *)locationUrlArray, locationUrlString.length());

        strcpy_P(buffer, IGD_RQST_LINE1B);
        _igdClient.print(buffer);

        free(buffer);        
        return parseXmlResponse();
    } 
    else {
        // if you didn't get a connection to the server:
        _igdClient.stop();
        return 0;
    }
}

int MSearchClass::parseXmlResponse()
{
    // set variable intial states, and create local variables
    char * buffer = (char *) malloc (32);
    memset(buffer, 0, 32);
    char c;
    unsigned long responseStartTime = millis();

    while (_igdClient.connected()) 
    {
        if (_igdClient.available()) 
        {
            // first strip out any spaces
            while (_igdClient.peek() == ASCII_SPACE)
            {
                _igdClient.read(); //discard white space character
            }
            memset(buffer, 0, 32);
            _igdClient.readBytes(buffer, 9);
            if (strncmp_P(buffer, SERVICE_TYPE, 9) == 0) // if new line starts with '<serviceTy' then continue checking
            {
                memset(buffer, 0, 32);
                _igdClient.readBytes(buffer, 31);
                if (strncmp_P(buffer, SERVICE_STRING, 31) == 0) // if first part of line matches our string then continue checking
                {
                    memset(buffer, 0, 32);
                    _igdClient.readBytes(buffer, 19);
                    if (strncmp_P(buffer, SERVICE_STRING_2, 19) == 0) // if second part of line matches our string then continue checking
                    {
                        // now we have found the service type we need setup a loop to iterate thru each line till we find control URL
                        // we only loop till we hit the  the service tag end wrapper. </service> however we should not get this far before
                        // finding the control URL tag. not finding control URL tag and getting to </service> is an error condition
                        while (strncmp_P(buffer, END_SERVICE, 10) != 0)
                        {
                            c = _igdClient.read();
                            while (strncmp_P(&c, LINE_FEED, 1) != 0) // iterate through the characters until we hit a LF then drop out
                            {
                                c = _igdClient.read();
                            }
                            // strip out any spaces at start of new line
                            while (_igdClient.peek() == ASCII_SPACE)
                            {
                                _igdClient.read(); //discard white space character
                            }
                            memset(buffer, 0, 32);
                            _igdClient.readBytes(buffer, 12);
                            if (strncmp_P(buffer, CONTROL_URL, 12) == 0) // if new line starts with '<controlURL>' then continue checking
                            {
                                c = _igdClient.read(); //read the next character
                                while (strncmp_P(&c, START_XML_TAG, 1) != 0)  // if we find an XML end tag it means we are at the end of the line
                                {
                                    _msearchIgdControlUrl += c;
                                    c = _igdClient.read(); //read the next character and append to the string
                                }
                                _igdClient.stop();
                                free(buffer);
                                return 1;
                            }
                        }
                        // We have reached the end service wrapper tag - error condition.
                        // If we reach here this is an error condition because we should have found
                        // the control URL tags for this service before getting this far.
                        _igdClient.stop();
                        free(buffer);
                        return 0;
                    }
                }
            }
            // flush out the full line of characters
            c = _igdClient.read();
            while (strncmp(&c, LINE_FEED, 1) != 0) // iterate through the characters until we hit a LF then drop through
            {
                //Serial.print(c);
                c = _igdClient.read();
            }
        } //end if (client.available())
        if((millis() - responseStartTime) > IGD_RESPONSE_TIMEOUT)
        {
            //Serial.println(F("ran out of time!"));
            _igdClient.stop();
            free(buffer);
            return 0;
        }
    } //end while (client.connected())
    // if we are here it is because we couldnt find the URL in the XML. return an error condition
    _igdClient.stop();
    free(buffer);
    return 0;
} //end function parseXmlResponse