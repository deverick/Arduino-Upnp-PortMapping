// UPNP IGD Port Mapping    Library v1.0 - August 8th 2012
// Author: Deverick McIntyre

	#include "portMapping.h"


	void PortMapClass::reset() 
	{
		_portmap_state = 0;
        _portmapInternalPort = 0;
        _portmapExternalPort = 0;
	}

	int PortMapClass::addPortMap(IPAddress myIP)
	{
        unsigned long responseStartTime = millis(); 
        uint16_t randomPort;

        while ((millis() - responseStartTime) < PM_RESPONSE_TIMEOUT)  //keep trying with different random port numbers until success or time runs out
        {
            randomSeed(millis());
            randomPort = random(RAND_PORT_RANGE_MIN, RAND_PORT_RANGE_MAX);
            if (addPortMap(myIP, randomPort, randomPort))  // if add port mapping was successful then exit
            {
                return 1;
            }
            delay(10000);
        }
        return 0;  //timed out before successful port mapping so fail
	}

	int PortMapClass::addPortMap(IPAddress myIP, uint16_t requestedIntPort, uint16_t requestedExtPort)
	{	
		reset();

        return addPortMapRequest(myIP, requestedIntPort, requestedExtPort);
	}

	int PortMapClass::addPortMapRequest(IPAddress myIP, uint16_t requestedIntPort, uint16_t requestedExtPort)
	{

		if (_apmClient.connect(igdIp(), igdPort()))
		{
			char * buffer = (char *) malloc (448);
            memset(buffer, 0, 448);
            char * smallbuffer = (char *) malloc (8);
    		memset(smallbuffer, 0,8);

    		strcpy_P(buffer, PM_HEADER1A);

            // convert control URL from string to char array and append to buffer contents
            String controlUrlString = igdControlUrl();
            char controlUrlStringArray[controlUrlString.length()+1];
            controlUrlString.toCharArray(controlUrlStringArray, sizeof(controlUrlStringArray));
            strcat(buffer, controlUrlStringArray);

            strcat_P(buffer, PM_HEADER1B);

            // convert igd IP address to char array and append to buffer contents
            IPAddress igdIpAddress = igdIp();
            strcat(buffer, itoa(igdIpAddress[0], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ".");
            strcat(buffer, itoa(igdIpAddress[1], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ".");
            strcat(buffer, itoa(igdIpAddress[2], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ".");
            strcat(buffer, itoa(igdIpAddress[3], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ":");
            
            // convert igd port number to char array and append
            uint16_t igdPortNumber = igdPort();
            strcat(buffer, itoa(igdPortNumber, smallbuffer, 10));
            memset(smallbuffer, 0,8);

            strcat_P(buffer, PM_HEADER1C);

			strcat_P(buffer, PM_BODY1);

			strcat_P(buffer, PM_BODY2);

			strcat_P(buffer, PM_BODY3);

			strcat_P(buffer, PM_BODY4);

			strcat_P(buffer, PM_BODY5);
			
			strcat_P(buffer, PM_BODY6A);

            strcat(buffer, itoa(requestedExtPort, smallbuffer, 10));
            memset(smallbuffer, 0,8);
			
			strcat_P(buffer, PM_BODY6B);

            strcat_P(buffer, PM_BODY7);

            strcat_P(buffer, PM_BODY8A);

            strcat(buffer, itoa(requestedIntPort, smallbuffer, 10));
            memset(smallbuffer, 0,8);
           
            strcat_P(buffer, PM_BODY8B);

            strcat_P(buffer, PM_BODY9A);

            strcat(buffer, itoa(myIP[0], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ".");
            strcat(buffer, itoa(myIP[1], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ".");
            strcat(buffer, itoa(myIP[2], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ".");
            strcat(buffer, itoa(myIP[3], smallbuffer, 10));
            memset(smallbuffer, 0,8);

            strcat_P(buffer, PM_BODY9B);
            _apmClient.print(buffer);
            memset(buffer, 0,448);

            strcpy_P(buffer, PM_BODY10);
            
            strcat_P(buffer, PM_BODY11);

            strcat_P(buffer, PM_BODY12);

            strcat_P(buffer, PM_BODY13);

            strcat_P(buffer, PM_BODY14);

            strcat_P(buffer, PM_BODY15);
            _apmClient.print(buffer);

			free(buffer);
            free(smallbuffer);

			return parsePortMapResponse(requestedIntPort, requestedExtPort);

		} else {
        	// if you didn't get a connection to the server:
        	_apmClient.stop();
        	return 0;
    	}
	}

	int PortMapClass::parsePortMapResponse(uint16_t requestedIntPort, uint16_t requestedExtPort)
	{
        // set variable intial states, and create local variables
        char * buffer = (char *) malloc (32);
        memset(buffer, 0, 32);
        char c;
        unsigned long responseStartTime = millis();

        while (_apmClient.connected()) 
        {
            if (_apmClient.available()) 
            {
                memset(buffer, 0, 32);
                _apmClient.readBytes(buffer, 12);
                if (strncmp_P(buffer, HTTP_RESPONSE, 12) == 0) // if packet is 200 OK response then store port numbers
                {
                    _portmapInternalPort = requestedIntPort;
                    _portmapExternalPort = requestedExtPort;
                    _apmClient.flush();
                    _apmClient.stop();
                    free(buffer);
                    return 1;
                } else { // did not receive a 200 OK HTTP response so cleanup and return a fail
                    while (_apmClient.available()) 
                    {
                        c = _apmClient.read(); //read the next character
                        Serial.print(c);
                    }
                    _apmClient.flush();
                    _apmClient.stop();
                    free(buffer);
                    return 0;
                }
            }
            if((millis() - responseStartTime) > PM_RESPONSE_TIMEOUT)
            {
                //Serial.println(F("ran out of time!"));
                _apmClient.flush();
                _apmClient.stop();
                free(buffer);
                return 0;
            }
        }
        _apmClient.flush();
        _apmClient.stop();
        free(buffer);
        return 0;
	}

	uint16_t PortMapClass::internalPort()
	{
		// returns the internal port if port map successful, else zero
        return _portmapInternalPort;
	}

	uint16_t PortMapClass::externalPort()
	{
		// returns the external port if port map successful, else zero
        return _portmapExternalPort;
	}

	IPAddress PortMapClass::externalIp()
	{
		if (_externalIp[1] != 0)
		{
			// if we already have retrieved the external Ip then return it
			return _externalIp;

		} else {
			
			getExternalIpRequest();
		}
		// returns the external Ip if check successful, else zero Ip
		return _externalIp;
	}

	int PortMapClass::getExternalIpRequest()
	{

		if (_apmClient.connect(igdIp(), igdPort()))
		{
        	char * buffer = (char *) malloc (256);
    		memset(buffer, 0, 256);
            char * smallbuffer = (char *) malloc (8);
            memset(smallbuffer, 0,8);

        	// Make a HTTP request:

        	strcpy_P(buffer, EXTIP_HEADER1A);

            // convert control URL from string to char array and append to buffer contents
            String controlUrlString = igdControlUrl();
            char controlUrlStringArray[controlUrlString.length()+1];
            controlUrlString.toCharArray(controlUrlStringArray, sizeof(controlUrlStringArray));
            strcat(buffer, controlUrlStringArray);

            strcat_P(buffer, EXTIP_HEADER1B);

            // convert igd IP address to char array and append to buffer contents
            IPAddress igdIpAddress = igdIp();
            strcat(buffer, itoa(igdIpAddress[0], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ".");
            strcat(buffer, itoa(igdIpAddress[1], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ".");
            strcat(buffer, itoa(igdIpAddress[2], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ".");
            strcat(buffer, itoa(igdIpAddress[3], smallbuffer, 10));
            memset(smallbuffer, 0,8);
            strcat(buffer, ":");
            
            // convert igd port number to char array and append
            uint16_t igdPortNumber = igdPort();
            strcat(buffer, itoa(igdPortNumber, smallbuffer, 10));
            memset(smallbuffer, 0,8);

            strcat_P(buffer, EXTIP_HEADER1C);

			strcat_P(buffer, EXTIP_BODY1);

        	strcat_P(buffer, EXTIP_BODY2);

        	strcat_P(buffer, EXTIP_BODY3);

        	strcat_P(buffer, EXTIP_BODY4);

        	strcat_P(buffer, EXTIP_BODY5);

        	strcat_P(buffer, EXTIP_BODY6);
        	_apmClient.print(buffer);

        	free(buffer);
            free(smallbuffer);

        	return parseExternalIpResponse();
    	} else {
        	// if you didn't get a connection to the server:
        	_apmClient.stop();
        	return 0;
    	}
	}

	int PortMapClass::parseExternalIpResponse()
	{
		// set variable intial states, and create local variables
   		char * buffer = (char *) malloc (32);
    	memset(buffer, 0, 32);
		char c;
		unsigned long responseStartTime = millis();

		while (_apmClient.connected()) 
    	{
        	if (_apmClient.available()) 
        	{
        		memset(buffer, 0, 32);
            	_apmClient.readBytes(buffer, 12);
            	if (strncmp_P(buffer, HTTP_RESPONSE, 12) == 0) // if packet is 200 OK response then continue
            	{
            		// setup inner loop to iterate thru packet contents to find external IP tag
            		while (_apmClient.connected()) 
        			{
        				c = _apmClient.read();
        				if (strncmp_P(&c, XML_TAG_OPEN, 1) == 0) // if character is tag start then continue
        				{
        					memset(buffer, 0, 32);
            				_apmClient.readBytes(buffer, 21);
            				if (strncmp_P(buffer, XML_TAG_EXTIP, 21) == 0) // if string is external IP tag start then continue
            				{
            					//copy the IP address into a string variable for parsing
            					String ipString;
            					c = _apmClient.read();
            					while (strncmp_P(&c, XML_TAG_OPEN, 1) != 0)
        						{
        							ipString += c;
        							c = _apmClient.read();
        						}
        						// next parse the string into separate IP address octets and store in local variable
        						int firstDot = ipString.indexOf('.'); // get the locations of the dots in the IP string
        						int secondDot = ipString.indexOf('.', firstDot + 1);
        						int thirdDot = ipString.indexOf('.', secondDot + 1);
        						if (firstDot == -1 || secondDot == -1 || thirdDot == -1) // if we cant find 3 dots then IP address invalid so error
        						{
        							return 0;
        						}
        						String firstOctet = ipString.substring(0, firstDot);  // next substring each octet out from between dots
        						String secondOctet = ipString.substring(firstDot + 1, secondDot);
        						String thirdOctet = ipString.substring(secondDot + 1, thirdDot);
        						String fourthOctet = ipString.substring(thirdDot + 1);
        						char firstOctetArray[firstOctet.length()+1];  // next instantiate char arrays for conversion
        						char secondOctetArray[secondOctet.length()+1];
        						char thirdOctetArray[thirdOctet.length()+1];
        						char fourthOctetArray[fourthOctet.length()+1];
        						firstOctet.toCharArray(firstOctetArray, sizeof(firstOctetArray)); //convert strings to char arrays
        						secondOctet.toCharArray(secondOctetArray, sizeof(secondOctetArray));
        						thirdOctet.toCharArray(thirdOctetArray, sizeof(thirdOctetArray));
        						fourthOctet.toCharArray(fourthOctetArray, sizeof(fourthOctetArray));
        						_externalIp[0] = atoi(firstOctetArray); // convert char arrays to integers and assign to class variable
        						_externalIp[1] = atoi(secondOctetArray);
        						_externalIp[2] = atoi(thirdOctetArray);
        						_externalIp[3] = atoi(fourthOctetArray);
        						_apmClient.flush();
        						_apmClient.stop();
        						free(buffer);
        						return 1;
            				}
        				}
        			}
        			// if we dropped out to here we did not find the external IP so error
        			return 0;

            	} else { // did not receive a 200 OK HTTP response so cleanup and return a fail
            		_apmClient.flush();
            		_apmClient.stop();
            		free(buffer);
            		return 0;
            	}
        	}
        	if((millis() - responseStartTime) > PM_RESPONSE_TIMEOUT)
        	{
            	//Serial.println(F("ran out of time!"));
            	_apmClient.flush();
            	_apmClient.stop();
            	free(buffer);
            	return 0;
        	}
        }
        _apmClient.flush();
        _apmClient.stop();
        free(buffer);
        return 0;
	}

	IPAddress PortMapClass::igdIp()
	{
		if (_igdIp[0] != 0)
		{
			// if we already have igd IP then return it
			return _igdIp;

		} else {
			if (getIgdParams() == 1)
			{
				return _igdIp; 
			}
		}
	}

	uint16_t PortMapClass::igdPort()
	{

		if (_igdPort != 0)
		{
			// if we already have retrieved the igd Port then return it
			return _igdPort;

		} else {
			if (getIgdParams() == 1)
			{
				return _igdPort; 
			}
		}
	}

	String PortMapClass::igdControlUrl()
	{

		if (_igdControlUrl != "")
		{
			// if we already have retrieved the igd control URL then return it
			return _igdControlUrl;

		} else {
			if (getIgdParams() == 1)
			{
				return _igdControlUrl; 
			}
		}
	}

	int PortMapClass::getIgdParams()
	{
		if (msearch.begin() != 0)
 		{
    		_igdIp 			= msearch.igdIp();
    		_igdPort 		= msearch.igdPort();
    		_igdControlUrl	= msearch.controlUrl();
    		return 1;

 		} else {
 			return 0;
 		}

	}