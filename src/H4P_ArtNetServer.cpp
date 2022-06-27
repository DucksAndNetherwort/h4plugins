/*
 MIT License

Copyright (c) 2022 Ducks And Netherwort <ducks.and.netherwort@gmail.com>
   github     https://github.com/DucksAndNetherwort

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

#include<H4P_ArtNetServer.h>


const uint16_t artPort = 6454; //artnet port

void H4P_ArtNetServer::_handleEvent(const std::string& svc,H4PE_TYPE t,const std::string& msg){ //event handler
	switch(t){
		case H4PE_VIEWERS:
			{
				uint32_t mode=STOI(msg);
				if(mode) {
				#if H4P_USE_WIFI_AP
					if(mode==WIFI_AP) h4puiAdd(nameTag(),H4P_UI_INPUT,"s");
					else h4puiAdd(nameTag(),H4P_UI_TEXT,"s");
				#else
					h4puiAdd(nameTag(),H4P_UI_TEXT,"s");
				#endif
				}
			}
			break;
	}
}

void H4P_ArtNetServer::_artPoll(IPAddress pollIp, std::string pollPacket) { //reply to artpoll
	_udp.writeTo((uint8_t *)&pollReplyPacket, 214, pollIp, artPort); //send the packet
}

void H4P_ArtNetServer::_artDmx(const std::string* packet) { //process artdmx
	//Serial.println("dmx");
	std::string dmx((*packet), startChannel + 18, channelCount); //transfer needed channels to a string
	memcpy(dmxData, dmx.data(), channelCount * sizeof(uint8_t));
	h4psysevent(artNetTag(), H4PE_ARTDMX, "", 0); //send event
}

uint8_t H4P_ArtNetServer::GetChannel(uint16_t channel) { //return selected channel's contents
	return(dmxData[channel]);
}

void H4P_ArtNetServer::_listenUDP(){ //set up udp listening
	if(!_udp.listenMulticast(_ubIP, artPort)) return;
	_udp.onPacket([this](AsyncUDPPacket packet){ //this is the callback for processing udp
		//Serial.println("pkt"); //can be uncommented to check if udp is coming through. Will probably lag everything out
		std::string pkt((const char*)packet.data(),packet.length()); //make a std::string with packet contents
		IPAddress ip = packet.remoteIP(); //extract remote ip address

		if(pkt.compare(0, 7, "Art-Net") != 0) return; //check if packet is artnet
		if((pkt[8] == 0x00) && (pkt[9] == 0x50)) h4.queueFunction([=](){ _artDmx(&pkt); }, nullptr, H4P_TRID_ARTD); //ArtDmx
		if((pkt[8] == 0x00) && (pkt[9] == 0x20)) h4.onceRandom(0, 1000, [=](){ _artPoll(ip, pkt); }, nullptr, H4P_TRID_ARTD); //ArtPoll
	}); 
}

void H4P_ArtNetServer::_init(){ //initialization
}

#if H4P_LOG_MESSAGES
void H4P_ArtNetServer::info(){ //info stuffs
	H4Service::info(); //tell h4 I'm sending info. I think.

    reply(" ShortName: %s", pollReplyPacket.ShortName); //actually send the info
	reply(" LongName: %s", pollReplyPacket.LongName);
	char * buffer = (char*)malloc(int((log10((startChannel > channelCount) ? startChannel : channelCount) < 3) ? 3 : log10((startChannel > channelCount) ? startChannel : channelCount) + 1)); //allocate the bare minimum of memory to fit either one
	if (!buffer) {
		Serial.println("malloc failed for integer string conversion buffer. Firing H4PE_SYSFATAL");
		h4psysevent(artNetTag(), H4PE_SYSFATAL, "malloc failed", 0);
	}
	itoa(startChannel, buffer, 10); //make it a string
	reply(" Start Channel: %s", buffer); //send it
	itoa(channelCount, buffer, 10); //make it a string
	reply(" Channel Count: %s", buffer); //send it
	for(int i = 0; i < channelCount; i++) { //send channel data
		itoa(i + 1, buffer, 10);
		reply(" channel %s:", buffer);
		itoa(dmxData[i], buffer, 10);
		reply(" %s", buffer);
	}
	free(buffer); //free all 4 bytes
}
#endif

void H4P_ArtNetServer::svcDown(){ //sudo service artNet stop
	_udp.close(); //close the udp port
	H4Service::svcDown(); //tell h4 I've finished shutting down. I think.
}

void H4P_ArtNetServer::svcUp(){ //bootup
#if H4P_USE_WIFI_AP
	if(WiFi.getMode()==WIFI_AP) return;
#endif
	_listenUDP(); //start listening for udp
	H4Service::svcUp(); //tell h4 I'm online
}
