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

class H4P_ArtNetServer: public H4Service {
            //H4P_WiFi*           _pWiFi;
            WiFiClient          _wifi;
            AsyncUDP 	        _udp;
            IPAddress		    _ubIP=IPAddress(239,255,255,250);

        virtual void            _handleEvent(const std::string& svc,H4PE_TYPE t,const std::string& msg) override;
                void            _handlePacket(std::string packet,IPAddress ip);
                void            _listenUDP();
                void            _artPoll(IPAddress pollIp, std::string pollPacket);
                void            _artDmx(std::string packet);
                //void            _notify(const std::string& s);

        //static  std::string     replaceParamsFile(const std::string &f){ return h4preplaceparams(CSTR(H4P_SerialCmd::read(f))); }
    public:                
        H4P_ArtNetServer(const std::string& name=""): H4Service(artNetTag(),H4PE_GVCHANGE|H4PE_VIEWERS){
            //_pWiFi=depend<H4P_WiFi>(wifiTag());
            /*if(!h4p.gvExists(nameTag())) h4p.gvSetstring(nameTag(),(name=="") ? (uppercase(h4Tag())+" "+deviceTag()+" "+std::string(h4p[chipTag()])):name,true);
            XLOG("UPNP name %s",CSTR(h4p[nameTag()]));*/
        }
//               void            friendlyName(const std::string& name){ h4p[nameTag()]=name; }

                void           svcUp() override;
                void           svcDown() override;
//          _syscall only
                void           _init() override;
};
