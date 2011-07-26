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

#include <exception>

#include "DLFrame.h"
#include "cxtypes.h" // opencv types for colorspaces

DLFrame::~DLFrame(){ delete pixels; }

DLFrame::DLFrame(long width, long height, long row_bytes, ColorSpace color_space)
//DLFrame::DLFrame(long width, long height, long row_bytes, ColorSpace color_space, bool bUseTexture)
{
    this->pixels      = new BYTE[height*row_bytes];
    this->width       = width;
    this->height      = height;
    _mRowBytes        = row_bytes;
    _mColorSpace      = color_space;
    //_mTex.loadData(getPixels(), (int)width, (int)height, getOpenGLType());
}

DLFrame::DLFrame(BYTE* data, long width, long height, long row_bytes, ColorSpace color_space)
//DLFrame::DLFrame(BYTE* data, long width, long height, long row_bytes, ColorSpace color_space, bool bUseTexture)
{
    this->pixels      = data;
    this->width       = width;
    this->height      = height;
    _mRowBytes        = row_bytes;
    _mColorSpace      = color_space;
    //_mTex.loadData(getPixels(), (int)width, (int)height, getOpenGLType());
}

long
DLFrame::getWidth()
{
    return width;
}

long
DLFrame::getHeight()
{
    return height;
}

unsigned char*
DLFrame::getPixels()
{
    return (unsigned char *) pixels;
}

DLFrame::ColorSpace
DLFrame::getNativeType()
{
    return _mColorSpace;
}

int
DLFrame::getOpenGLType()
{
    switch( _mColorSpace ) {
        case DL_GRAYSCALE:
            return GL_LUMINANCE;
        case DL_RGB:
            return GL_RGB;
    }

    // shouldn't get here
    std::cout << "DLFrame - _mColorSpace is set to something illegal" << std::endl;
	throw;
}

int
DLFrame::getOpenCVType()
{
    switch( _mColorSpace ) {
        case DL_GRAYSCALE:
            return CV_8UC1;
        case DL_RGB:
            return CV_8UC3;
    }

    // shouldn't get here
    std::cout << "DLFrame - colorSpace is set to something illegal" << std::endl;
    throw;
}

// void
// DLFrame::setUseTexture(bool bUse)
// {
// 	_mUseTexture = bUse;
// 	if(bUse == true)
// 		_mTex.allocate(width,height,getOpenGLType());
// }

// void
// DLFrame::setAnchorPercent(float xPct, float yPct)
// {
//     if (_mUseTexture)
//         _mTex.setAnchorPercent(xPct, yPct);
//     else {
//         std::cout << "DLFrame - tried to set a texture property but you're not using a texture!" << std::endl;
//         throw;
//     }
// }

// void
// DLFrame::setAnchorPoint(int x, int y)
// {
//     if (_mUseTexture)
//         _mTex.setAnchorPoint(x, y);
//     else {
//         std::cout << "DLFrame - tried to set a texture property but you're not using a texture!" << std::endl;
//         throw;
//     }
// }

// void
// DLFrame::resetAnchor()
// {
//    	if (_mUseTexture)
//         _mTex.resetAnchor();
//     else {
//         std::cout << "DLFrame - tried to set a texture property but you're not using a texture!" << std::endl;
//         throw;
//     }
// }

// void
// DLFrame::draw(float _x, float _y, float _w, float _h)
// {
// 	if (_mUseTexture){
// 		_mTex.draw(_x, _y, _w, _h);
// 	} else {
//         std::cout << "DLFrame - tried to set a texture property but you're not using a texture!" << std::endl;
//         throw;
//     }
// }

// void
// DLFrame::draw(float _x, float _y)
// {
// 	draw(_x, _y, (float)width, (float)height);
// }
