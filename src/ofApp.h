#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOsc.h"
#include "ofxImGui.h"
#include "ofxNetwork.h"

// listening port
#define INCOMING_PORT 6000
#define OUTGOING_PORT 7777
#define OUTGOING_IP "127.0.0.1"

struct oscMessageStruct
{
    u_int32_t size;
    u_int32_t timestamp;
    char* data;
};


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void printUDPpacket(char* packet, int size);
    
        //OSC recording
        void saveRecording();
        void loadRecording(int index);
        void addMessageToArray(char* packet, int size);
        bool isRecording;
        bool isPlaying;
        std::vector<oscMessageStruct> udpMessages;
        int counter;
    
        ofxUDPManager udpReceiver;
        ofxUDPManager udpSender;
        void setupOSCSender();
        void setupOSCRecorder();
        void processOSCRecorder();
        void destroyOSCRecorder();

        int messageCount;   // counter for received messages;
        int oscListenPort = 6000;
        int oscDestPort = 6001;
        char oscDestHost[200] =  "127.0.0.1";


        ofXml xml;
    
        //File display
        ofDirectory dir;
        string recordingDir;
        int loadedFileIndex = -1;
        std::vector<ofFile> files;
        void updateFileListing();
    
        //GUI
        ofxImGui::Gui gui;
        bool guiVisible;
        bool mouseOverGui;
        void doGui();
        ImFont* fontDefault;
        ImFont* fontSubTitle;
        ImFont* fontTitle;
    
        string selectedRecoding;
    
        void SerializeInt32(char (&buf)[4], u_int32_t val);
        u_int32_t ParseInt32(const char (&buf)[4]);		
};
