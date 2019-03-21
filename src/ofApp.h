#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOsc.h"
#include "ofxImGui.h"
#include "ofxNetwork.h"

// listening port
#define PORT 6000

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
        void setRecord();
        void loadRecording(int index);
        void addMessageToArray(char* packet, int size);
        bool isRecording;
        bool isPlaying;
        std::vector<oscMessageStruct> udpMessages;
        int counter;
    
        ofxOscReceiver receiver;
        ofxUDPManager udpConnection;
        ofxUDPManager udpSender;
    
        ofXml xml;
    
        //File display
        ofDirectory dir;
        string recordingDir;
        std::vector<std::string> fileNames;
        std::vector<ofFile> files;
    
        //GUI
        ofxImGui::Gui gui;
        bool guiVisible;
        bool mouseOverGui;
        void doGui();
        ImFont* fontDefault;
        ImFont* fontSubTitle;
        ImFont* fontTitle;
    
        void SerializeInt32(char (&buf)[4], u_int32_t val);
        u_int32_t ParseInt32(const char (&buf)[4]);
    
        int count;

		
};
