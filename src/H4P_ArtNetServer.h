/*
 MIT License

Copyright (c) 2020 Phil Bowles <H48266@gmail.com>
   github     https://github.com/philbowles/H4
   blog       https://8266iot.blogspot.com
   groups     https://www.facebook.com/groups/esp8266questions/
			  https://www.facebook.com/H4-Esp8266-Firmware-Support-2338535503093896/


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#pragma once

#include<H4Service.h>
#include<H4P_WiFi.h>
#include<H4P_SerialCmd.h>

struct AN_pollReply_t { //poll reply packet structure
	const char ID[8] = "Art-Net";
	const uint8_t OpCodeLo = 0x00;
	const uint8_t OpCodeHi = 0x21;
	uint8_t IpAddress[4];
	const uint8_t PortNumberLo = 0x36;
	const uint8_t PortNumberHi = 0x19;
	uint8_t VersInfoHi = 0;
	uint8_t VersInfoLo = 0;
	uint8_t NetSwitch = 0;
	uint8_t SubSwitch = 0;
	uint8_t OemHi = 0;
	uint8_t OemLo = 0;
	uint8_t UebaVersion = 0;
	uint8_t Status1 = 0;
	uint8_t EstaManLo = 0;
	uint8_t EstaManHi = 0;
	char ShortName[18] = {0};
	char LongName[64] = {0};
	char NodeReport[64] = {0};
	uint8_t NumPortsHi = 0;
	uint8_t NumPortsLo = 0;
	uint8_t PortTypes[4] = {0};
	uint8_t GoodInput[4] = {0};
	uint8_t GoodOutput[4] = {0};
	uint8_t SwIn[4] = {0};
	uint8_t SwOut[4] = {0};
	const uint8_t SwVideo = 0;
	const uint8_t SwMacro = 0;
	const uint8_t SwRemote = 0;
	const uint8_t Spare1 = 0;
	const uint8_t Spare2 = 0;
	const uint8_t Spare3 = 0;
	uint8_t Style = 0;
	uint8_t Mac[6] = {0};
	uint8_t BindIp[4];
	uint8_t BindIndex = 0;
	uint8_t Status2 = 0;
	uint8_t Filler[26] = {0};
};

class H4P_ArtNetServer: public H4Service {
			H4P_WiFi*           _pWiFi;
			AsyncUDP 	        _udp;
			IPAddress		    _ubIP=IPAddress(239,255,255,250);
			uint16_t startChannel;
			uint16_t channelCount;

		virtual void            _handleEvent(const std::string& svc,H4PE_TYPE t,const std::string& msg) override;
				void            _handlePacket(std::string packet,IPAddress ip);
				void            _listenUDP();
				void            _artPoll(IPAddress pollIp, std::string pollPacket);
				void            _artDmx(const std::string* packet);
				//void            _notify(const std::string& s);

		//static  std::string     replaceParamsFile(const std::string &f){ return h4preplaceparams(CSTR(H4P_SerialCmd::read(f))); }
	public:

		AN_pollReply_t pollReplyPacket; //get a poll reply packet

		void _Construct(const char* shortName, const char* longName, uint16_t start, uint16_t count) { //the constructor
			_pWiFi=depend<H4P_WiFi>(wifiTag()); //set wifi as a dependency
			std::string chipId = h4p.gvGetstring(chipTag()); //get chipid
			memcpy(pollReplyPacket.Mac, chipId.c_str(), 6); //put chipid in mac slot of pollreplypacket
			memcpy(pollReplyPacket.ShortName, shortName, 18); //put shortname in proper place
			memcpy(pollReplyPacket.LongName, longName, 64); //put long name in proper place
			pollReplyPacket.Status1 = 0b00000000; //set status1
			pollReplyPacket.PortTypes[0] = 0b10000000; //set the port types
			pollReplyPacket.GoodOutput[0] = 0b10000000; //add the working output
			pollReplyPacket.Status2 = 0b00000010; //status2
			startChannel = start - 1;
			channelCount = count;
		}

		H4P_ArtNetServer(uint16_t dmxStartChannel, uint16_t numChannels): H4Service(artNetTag(),H4PE_GVCHANGE|H4PE_VIEWERS) {
			std::string chipId = h4p.gvGetstring(chipTag());
			std::string shortName =  "H4P " + chipId;
			//strcpy(pollReplyPacket.ShortName, shortName.c_str());
			std::string longName = "H4Plugins ArtNet receiver on ESP8266";
			//strcpy(pollReplyPacket.LongName, longName.c_str());
			//H4P_ArtNetServer(shortName.c_str(), longName.c_str());
			_Construct(shortName.c_str(), longName.c_str(), dmxStartChannel, numChannels);
		}

		H4P_ArtNetServer(const char* shortName, const char* longName, uint16_t dmxStartChannel=1, uint16_t numChannels=1): H4Service(artNetTag(),H4PE_GVCHANGE|H4PE_VIEWERS){
			_Construct(shortName, longName, dmxStartChannel, numChannels);
		}

				static std::string decodeDmx(std::string data);
				void           svcUp() override;
				void           svcDown() override;
				#if H4P_LOG_MESSAGES
                void           info() override;
				#endif
//          _syscall only
				void           _init() override;
};
