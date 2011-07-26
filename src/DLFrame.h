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


