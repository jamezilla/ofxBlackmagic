#pragma once;

#include "windows.h"
#include "cxtypes.h" // opencv types for colorspaces
#include "ofTexture.h"

class DLFrame
{
public:
    enum ColorSpace {
        DL_GRAYSCALE,  // grayscale
        DL_RGB         // RGB color
    };

    DLFrame(){};
    DLFrame(long width, long height, long row_bytes, ColorSpace color_space);
    DLFrame(BYTE* data, long width, long height, long row_bytes, ColorSpace color_space);
    // DLFrame(long width, long height, long row_bytes, ColorSpace color_space, bool bUseTexture = false);
    // DLFrame(BYTE* data, long width, long height, long row_bytes, ColorSpace color_space, bool bUseTexture = false);

    ~DLFrame();
    
    long            getWidth();
    long            getHeight();
    long            getRowBytes();
    BYTE*           getPixels();
	int             getOpenGLType();
	int             getOpenCVType();
    ColorSpace      getNativeType();
    // void            setUseTexture(bool bUse);
    // void            draw(float x, float y, float w, float h);
    // void            draw(float x, float y);
    // void            setAnchorPercent(float xPct, float yPct);
    // void            setAnchorPoint(int x, int y);
    // void            resetAnchor();
    
    BYTE*           pixels;
    long            width;
    long            height;

private:
    ColorSpace      _mColorSpace;
    long            _mRowBytes;
    // bool            _mUseTexture;
    // ofTexture       _mTex;

};


