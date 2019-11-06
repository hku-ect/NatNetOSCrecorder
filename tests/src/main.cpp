#include "ofxUnitTests.h"
#include "ofxOsc.h"
#include "ofxNetwork.h"
#include <iostream>
#include <stdio.h>

#define PORT 6000

class ofApp: public ofxUnitTestsApp{
	void run(){
        ofxUDPManager udpReceiver;
        ofxUDPManager udpSender;

        ofxUDPSettings settingsRecv;
        settingsRecv.receiveOn(PORT);
        settingsRecv.blocking = false;
        udpReceiver.Setup(settingsRecv);

        ofxUDPSettings sett;
        sett.sendTo("127.0.0.1", PORT);
        udpSender.Setup(sett);

        { //  test if receive size = 0
            int packetSize = 1000;
            char udpMessage[packetSize];
            // as advised here: https://forum.openframeworks.cc/t/receiving-a-udp-broadcast/16781/5
            memset(udpMessage,0,sizeof(udpMessage));

            int size = udpReceiver.Receive(udpMessage, packetSize );

            ofxTestEq( size, 0, "no udp packets means size is 0" );
        }

        {   // construct an OSC packet, send and receive
            char buffer[1024] = "/ping";
            int needle = strlen(buffer)+1;
            char sig[] = "fsi";
            std::memcpy(&buffer[needle], &sig, strlen(sig)+1);
            needle += strlen(sig)+1;
            float f = 1.0f;
            std::memcpy(&buffer[needle], &f, sizeof(float));
            needle += sizeof(float);
            char hello[] = "hello";
            std::memcpy(&buffer[needle], &hello, strlen(hello)+1);
            needle += strlen(hello)+1;
            int32_t t = 2;
            std::memcpy(&buffer[needle], &t, sizeof(int32_t));
            needle += sizeof(int32_t);
            ofxTestEq(needle, 24, "needle should  be 24");


            int rc = udpSender.Send(buffer, needle);
            ofxTestEq( rc, 24, "Send returns the size if succesful");
            //  give os time to process the message
            ofSleepMillis(10);

            char recvBuf[1024];
            memset(recvBuf,0,1024);
            rc = udpReceiver.Receive(recvBuf, 1024);
            ofxTestEq( rc, needle, "receive returns the size of the buffer" );

            ofxTest( strcmp(recvBuf, "/ping") == 0, "the header of the package is a string");
            int needle2 = strlen(recvBuf) + 1;
            ofxTestEq(needle2, 6, "needle should  be 6");

            char sigtest[4];
            std::memcpy(sigtest, &recvBuf[needle2], strlen(sig)+1);
            needle2 += strlen(sigtest)+1;
            ofxTestEq( strlen(sigtest), 3, "strlen is 3");
            ofxTest( strcmp(sigtest, sig) == 0, "the signature should be 'fsi'");
            ofxTestEq(needle2, 10, "needle should  be 10");

            float ftest = 0.f;
            std::memcpy(&ftest, &recvBuf[needle2], sizeof (float) );
            needle2 += sizeof (float);
            ofxTestEq(ftest, 1.0f, "the received float should be 1.0f");

            char hellotest[6];
            std::memcpy(hellotest, &recvBuf[needle2], strlen(hello)+1);
            needle2 += strlen(hellotest) + 1;
            ofxTest( strcmp( hellotest, "hello") == 0, "the received string should be 'hello'");

            int32_t ttest = 0;
            std::memcpy(&ttest, &recvBuf[needle2], sizeof(int32_t));
            needle2 += sizeof(int32_t);
            ofxTestEq(ttest, 2, "the received int should be 2");
            ofxTestEq(needle2, 24, "needle should  be 24");
        }

	}
};


#include "ofAppNoWindow.h"
#include "ofAppRunner.h"
//========================================================================
int main( ){
	ofInit();
	auto window = std::make_shared<ofAppNoWindow>();
	auto app = std::make_shared<ofApp>();
	ofRunApp(window, app);
	return ofRunMainLoop();
}
