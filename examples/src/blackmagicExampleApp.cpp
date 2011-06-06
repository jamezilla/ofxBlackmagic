#include "BlackmagicExampleApp.h"

//--------------------------------------------------------------
void BlackmagicExampleApp::setup(){

	mVidGrabber.listDevices();

	//
    // Camera Setup
    //
    
	//mGrayscaleImage.allocate(1920, 1080, OF_IMAGE_GRAYSCALE);
	mVidGrabber.setVerbose();
	mVidGrabber.listDevices();
    mVidGrabber.setDeviceID(0);
	//mVidGrabber.setSize(1920, 1080);
	mVidGrabber.initGrabber(bmdModeHD1080i5994);
	//mVidGrabber.setSize(360, 243);
	//mVidGrabber.initGrabber(bmdModeNTSC);
}

//--------------------------------------------------------------
void BlackmagicExampleApp::update(){
    mVidGrabber.grabFrame();
	if (mVidGrabber.isFrameNew()){
		// wrap the pixels in an ofImage for ease of passing around
		// TODO: get rid of this? we can probably just pass around a raw pointer + sizes
		//       would be good to get rid of, since wrapPixels is a non-standard extension
		//mGrayscaleImage.wrapPixels(mVidGrabber.getPixels(), false);
	}

    ofSetWindowTitle("framerate: " + ofToString(ofGetFrameRate(),2)+ "fps");
}

//--------------------------------------------------------------
void BlackmagicExampleApp::draw(){
	// draw the video capture
    ofSetColor(0xffffff);
	mVidGrabber.draw(0.0f,0.0f);

	ofFill();
	ofSetColor(0x333333);
	ofRect(10,10,300,40);
	ofSetColor(0x00ff00);
	// display the actual rate of capture
	ofDrawBitmapString("Capture Framerate:   " + ofToString(mVidGrabber.getFrameRate()), 20, 30);
	// display the size of the capture queue - it should be at or near zero if your code is keeping up with capture
	//ofDrawBitmapString("Capture Queue Depth: " + ofToString(mVidGrabber.getQueueDepth()), 20, 40);
}

//--------------------------------------------------------------
void BlackmagicExampleApp::keyPressed(int key){

}

//--------------------------------------------------------------
void BlackmagicExampleApp::keyReleased(int key){

}

//--------------------------------------------------------------
void BlackmagicExampleApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void BlackmagicExampleApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void BlackmagicExampleApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void BlackmagicExampleApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void BlackmagicExampleApp::windowResized(int w, int h){

}

