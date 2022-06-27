#define H4P_VERBOSE 1
#include<H4Plugins.h>
H4_USE_PLUGINS(115200,H4_Q_CAPACITY,false) // Serial baud rate, Q size, SerialCmd autostop
std::string defaultSsid = "XXXXXXXXX"; //default ssid to be used on factory reset
std::string defaultPsk = "XXXXXXXXXX"; //default psk to be used on factory reset
H4P_WiFi wiffy(defaultSsid, defaultPsk, "Art-Net Client");

const int startChannel = 0; //factory start channel to use. starting from this channel the colour layout is RGB
const int red = 14; //pin for red led
const int green = 12; //green led pin
const int blue = 13; //blue led pin
const int resetPin = 0; //pin the reset button is connected to. 0 corresponds to 'flash' on nodemcu

h4pMultifunctionButton mfb(resetPin, INPUT_PULLUP, ACTIVE_LOW, 20);

H4P_ArtNetServer* ArtNet; //get a global pointer for the Art-Net

void h4setup() { //setup function
	analogWriteRange(255); //adjust range of analog write to 0-255, for making it work with Art-Net
	pinMode(red, OUTPUT); //initialize pins
	pinMode(green, OUTPUT);
	pinMode(blue, OUTPUT);
	if(!h4p.gvExists("wifi ssid") || !h4p.gvExists("wifi ssid") || !h4p.gvExists("Start Channel")) { //initialize h4p globals
		h4p.gvSetstring("wifi ssid", defaultSsid.c_str(), true);
		h4p.gvSetstring("wifi psk", defaultPsk.c_str(), true);
		char * buffer = (char*)malloc(int(log10(startChannel) + 1)); //allocate the bare minimum of memory to fit either one
		itoa(startChannel, buffer, 10);
		h4p.gvSetstring("Start Channel", buffer, true);
		free(buffer);
	}
	else if((h4p.gvGetstring("wifi ssid") == "") || (h4p.gvGetstring("wifi ssid") == "") || (h4p.gvGetstring("Start Channel") == "")) {
		h4p.gvSetstring("wifi ssid", defaultSsid.c_str(), true);
		h4p.gvSetstring("wifi psk", defaultPsk.c_str(), true);
		char * buffer = (char*)malloc(int(log10(startChannel) + 1)); //allocate the bare minimum of memory to fit either one
		itoa(startChannel, buffer, 10);
		h4p.gvSetstring("Start Channel", buffer, true);
		free(buffer);
	}

	wiffy.change(h4p.gvGetstring("wifi ssid"), h4p.gvGetstring("wifi psk")); //update wifi config
	ArtNet = new H4P_ArtNetServer(STOI(h4p.gvGetstring("Start Channel")) + 1, 3); //start Art-Net
}

void artNetHandler(std::string svc, H4PE_TYPE t, std::string msg){ //handle Art-Net data
	analogWrite(red, (*ArtNet).GetChannel(0)); //GetChannel retrieves the data for the supplied channel
	analogWrite(green, (*ArtNet).GetChannel(1));
	analogWrite(blue, (*ArtNet).GetChannel(2));
	//for(int i = 0; i<3; i++) Serial.println((*ArtNet).GetChannel(i));
};

void resetHandler(std::string svc, H4PE_TYPE t, std::string msg){ //handler for factory reset
	Serial.println("factory reset detected");
	h4p.gvSetstring("wifi ssid", defaultSsid.c_str(), true); //reset ssid
	h4p.gvSetstring("wifi psk", defaultPsk.c_str(), true); //reset psk
	h4p.gvSetstring("Start Channel", "0", true); //reset start channel gv
}

void onViewersConnect() { //called when a user connects to webui
	Serial.println("User connected");
	wiffy.uiAddInput("wifi ssid"); //add all the needed inputs to the webui
	wiffy.uiAddInput("wifi psk");
	wiffy.uiAddInput("Start Channel");
	wiffy.uiAddImgButton("reboot");
	h4.once(175,[]{ h4puiSync("reboot","0"); }); //make the reboot button visible
}

void onViewersDisconnect() { //called when user disconnects from webui
	Serial.println("User left");
}

void onrebootButton() { //called when the webui reboot button is pressed
	Serial.println("onrebootButton called");
	h4p.invokeCmd("h4/reboot");
}

H4P_EventListener artDmxListener(H4PE_ARTDMX, artNetHandler); //register an ARTDMX event listener and point it to the Art-Net handler

H4P_EventListener resetListener(H4PE_FACTORY, resetHandler); //register a factory reset event listener and point it to the factory reset code

H4P_EventListener allexceptmsg(H4PE_VIEWERS | H4PE_GVCHANGE, [](const std::string& svc, H4PE_TYPE t, const std::string& msg){ //main event listener
	switch (t) {
	case H4PE_VIEWERS:
		H4P_SERVICE_ADAPTER(Viewers);
		break;
	
	case H4PE_GVCHANGE:
		H4P_TACT_BUTTON_CONNECTOR(reboot); //connect the reboot button

		if((svc == "wifi ssid") || (svc == "wifi psk")) { //triggered if wifi credentials changed
			Serial.print((std::string(svc.c_str()) + ": ").c_str()); //print what gv changed
			Serial.println(h4p.gvGetstring(svc.c_str()).c_str()); //print the new data
		}
		break;

	default:
		break;
	}
});
