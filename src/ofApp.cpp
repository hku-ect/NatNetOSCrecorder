#include "ofApp.h"
#include "fontawesome5.h"
#include "themes.h"

static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetWindowTitle("oscRecorder");
    ofSetFrameRate(30); // run at 60 fps
    ofSetVerticalSync(true);
    
    // listen on the given port
    ofLog() << "listening for osc messages on port " << PORT;
    //receiver.setup(PORT);
    
    // SETUP UDP
    //create the socket and bind to port 11999
    ofxUDPSettings settings;
    settings.receiveOn(PORT);
    settings.blocking = true;
    
    udpConnection.Setup(settings);
    
    ofxUDPSettings sett;
    
    sett.sendTo("127.0.0.1", 7777);
    udpSender.Setup(sett);

    
   

    
    
    // SETUP GUI
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;                  // no imgui.ini
    fontDefault = io.Fonts->AddFontDefault();
    ImFontConfig config;
    config.MergeMode = true;
    //config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
    config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF(ofToDataPath( FONT_ICON_FILE_NAME_FAS ).data(), 12.0f, &config, icon_ranges);
    
    string t = ofFilePath::getAbsolutePath("verdana.ttf");
    fontSubTitle = io.Fonts->AddFontFromFileTTF(t.c_str(), 16.0f);
    fontTitle = io.Fonts->AddFontFromFileTTF(t.c_str(), 18.0f);
    gui.setup(new GuiGreenTheme(), false);              // default theme, no autoDraw!
    
    guiVisible = true;
    isRecording = false;


}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    char udpMessage[100000];
    int size = udpConnection.Receive(udpMessage,100000);
    
    ofLogNotice(ofToString(size));
    
    string message=udpMessage;
    if(message!=""){
        ofLogNotice(message);
    }
    
    udpSender.Send(udpMessage,size);
    
    
    /*
    // check for waiting messages
    while(receiver.hasWaitingMessages()){
        
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        recordOSC(m);
    }
    */

    
    doGui();

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if ( this->guiVisible ) { gui.draw(); }

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::setRecord(){
    
    if(isRecording == true){
        ofLogNotice("STOP Recording and save");
        
        map<int,ofxOscMessage>::iterator it;
        for (it=OSCframes.begin(); it!=OSCframes.end(); ++it){
            // it->first contains the key
            cout << " this is the key " << it->first << endl;
            // it->second contains the value
            //it->second;
        }
    }
    else if(isRecording == false){
        ofLogNotice("START Recording");
    }
    
    
    isRecording = ! isRecording;
    ofLogNotice("Record OSC data:"+ofToString(isRecording));
    
}

void ofApp::recordOSC(ofxOscMessage m){
    
    if(isRecording){
        // use global bundle
        ofLogNotice(ofGetTimestampString()+" "+ofToString(ofGetSystemTimeMillis()));
        
        // recordBundle
        OSCframes[ofGetSystemTimeMillis()] = m;
    }
}

static bool version_popup = false;

void ofApp::doGui() {
    this->mouseOverGui = false;
    if (this->guiVisible)
    {
        auto mainSettings = ofxImGui::Settings();
        //ui stuff
        gui.begin();
        // Create a main menu bar
        float mainmenu_height = 0;
        if (ImGui::BeginMainMenuBar())
        {
            
            mainmenu_height = ImGui::GetWindowSize().y;
            ImGui::EndMainMenuBar();
        }
        
      
        
        // right dock
        ImGui::SetNextWindowPos(ImVec2( 0, mainmenu_height ));
        ImGui::SetNextWindowSize(ImVec2( 350, ofGetHeight()-mainmenu_height));
        ImGui::Begin("rightpanel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar);
        
        ImGui::PushFont(fontSubTitle);
        ImGui::Text("Global Settings");
        ImGui::PopFont();
        
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        
        
        if ( ImGui::Button(ICON_FA_PLAY_CIRCLE " Record OSC") )
        {
            setRecord();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::End();
        
               
        gui.end();
        this->mouseOverGui = mainSettings.mouseOverGui;
    }
}

