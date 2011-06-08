#include "BlackmagicExampleApp.h"

//--------------------------------------------------------------
void BlackmagicExampleApp::setup(){

    mZoom = false;

    // list the blackmagic devices and their capabilities
    mVidGrabber.listDevices();

	//
    // Camera Setup
    //
    
	mVidGrabber.setVerbose();
    mVidGrabber.setDeviceID(0);

    ////////////////////////////////////////////////////////////////////////////////
    // Valid parameters to setDisplayMode()
    //   - match this to your camera's configured output mode
    //
    //                                     Frames      Fields  Suggested
    //                                     per         per     Time        Display
    // Mode                Width   Height  Second      Frame   Scale       Duration
    // ----------------------------------------------------------------------------
    // bmdModeNTSC         720     486     30/1.001    2       30000       1001
    // bmdModeNTSC2398     720     486     30/1.001*   2       24000*      1001
    // bmdModePAL          720     576     25          2       25          1
    // bmdModeHD720p50     1280    720     50          1       50          1
    // bmdModeHD720p5994   1280    720     60/1.001    1       60000       1001
    // bmdModeHD720p60     1280    720     60          1       60          1
    // bmdModeHD1080p2398  1920    1080    24/1.001    2       24000       1001
    // bmdModeHD1080p24    1920    1080    24          2       24          1
    // bmdModeHD1080p25    1920    1080    25          1       25000       1000
    // bmdModeHD1080p2997  1920    1080    30/1.001    1       30000       1001
    // bmdModeHD1080p30    1920    1080    30          1       30000       1000
    // bmdModeHD1080i50    1920    1080    25          2       25          1
    // bmdModeHD1080i5994  1920    1080    30/1.001    2       30000       1001
    // bmdModeHD1080i6000  1920    1080    30          2       30000       1000
    // bmdModeHD1080p50    1920    1080    50          1       50000       1000
    // bmdModeHD1080p5994  1920    1080    60/1.001    1       60000       1001
    // bmdModeHD1080p6000  1920    1080    60          1       60000       1000
    // bmdMode2k2398       2048    1556    24/1.001    2       24000       1001
    // bmdMode2k24         2048    1556    24          2       24          1
    // bmdMode2k25         2048    1556    25          2       25000       1000
    /////////////////////////////////////////////////////////////////////////////////

    // capture at 1080i, 29.97 fps - this has an internal call to setSize()
    // which sets the size to the full resolution of the capture mode
	mVidGrabber.setDisplayMode(bmdModeHD1080i5994);
	mVidGrabber.initGrabber();

    // Display at 360 x 243. This is a software resize! It's more efficient to
    // resize in hardware by passing dimensions to draw().
	// mVidGrabber.setSize(480, 270);

}

//--------------------------------------------------------------
void BlackmagicExampleApp::update(){
    mVidGrabber.grabFrame();

	if (mVidGrabber.isFrameNew()){
        // update some junk
	}

    ofSetWindowTitle("framerate: " + ofToString(ofGetFrameRate(),2)+ "fps");
}

//--------------------------------------------------------------
void BlackmagicExampleApp::draw(){
	// draw the video capture
    ofSetColor(0xffffff);
	mVidGrabber.draw(0.0f,0.0f);

	// display the actual rate of capture
	ofFill();
	ofSetColor(0x333333);
	ofRect(10,10,300,30);
	ofSetColor(0x00ff00);
	ofDrawBitmapString("Capture Framerate:   " + ofToString(mVidGrabber.getFrameRate()), 20, 30);
}

//--------------------------------------------------------------
void BlackmagicExampleApp::keyPressed(int key){

}

//--------------------------------------------------------------
void BlackmagicExampleApp::keyReleased(int key){
    switch(key) {
        case 'z':
            mZoom ? mVidGrabber.setSize(1920, 1080) : mVidGrabber.setSize(480, 270);
            mZoom = !mZoom;
            break;
    }
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

