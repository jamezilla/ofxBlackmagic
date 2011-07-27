// Copyright (c) 2011, James Hughes
// All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <iomanip>
#include <queue>

#include "ofTexture.h"
#include "ofxBlackmagic.h"
#include "DLCapture.h"
#include "DLCard.h"
#include "DLFrame.h"

#define COLUMN_WIDTH 35
using namespace std;

ofxBlackmagic::ofxBlackmagic()
{
	IDeckLinkIterator*			deckLinkIterator;
	IDeckLinkAPIInformation*	deckLinkAPIInformation;
	IDeckLink*					deckLink;
	int							numDevices = 0;
	HRESULT						result;
	
	// Initialize COM on this thread
	result = CoInitialize(NULL);
	if (FAILED(result))
    {
		cout << "Initialization of COM failed - result = " << result << endl;
		// TODO: convert to exception?
		std::exit(1);
	}
	
	// Create an IDeckLinkIterator object to enumerate all DeckLink cards in the system
	result = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&deckLinkIterator);
	if (FAILED(result))
	{
		cout << "A DeckLink iterator could not be created.  The DeckLink drivers may not be installed." << endl;
		// TODO: convert to exception?
		std::exit(1);
	}
	
	// We can get the version of the API like this:
	result = deckLinkIterator->QueryInterface(IID_IDeckLinkAPIInformation, (void**)&deckLinkAPIInformation);
	if (result == S_OK)
	{
		LONGLONG		deckLinkVersion;
		int				dlVerMajor, dlVerMinor, dlVerPoint;
		
		// We can also use the BMDDeckLinkAPIVersion flag with GetString
		deckLinkAPIInformation->GetInt(BMDDeckLinkAPIVersion, &deckLinkVersion);
		
		dlVerMajor = (deckLinkVersion & 0xFF000000) >> 24;
		dlVerMinor = (deckLinkVersion & 0x00FF0000) >> 16;
		dlVerPoint = (deckLinkVersion & 0x0000FF00) >> 8;
		
		cout << setw(COLUMN_WIDTH) << left << "DeckLinkAPI version: " << dlVerMajor << "." << dlVerMinor << "." << dlVerPoint << endl;
		
		deckLinkAPIInformation->Release();
	}

	// Enumerate all cards in this system
	while (deckLinkIterator->Next(&deckLink) == S_OK)
	{
		numDevices++;
        DLCard dlc(deckLink);
        _mCards.push_back(dlc);
	}
	
	// If no DeckLink cards were found in the system, inform the user
	if (numDevices == 0)
		cout << "No Blackmagic Design devices were found." << endl;
	
    setDeviceID(0);
    setVerbose(false);
    _mNewFrame   = false;
	_mUseTexture = true;
}

ofxBlackmagic::~ofxBlackmagic()
{
	// Uninitalize COM on this thread
	CoUninitialize();
}

void ofxBlackmagic::listDevices(void)
{
    cout << endl << "DEVICES" << endl;
    
    for(unsigned int i=0; i<_mCards.size(); i++) {
        cout << "[" << i << "] ";
		_mCards[i].print_name();

        if(_mVerbose) {
            // ** Print all DeckLink Attributes
            _mCards[i].print_attributes();
		
            // ** List the video output display modes supported by the card
            _mCards[i].print_output_modes();
		
            // ** List the input and output capabilities of the card
            _mCards[i].print_capabilities();
        }
	}
}

void ofxBlackmagic::setVerbose(bool bTalkToMe)
{
    _mVerbose = bTalkToMe;
}

void ofxBlackmagic::setDeviceID(int _deviceID)
{
    _mActiveCard = &(_mCards[_deviceID]);
}

bool ofxBlackmagic::setDisplayMode(BMDDisplayMode displayMode)
{
	return _mActiveCard->setDisplayMode(displayMode);
}

bool ofxBlackmagic::setPixelFormat(BMDPixelFormat pixelFormat)
{
    return _mActiveCard->setPixelFormat(pixelFormat);
}

void ofxBlackmagic::initGrabber(bool bTexture)
{
	_mActiveCard->initGrabber();
	setUseTexture(bTexture);
}

void ofxBlackmagic::close(void)
{
	_mActiveCard->close();
}

bool ofxBlackmagic::isFrameNew(){
    return _mNewFrame;
}

void ofxBlackmagic::grabFrame()
{
    // we don't have ANYTHING to send to the client
    if(_mRawFrameInitialized == false){
        // idle until we have SOMETHING
        while(_mActiveCard->m_pDelegate->getFrame(_mRawFrame))
            Sleep(10);
        // from now on we can send stale stuff
        _mRawFrameInitialized = true;
	} else if(_mActiveCard->m_pDelegate->getFrame(_mRawFrame)){
		// TODO: test with with texture data loading in the background
		_mTex.loadData(_mRawFrame->getPixels(), _mRawFrame->getWidth(), _mRawFrame->getHeight(), _mRawFrame->getOpenGLType());
		_mNewFrame = true;
	} else {
		_mNewFrame = false;
	}
}

float ofxBlackmagic::getWidth()
{
	// TODO: make the delegate properties private
	return (float)_mActiveCard->m_pDelegate->getWidth();
}

float ofxBlackmagic::getHeight()
{
	// TODO: make the delegate properties private
	return (float)_mActiveCard->m_pDelegate->getHeight();
}

void ofxBlackmagic::setSize(int new_width, int new_height)
{
    _mActiveCard->m_pDelegate->setSize(new_width, new_height);
}

unsigned char* ofxBlackmagic::getPixels()
{
	if(_mRawFrame == NULL)
		return NULL;
    return _mRawFrame->getPixels();
}

int ofxBlackmagic::getFrameCount() 
{
    return _mActiveCard->m_pDelegate->getFrameCount();
}

float ofxBlackmagic::getFrameRate() 
{
    return _mActiveCard->m_pDelegate->getFrameRate();
}

//int ofxBlackmagic::getQueueDepth()
//{
//	return _mActiveCard->m_pDelegate->getPreviewQueueSize();
//}

void ofxBlackmagic::setUseTexture(bool bUse)
{
	_mUseTexture = bUse;
	if(bUse == true) {
		int width = (int)_mActiveCard->m_pDelegate->getWidth();
		int height = (int)_mActiveCard->m_pDelegate->getHeight();
		_mTex.allocate(width, height,GL_RGB);
	}
}

void ofxBlackmagic::setAnchorPercent(float xPct, float yPct)
{
    if (_mUseTexture)_mTex.setAnchorPercent(xPct, yPct);
}

// TODO: what's all this casting about?
void ofxBlackmagic::setAnchorPoint(int x, int y)
{
    if (_mUseTexture)_mTex.setAnchorPoint((float)x, (float)y);
}

void ofxBlackmagic::resetAnchor()
{
   	if (_mUseTexture)_mTex.resetAnchor();
}

void ofxBlackmagic::draw(float _x, float _y, float _w, float _h)
{
	if (_mUseTexture){
		_mTex.draw(_x, _y, _w, _h);
	}
}

void ofxBlackmagic::draw(float _x, float _y)
{
	float width = (float)_mActiveCard->m_pDelegate->getWidth();
	float height = (float)_mActiveCard->m_pDelegate->getHeight();
	draw(_x, _y, width, height);
}
//
//void ofxBlackmagic::startRecording(void)
//{
//    _mActiveCard->m_pDelegate->startRecording();
//}
//
//boost::shared_ptr<MovieInfo> ofxBlackmagic::stopRecording()
//{
//    return _mActiveCard->m_pDelegate->stopRecording();
//}
//
//bool ofxBlackmagic::isRecording()
//{
//	return _mActiveCard->m_pDelegate->isRecording();
//}
