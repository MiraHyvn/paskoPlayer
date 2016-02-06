#include "ofApp.h"
#include <stdio.h>
#include <sstream>

using namespace std;

void ofApp::saveFrame(){
 if(frameN < 10000) {
  stringstream ss;
  string filename;

  ss << "frames/frame" << frameN << ".png";
  ss >> filename;

  kuva.saveImage(filename);

  cout << "Wrote " << filename << "\n";

  frameN++;
 }
 else cout<< "Frame limit exceeded\n";
}
//--------------------------------------------------------------
void ofApp::setup(){

 ofSetFrameRate(80);

 displayW = 800;
 displayH = 600;

 paskoPlayer.init("media/video");
 //kuva.allocate(paskoPlayer.getWidth(), paskoPlayer.getHeight(), OF_IMAGE_COLOR);
 cout << paskoPlayer.getDuration() << "\n";

 seekPos = 0;
 frameN = 0;
 recording = false;

}

//--------------------------------------------------------------
void ofApp::update(){

 if(paskoPlayer.getNextFrame() <0)
  paskoPlayer.rewind();

 while(!paskoPlayer.frameChanged() ) {
  if(paskoPlayer.getNextFrame() <0) {
   paskoPlayer.rewind();
   }
 }
 kuva.setFromPixels(paskoPlayer.getPixelData(), paskoPlayer.getWidth(), paskoPlayer.getHeight(), OF_IMAGE_COLOR);

 if(skipping)
  paskoPlayer.seek(seekPos);

 if(recording)
  saveFrame();
}

//--------------------------------------------------------------
void ofApp::draw(){
/*
 display = kuva;
 display.resize(displayW, displayH);
 display.draw(0,0);
*/
 kuva.draw(0,0);
}

//--------------------------------------------------------------

void ofApp::generoiKuva(){

 for(int i=0; i < kuva.getPixelsRef().size(); i++){
  kuva.getPixelsRef()[i] = rand()%255;
 }

 kuva.reloadTexture();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
 if( (char)key == 'r')
  recording = !recording;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){


 int fr = (100 * y / displayH);
 if(fr<5) fr=5;

 ofSetFrameRate(fr);

 seekPos = paskoPlayer.getDuration() * x / displayW;

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

 int fr = (100 * y / displayH);
 if(fr<5) fr=5;

 ofSetFrameRate(fr);

 seekPos = paskoPlayer.getDuration() * x / displayW;

 skipping = true;

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

 skipping = false;

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
 displayW = w;
 displayH = h;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
