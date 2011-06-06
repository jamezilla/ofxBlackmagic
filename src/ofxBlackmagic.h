#pragma once
#include <objbase.h>        // Necessary for COM
#include <vector>
#include "boost/shared_ptr.hpp"
#include "DeckLinkAPI_h.h"
#include "ofTexture.h"
#include "DLCapture.h"


// Valid parameters to setDisplayMode:
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



// forward declarations
class DLCard;
class DLFrame;

class ofxBlackmagic
{
public:
    ofxBlackmagic();
    ~ofxBlackmagic();

    void            listDevices();
    bool            isFrameNew();
    void            grabFrame();
    void            close();
    void            initGrabber(BMDDisplayMode displayMode, bool bTexture = true);
    unsigned char*  getPixels();
    void            setSize(int height, int width);
    void            setVerbose(bool bTalkToMe = true);
    void            setDeviceID(int _deviceID);
    bool            setDisplayMode(BMDDisplayMode displayMode);
    void            setUseTexture(bool bUse);
    void            draw(float x, float y, float w, float h);
    void            draw(float x, float y);
    void            setAnchorPercent(float xPct, float yPct);
    void            setAnchorPoint(int x, int y);
    void            resetAnchor();
    float           getHeight();
    float           getWidth();
    int             getFrameCount();
	float           getFrameRate();
	//int             getQueueDepth();

private:
    bool                       _mVerbose;
    std::vector<DLCard>        _mCards;
    DLCard*                    _mActiveCard;
	boost::shared_ptr<DLFrame>      _mRawFrame;
    bool                       _mRawFrameInitialized;
    bool                       _mNewFrame;
    bool                       _mUseTexture;
    ofTexture                  _mTex;
};
