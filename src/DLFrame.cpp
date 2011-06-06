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
