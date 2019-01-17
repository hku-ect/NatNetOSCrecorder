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
    settings.blocking = false;
    
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
    
    // OSC playback
    isPlaying = false;
    counter = 0;

}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    int packetSize = 1000;
    char udpMessage[packetSize];
    // as advised here: https://forum.openframeworks.cc/t/receiving-a-udp-broadcast/16781/5
    memset(udpMessage,0,sizeof(udpMessage));
    
    // recieve is blocking so no if no message is recieved nothing happens
    int size = udpConnection.Receive(udpMessage,packetSize);
    
    cout << SOCKET_TIMEOUT << " size: " << size << endl;
    
    // --> stop char array's in vector
    // --> dump vector op de harde schijf. 
    
    
    // check for content only send if there is a message
    if(size != 0 && size != -1)
    {
        
        // print bytes
        /*
        std::cout << "Packet Size: " << size << std::endl;
        std::cout << "UDP Packet: " << std::endl;
        for(int i = 0; i < size; i++) {
            printf("0x%02X \n", (unsigned char)udpMessage[i]);
        }
        */
        
        
        // convert bytes to string
        //std::string s( reinterpret_cast<char const*>(udpMessage), size ) ;
        //cout << "string: " << s << endl;
        
        // send the messag through to another ip/port
        //udpSender.Send(udpMessage,size);
        
        // record message to vector
        if(isRecording){
            
            ofLogNotice("Packet Size: "+ofToString(size));
            
            // output the udp message to the console
            string msg = udpMessage;
            cout << "RECORDING: "<< msg << endl;
            
            oscMessageStruct message;
            message.size = size;
            message.timestamp = ofGetUnixTime();
            message.data = udpMessage;
            
            // add message to vector
            udpMessages.push_back(message);
        }
    }
    
    if(isPlaying == true && isRecording == false){
        
        
        ofLogNotice("Send: "+ofToString(counter)+" / "+ofToString(udpMessages.size()));
        
        // output the udp message to the console
        cout << "PLAYBACK: " << udpMessages[counter].timestamp << udpMessages[counter].size << " " << udpMessages[counter].data << endl;

        // sent the messages
        udpSender.Send(udpMessages[counter].data,udpMessages[counter].size);
        
        
        // Step through the recorded array
        counter ++;
        if(counter >= udpMessages.size()) counter = 0;
    }
    
    
   
    
    // save to disk
    // https://stackoverflow.com/questions/19195306/c-best-way-to-store-arrays-or-vectors-of-objects-on-disk-for-a-simple-database
    // https://openframeworks.cc/learning/01_basics/how_to_save_a_file/
    

   
    
    
    
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
        
        ofLogNotice("size: "+ofToString(udpMessages.size()));
        
        ofstream myFile;
        myFile.open (ofToDataPath("oscrecording.bin"), ios::out | ios::binary);
        if (myFile.is_open())
        {
            for(int i=0;i<udpMessages.size();i++)
            {
                myFile.write (udpMessages[i].data, udpMessages[i].size);
                cout << "WRITE: " << udpMessages[i].timestamp <<" " << udpMessages[i].size << " " << udpMessages[i].data << endl;
            }
        }
        myFile.close();
        
        
       
    }
    else if(isRecording == false){
        ofLogNotice("START Recording");
    }
    
    
    isRecording = ! isRecording;
    ofLogNotice("Record OSC data:"+ofToString(isRecording));
    
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
        
        if ( ImGui::Button(ICON_FA_PLAY_CIRCLE " Play last OSC recording") )
        {
            isPlaying = !isPlaying;
            ofLogNotice("Play status is: "+ofToString(isPlaying));
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::End();
        
               
        gui.end();
        this->mouseOverGui = mainSettings.mouseOverGui;
    }
}

