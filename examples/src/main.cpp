#include "ofMain.h"
#include "blackmagicExampleApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main(int ac, char* av[]){

    //
    // Windowing setup
    //

    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1920,1080, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new BlackmagicExampleApp());

}
