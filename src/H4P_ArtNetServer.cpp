/*
 MIT License

Copyright (c) 2020 Phil Bowles <h4plugins@gmail.com>
   github     https://github.com/philbowles/esparto
   blog       https://8266iot.blogspot.com     
   groups     https://www.facebook.com/groups/esp8266questions/
			  https://www.facebook.com/Esparto-Esp8266-Firmware-Support-2338535503093896/
							  

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

void H4P_ArtNetServer::_handleEvent(const std::string& svc,H4PE_TYPE t,const std::string& msg){
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
/*        case H4PE_GVCHANGE:
			if(_running && svc==nameTag()){
//            if(svc==nameTag()){
				svcDown(); // shut down old name, send bye bye etc
				svcUp();
			}*/
	}
}

void H4P_ArtNetServer::_artPoll(IPAddress pollIp, std::string pollPacket) { //reply to artpoll
	Serial.println("pollreply");
	//IPAddress lIp;
	//lIp.fromString(h4p.gvGetstring(ipTag()).c_str());
	//for(int i = 0; i < 4; i++) pollReplyPacket.IpAddress[i] = lIp[i];
	//for(int i = 0; i < 4; i++) pollReplyPacket.BindIp[i] = lIp[i];

	_udp.writeTo((uint8_t *)&pollReplyPacket, 214, pollIp, artPort); //send the packet
}

void H4P_ArtNetServer::_artDmx(const std::string* packet) { //process artdmx
	//Serial.println("dmx");
	std::string dmx((*packet), startChannel + 18, channelCount); //transfer needed channels to a string
	memcpy(dmxData, dmx.data(), channelCount * sizeof(uint8_t));
	/*std::string dmx;
	for(int i = 0; i < channelCount; i++) dmx[i] = (*packet)[i + 18];*/
	//const std::string dmx(((*packet).data() + 18), 2); //transfer needed channels to a string
	//for(int i = 0; i < (channelCount + 1); i++) dmx[i] = (*packet)[i + (18 + channelOffset)];
	//analogWrite(0, 255 - dmx[0]);
	//Serial.print("strlen: ");
	//Serial.println(dmx.length());
	//Serial.println("channelcount");
	//Serial.println(channelCount);
	//int bytesSent = Serial.write((*(*packet).data()));
	//Serial.println((*packet).size());
	//for(int i = 0; i < channelCount; i++) Serial.print((*packet)[i + 18], HEX);

	h4psysevent(artNetTag(), H4PE_ARTDMX, "", 0); //send event with requested channels
}

uint8_t H4P_ArtNetServer::GetChannel(uint16_t channel) {
	return(dmxData[channel]);
}

void H4P_ArtNetServer::_listenUDP(){ //set up udp listening
	if(!_udp.listenMulticast(_ubIP, artPort)) return; // some kinda error?
	_udp.onPacket([this](AsyncUDPPacket packet){ //this is the callback for processing udp
		//Serial.println("pkt");
		std::string pkt((const char*)packet.data(),packet.length()); //make a std::string with packet contents
		IPAddress ip = packet.remoteIP(); //extract remote ip address

		if(pkt.compare(0, 7, "Art-Net") != 0) return; //check if packet is artnet
		if((pkt[8] == 0x00) && (pkt[9] == 0x50)) h4.queueFunction([=](){ _artDmx(&pkt); }, nullptr, H4P_TRID_ARTD); //ArtDmx
		if((pkt[8] == 0x00) && (pkt[9] == 0x20)) h4.onceRandom(0, 1000, [=](){ _artPoll(ip, pkt); }, nullptr, H4P_TRID_ARTD); //ArtPoll
	}); 
}

void H4P_ArtNetServer::_init(){ //initialization
	//pinMode(0, OUTPUT); //set onboard led pin to output
	//analogWriteRange(255); //set the analog write range to 8-bit
}

#if H4P_LOG_MESSAGES
void H4P_ArtNetServer::info(){ //info stuffs
	H4Service::info(); //tell h4 I'm sending info. I think.
    reply(" ShortName: %s\n LongName: %s", pollReplyPacket.ShortName, pollReplyPacket.LongName); //actually send the info
}
#endif

void H4P_ArtNetServer::svcDown(){ //sudo shutdown -h now
	Serial.println("service down");
	//h4.cancelSingleton(H4P_TRID_NTFY);
	_udp.close(); //close the udp port
	H4Service::svcDown(); //tell h4 I've finished shutting down. I think
}

void H4P_ArtNetServer::svcUp(){ //bootup
	Serial.println("service up");
#if H4P_USE_WIFI_AP
	if(WiFi.getMode()==WIFI_AP) return;
#endif
	_listenUDP(); //start listening for udp
	H4Service::svcUp(); //tell h4 I'm online
}
