#include "ofApp.h"
#include "fontawesome5.h"
#include "themes.h"

static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };


// TODO: playback in correct speed
// TODO: make gui and logic to select port  / ip / network interface
// TODO: playback part of recording ?



/*
 Reading Writeing:
 http://forums.codeguru.com/showthread.php?269648-C-Structure-How-do-I-write-a-structure-to-a-file <-- !!!!!!
 http://www.cplusplus.com/forum/beginner/149195/
 https://stackoverflow.com/questions/4300173/reading-and-writing-a-vector-of-structs-to-file
 http://www.cplusplus.com/articles/DzywvCM9/
 https://isocpp.org/wiki/faq/serialization#serialize-overview --> serialisation
 https://www.geeksforgeeks.org/readwrite-structure-file-c/ --> read/write in c
 
 
 
 */

//--------------------------------------------------------------



void ofApp::setup(){
    
    ofSetWindowTitle("oscRecorder");
    ofSetFrameRate(30); // run at 60 fps
    ofSetVerticalSync(true);
    
    recordingDir = "oscRecordings";
    selectedRecoding = "none";
    
    // listen on the given port
    ofLog() << "listening for osc messages on port " << INCOMING_PORT;

    ofxUDPSettings sett;
    
    sett.sendTo(OUTGOING_IP, OUTGOING_PORT);
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
    
    // File listing
    dir.open(ofFilePath::getAbsolutePath(recordingDir));
    dir.allowExt("bin");
    updateFileListing();
}

void ofApp::setupOSCRecorder()
{
    udpMessages.clear();
    messageCount = 0;

    ofxUDPSettings settings;
    settings.receiveOn(this->oscListenPort);
    settings.blocking = false;
    udpReceiver.Setup(settings);
}

//  this is a blocking call so no data means the app freezes?
void ofApp::processOSCRecorder()
{
    int packetSize = 1000;
    char udpMessage[packetSize];
    // as advised here: https://forum.openframeworks.cc/t/receiving-a-udp-broadcast/16781/5
    memset(udpMessage,0,sizeof(udpMessage));

    // recieve is blocking so no if no message is recieved nothing happens
    int size = udpReceiver.Receive(udpMessage,packetSize);

    // check for content only send if there is a message
    if(size != 0 && size != -1)
    {
        // record message to vector
        if(isRecording){

            //ofLogNotice("Update :: Packet Size: "+ofToString(size));

            // output the udp message to the console
            string msg = udpMessage;
            cout << "RECORDING: "<< messageCount << "msg: " << msg << endl;


            //cout << "Update :: " << endl;
            //printUDPpacket(udpMessage,size);

            // Add message to messages array to store on disk
            addMessageToArray(udpMessage, size);
            messageCount++;
        }
    }
}

void ofApp::destroyOSCRecorder()
{
    udpReceiver.Close();
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if ( isRecording )
    {
        processOSCRecorder();
    }

    // Playback recorded UDP messages
    // only if: we are not recording we have recorded something.
    if(isPlaying == true && isRecording == false ){
        
        ofLogNotice("Send: "+ofToString(counter)+" / "+ofToString(udpMessages.size()));
        
        // output the udp message to the console
        cout << "PLAYBACK: " << udpMessages[counter].timestamp << udpMessages[counter].size << " " << udpMessages[counter].data << endl;

        // sent the messages
        udpSender.Send(udpMessages[counter].data,udpMessages[counter].size);
        
        
        // Step through the recorded array
        counter ++;
        if(counter >= udpMessages.size()) counter = 0;
      
    }
    
        
    doGui();
    
    
   
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if ( this->guiVisible ) { gui.draw(); }
    
    ofDrawBitmapString("Selected OSC recording", 400,70);
    ofDrawBitmapString(selectedRecoding, 400,100);
    ofDrawBitmapString("Frame: "+ofToString(counter)+"/"+ofToString(udpMessages.size()), 400,120);
    
    

    
    
}

//--------------------------------------------------------------
void ofApp::printUDPpacket(char* packet, int size){
    
  
    
    std::cout << "Packet Size: " << size << std::endl;
    

    std::cout << "UDP Packet: " << std::endl;
    for(int i = 0; i < size; i++) {
        printf("0x%02X \n", (unsigned char)packet[i]);
    }
    cout << "end packet" << endl;
    cout << "-----------------------------" << endl;

    
}

//--------------------------------------------------------------
void ofApp::loadRecording(int index){
    
    ofLog() << " VectorListBox FILE PATH: "  << files[index].getAbsolutePath();
    
    selectedRecoding = files[index].getFileName();
    
    // clear udpmessages array
    udpMessages.clear();
    
    // set message counter to 0
    counter = 0;
    
    // read the file
    std::ifstream infile (files[index].getAbsolutePath(), std::ifstream::binary);
    
    // var to store number of messages in file
    int sizeOfVector;
    
    // read number of messages in file
    infile.read(reinterpret_cast<char*>(&sizeOfVector), sizeof(u_int32_t));
    
    // loop through all the messages in file
    for(int i=0;i<sizeOfVector;i++)
    {
        // create empty OSCmessageStruct
        oscMessageStruct entry;
        
        // Read the size of the message
        infile.read(reinterpret_cast<char*>(&entry.size), sizeof(u_int32_t));
        // Read the timestamp of the message
        infile.read(reinterpret_cast<char*>(&entry.timestamp), sizeof(u_int32_t));
        
        // set the size of th emessage
        entry.data = new char[entry.size];
        // read the message
        infile.read(entry.data, entry.size);
        
        // add entry to vector
        udpMessages.push_back(entry);
        
    }
    infile.close();
    
    cout << "sizeOfVector: " << udpMessages.size() << endl;

}

//---------------------------ofToString(udpMessages.size()-----------------------------------
void ofApp::addMessageToArray(char* packet, int size){
    
    
    // create oscMessage object to add to vector
    oscMessageStruct message;
    message.size = size;
    message.timestamp = ofGetUnixTime();
    
    // allocate size for the udpmessage
    message.data = new char[size];
    
    // copy message
    memcpy(message.data, packet, size);
    
    // add message to vector
    udpMessages.push_back(message);
    
    // print bytes to console
    cout <<" addMessageToArray: " << endl;
    printUDPpacket(packet,size);
    
}

//--------------------------------------------------------------
void ofApp::updateFileListing(){
    
    //update file listing
    dir.listDir();
    
    fileNames.clear();
    files.clear();
    files = dir.getFiles();
    for(size_t i=0; i<files.size(); i++)
    {
        fileNames.push_back(files[i].getFileName());
    }

    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    if(key == 'q')
    {
        ofDirectory dir;
        dir.open(ofFilePath::getAbsolutePath(recordingDir));
        dir.allowExt("bin");
        int temp = dir.listDir();
        dir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order
        cout << "numFiles: " << temp << " dir: " << dir.getAbsolutePath() << endl;
        
        
    }

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


void ofApp::SerializeInt32(char (&buf)[4], u_int32_t val)
{
    std::memcpy(buf, &val, 4);
}

u_int32_t ofApp:: ParseInt32(const char (&buf)[4])
{
    u_int32_t val;
    std::memcpy(&val, buf, 4);
    return val;
}

void ofApp::saveRecording()
{
    if(isRecording == true){
        isRecording = false;
    }
    if ( messageCount )
    {
        string fileName = recordingDir+"/oscrecording_"+ofToString(ofGetUnixTime())+".bin";
        ofLogNotice("Save recording to disk: " + fileName + " messages size: " + ofToString(udpMessages.size() ) );
        
        ofstream myFile;
        myFile.open (ofToDataPath(fileName), ios::out | ios::binary);
        if (myFile.is_open())
        {
            cout << "#####-----------------------------------------------" << endl;
            cout << "SAVING UDP messages" << endl;
            
            // 1. Write lenght of vector
            // ---
            u_int32_t size = udpMessages.size();
            myFile.write((char*) &size,sizeof(u_int32_t));
            
            // 2. write vector entries
            for(int i=0;i<udpMessages.size();i++)
            {

                // write elements ove the vector entry
                myFile.write((char*)&udpMessages[i].size,sizeof(u_int32_t));
                myFile.write((char*)&udpMessages[i].timestamp,sizeof(u_int32_t));
                myFile.write(udpMessages[i].data,udpMessages[i].size);
                
                // print bytes to console
                //printUDPpacket(udpMessages[i].data,udpMessages[i].size);
                
                cout << "size to save " << sizeof(struct oscMessageStruct) << endl;
            }
        }
        // close file
        myFile.flush();
        myFile.close();
        cout << "file closed" << endl;
        
        //update file listing
        updateFileListing();

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
            if (ImGui::BeginMenu("File"))
            {
                //if (ImGui::MenuItem("Open Recording..", "Ctrl+O")) { loadAFile(); }
                //if (ImGui::MenuItem("Save Recording", "Ctrl+S"))   {saveData(); }
                if (ImGui::MenuItem("About", "Ctrl+i")) { version_popup=true; }
                if (ImGui::MenuItem("Exit", "Ctrl+W"))  { ofExit(0); }
                ImGui::EndMenu();
            }
            mainmenu_height = ImGui::GetWindowSize().y;
            ImGui::EndMainMenuBar();
        }
                
        // right dock
        ImGui::SetNextWindowPos(ImVec2( 0, mainmenu_height ));
        ImGui::SetNextWindowSize(ImVec2( 350, ofGetHeight()-mainmenu_height));
        ImGui::Begin("rightpanel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar );

        if ( ImGui::CollapsingHeader("Recorder Settings", NULL, ImGuiTreeNodeFlags_DefaultOpen) )
        {
            ImGui::InputInt("OSC listen port", &oscListenPort, 1, 100 );
            ImGui::InputText("OSC destination host", oscDestHost, 200);
            ImGui::InputInt("OSC destination port", &oscDestPort, 1, 100 );

            //ImGui::Text("incoming OSC port: %s",ofToString(INCOMING_PORT).c_str());
            //ImGui::Text("outgoing OSC port: %s",ofToString(OUTGOING_PORT).c_str());
            //ImGui::Text("outgoing address: %s",ofToString(OUTGOING_IP).c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Record OSC");
                
        // Toggle OSC recording
        // FIXME: make a proper toggle button
        bool wasRecording = isRecording;
        if (wasRecording)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1.0f, 0.8f, 0.8f));
        }
        static float recordStart;
        if ( ImGui::Button(ICON_FA_CIRCLE " Record OSC") )
        {
            if ( ! isRecording )
            {
                setupOSCRecorder();
                isRecording = true;
                recordStart = ImGui::GetTime();
            }
            else
            {
                saveRecording();
                destroyOSCRecorder();
                isRecording = false;
            }
        }
        if (wasRecording)
        {
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            ImGui::Text("Recording %c %.2fs", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3], ImGui::GetTime()-recordStart );
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        
        ImGui::PushFont(fontSubTitle);
        ImGui::Text("Play OSC");
        ImGui::PopFont();
        
        // Play OSC recording
        // This now sets a boolean isPlaying which loops through udpMessages array when set to true
        if ( ImGui::Button(ICON_FA_PLAY_CIRCLE " Play last OSC recording") )
        {
            if(udpMessages.size() > 0){
                isPlaying = !isPlaying;
                ofLogNotice("Play status is: "+ofToString(isPlaying));
            }
            else
            {
                ofLogWarning("No recording present so, Nothing to playback!");
            }
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        
        //ofxImGui::VectorListBox allows for the use of a vector<string> as a data source
        static int currentListBoxIndex = 0;
        if(ofxImGui::VectorListBox("OSC recordings", &currentListBoxIndex, fileNames))
        {
            // Is triggered when selecting a file form the list
            // Load file
            loadRecording(currentListBoxIndex);
        }
        
        if ( version_popup )
            ImGui::ShowDemoWindow(&version_popup);
        
        ImGui::End();
        
               
        gui.end();
        this->mouseOverGui = mainSettings.mouseOverGui;
    }
}

