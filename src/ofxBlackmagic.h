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

#pragma once
#include <objbase.h>        // Necessary for COM
#include <vector>
#include "boost/shared_ptr.hpp"
#include "DeckLinkAPI_h.h"

////////////////////////////////////////////////////////////////////////////////
// Valid parameters to setDisplayMode
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

// forward declarations
class DLCard;
class DLFrame;
class ofTexture;

class ofxBlackmagic
{
public:
    ofxBlackmagic();
    ~ofxBlackmagic();

    void            close();                                     // stop capture
    void            draw(float x, float y, float w, float h);    // draw the pic to screen
    void            draw(float x, float y);                    
    int             getFrameCount();                             // get the # of captured frames
	float           getFrameRate();                              // calculate the capture frame rate
    float           getHeight();                                 // get the height of the processed image
    float           getWidth();                                  // get the width of the processed image
    unsigned char*  getPixels();                                 // get a pointer to the image data
    void            grabFrame();                                 // pull the next captured frame
    void            initGrabber(bool bTexture = true);           // start image capture
    bool            isFrameNew();                                // is this a new image, or just the last one captured?
    void            listDevices();                               // dump some device data
    void            resetAnchor();                               // reset the point coordinates where images are drawn
    void            setAnchorPercent(float xPct, float yPct);    // set the coordinates where images are drawn (as a percentage)
    void            setAnchorPoint(int x, int y);                // set the coordinates where images are drawn (as a fixed point)
    void            setDeviceID(int _deviceID);                  // pick which decklink device to capture from
    bool            setDisplayMode(BMDDisplayMode displayMode);  // pick the hardware display mode (see table above)
    bool            setPixelFormat(BMDPixelFormat pixelFormat);  // pick the hardware pixel format (not all cards can change this)
    void            setSize(int height, int width);              // software image resize
    void            setVerbose(bool bTalkToMe = true);           // print a bunch of junk out
    void            setUseTexture(bool bUse);                    // load the captured frame to a texture

private:
    bool                       _mVerbose;
    std::vector<DLCard>        _mCards;
    DLCard*                    _mActiveCard;
	boost::shared_ptr<DLFrame> _mRawFrame;
    bool                       _mRawFrameInitialized;
    bool                       _mNewFrame;
    bool                       _mUseTexture;
    ofTexture                  _mTex;
};
