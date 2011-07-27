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

#include "DLCapture.h"
#include <iostream>
#include "cv.h"
#include "boost/thread.hpp"

using namespace boost;

DLCapture::DLCapture() : mRefCount(1),
                         mFrameCount(0),
                         mDimensionsInitialized(false),
                         mWidth(-1),
                         mHeight(-1),
                         mFramerateTimestamps(60)
{
    // generate the YUV lookup tables and store them in memory
	CreateLookupTables();

	// figure out how much concurrency we have on this box
	// note: it's possible for hardware_concurrency to return 0.
    // subtract 1 for the frame capture thread, and 1 for our host app
    int pool_size = max(thread::hardware_concurrency() - 2, 2);
	
	// size our threadpool appropriately
    setThreadpoolSize(pool_size);
}

DLCapture::~DLCapture()
{
}


HRESULT STDMETHODCALLTYPE
DLCapture::QueryInterface(REFIID iid, LPVOID *ppv)
{
    HRESULT result = E_NOINTERFACE;
    
    // Initialise the return result
    *ppv = NULL;
    
    // Obtain the IUnknown interface and compare it the provided REFIID
    if (iid == IID_IUnknown)
    {
        *ppv = this;
        AddRef();
        result = S_OK;
    }
    else if (iid == IID_IDeckLinkInputCallback)
    {
        *ppv = (IDeckLinkInputCallback*)this;
        AddRef();
        result = S_OK;
    }
    
    return result;
}

ULONG STDMETHODCALLTYPE
DLCapture::AddRef(void)
{
    return InterlockedIncrement((LONG*)&mRefCount);
}

ULONG STDMETHODCALLTYPE
DLCapture::Release(void)
{
    int newRefValue;
    
    newRefValue = InterlockedDecrement((LONG*)&mRefCount);
    if (newRefValue == 0)
    {
        delete this;
        return 0;
    }
    
    return newRefValue;
}

HRESULT STDMETHODCALLTYPE
DLCapture::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents,
                                   IDeckLinkDisplayMode* newDisplayMode,
                                   BMDDetectedVideoInputFormatFlags detectedSignalFlags)
{
    // NOTE: we can't set dimensions and stuff here because:
    // a) row bytes and pixel format aren't available
    // b) this doesn't get called on startup

    mDimensionsInitialized = false;
    return S_OK;
}

unsigned int
DLCapture::getWidth(void)
{
    return mWidth;
}

unsigned int
DLCapture::getHeight(void)
{
    return mHeight;
}

void
DLCapture::setSize(int width, int height)
{
    mWidth  = width;
    mHeight = height;
}

unsigned int
DLCapture::getCaptureWidth(void)
{
    return (unsigned int) mCaptureWidth;
}

unsigned int
DLCapture::getCaptureHeight(void)
{
    return (unsigned int) mCaptureHeight;
}

void
DLCapture::InitialiseDimensions(IDeckLinkVideoInputFrame* pArrivedFrame)
{
    mCaptureWidth      = pArrivedFrame->GetWidth();
    mCaptureHeight     = pArrivedFrame->GetHeight();
    mCaptureRowBytes   = pArrivedFrame->GetRowBytes();
    mCaptureTotalBytes = mCaptureRowBytes * mCaptureHeight;

	// bend over backwards to send memory aligned work units to each thread
	// NOTE: I'm sure there's a more elegant way to split up this 
	//       work to make it memory aligned, but I'm not smart enough
	//       to figure it out today...
    mConversionChunkSize = mCaptureRowBytes * (long)ceil(mCaptureHeight / (float)getThreadpoolSize());
	// calculate whether the last chunk will be memory aligned or not and set its size
	if(mCaptureHeight % getThreadpoolSize() == 0)
		mConversionChunkSizeLeftover = mConversionChunkSize;
	else
		mConversionChunkSizeLeftover = mCaptureTotalBytes % mConversionChunkSize;

	// precalculate some stuff
	mGrayscaleTotalBytes = mCaptureWidth * mCaptureHeight;
    mRgbRowBytes         = mCaptureWidth * 3;
}

bool
DLCapture::getFrame(shared_ptr<DLFrame> &frame)
{
	return fifo.Consume(frame);
}

long
DLCapture::getFrameCount(void)
{
    return mFrameCount;
}

float
DLCapture::getFrameRate(void)
{
    mutex::scoped_lock l(mFramerateMutex);
	if(mFramerateTimestamps.size() < 2)
		return 0.0f;
	else
		return  mFramerateTimestamps.size() / (mFramerateTimestamps.back() - mFramerateTimestamps.front());
}

unsigned int
DLCapture::getThreadpoolSize(void)
{
    return (unsigned int) conversion_workers.size();
}

void
DLCapture::setThreadpoolSize(unsigned int size)
{
    if(size < 1) size = 1;
    conversion_workers.size_controller().resize(size);
}
    
// TODO: take care of the fact that frames might get out of order? There's
//       no guarantee that threads will process these suckers in order, we'd
//       need to keep track of the frame number in DLFrame and make
//       the grabFrame() method smart enought to enforce ordering -- but how
//       do we do that with the potential for droppped frames?
void
DLCapture::PostProcess(IDeckLinkVideoInputFrame* pArrivedFrame)
{
    // post-process the preview frame
    // if(mCaptureHeight == mHeight || mCaptureWidth == mWidth){
    //     fifo.Produce(YuvToGrayscale(pArrivedFrame));
    // } else {
    //     fifo.Produce(Resize(YuvToGrayscale(pArrivedFrame), mWidth, mHeight));
    // }

    if(mCaptureHeight == mHeight || mCaptureWidth == mWidth){
        fifo.Produce(YuvToRgb(pArrivedFrame));
    } else {
       fifo.Produce(Resize(YuvToRgb(pArrivedFrame), mWidth, mHeight));
    }

    // free up the frame reference
    pArrivedFrame->Release();
}

shared_ptr<DLFrame>
DLCapture::YuvToGrayscale(IDeckLinkVideoInputFrame* pArrivedFrame)
{
    // TODO: don't assum YUV here
    BYTE* yuv;
    pArrivedFrame->GetBytes((void**)&yuv);

    shared_ptr<DLFrame> grayscale(new DLFrame(mCaptureWidth, mCaptureHeight, mCaptureWidth, DLFrame::DL_GRAYSCALE));
    
    // simple YUV -> Grayscale, just throw away the U and V channels
    for(int i=0; i<mGrayscaleTotalBytes; i++)
        grayscale->pixels[i] = yuv[(i*2)+1];

    return grayscale;
}

// YUV format conforms to ITU.BT-601
//
// http://www.fourcc.org/yuv.php#UYVY
// http://www.martinreddy.net/gfx/faqs/colorconv.faq
// http://developer.apple.com/quicktime/icefloe/dispatch027.html
// http://www.fourcc.org/source/YUV420P-OpenGL-GLSLang.c
//
//
// Check out bit layout for "bmdFormat8BitYUV : UYVY 4:2:2 Representation"
// on page 204 of the Decklink SDK documentation
//
// Below formulas from Color Space Conversions on pg 227 of the Decklink SDK documentation
// R = 1.164(Y - 16) + 1.793(Cr - 128)
// G = 1.164(Y - 16) - 0.534(Cr - 128) - 0.213(Cb - 128)
// B = 1.164(Y - 16) + 2.115(Cb - 128)

shared_ptr<DLFrame>
DLCapture::YuvToRgb(IDeckLinkVideoInputFrame* pArrivedFrame)
{
    BYTE* yuv;
    pArrivedFrame->GetBytes((void**)&yuv);

    // allocate space for the rgb image
    shared_ptr<DLFrame> rgb(new DLFrame(mCaptureWidth, mCaptureHeight, mRgbRowBytes, DLFrame::DL_RGB));

    int num_workers = conversion_workers.size() - 1;

    // split up the image into memory-aligned chunks so they take advantage of
    // the CPU cache
	for(int i=0; i<num_workers; i++) {
        conversion_workers.schedule(bind(&DLCapture::YuvToRgbChunk,
                                         this,
                                         yuv,
                                         rgb,
                                         mConversionChunkSize*i,
                                         mConversionChunkSize));
	}

    // get the off-sized leftover chunk and schedule it
    conversion_workers.schedule(bind(&DLCapture::YuvToRgbChunk,
                                     this,
                                     yuv,
                                     rgb,
                                     mConversionChunkSize*num_workers,
                                     mConversionChunkSizeLeftover));

    conversion_workers.wait();

    return rgb;
}

void 
DLCapture::YuvToRgbChunk(BYTE *yuv, shared_ptr<DLFrame> rgb, unsigned int offset, unsigned int chunk_size)
{
    // convert 4 YUV macropixels to 6 RGB pixels
	unsigned int i, j;
    unsigned int boundry = offset + chunk_size;
    unsigned char y, u, v;
    
    for(i=offset, j=(offset/4)*6; i<boundry; i+=4, j+=6){
        y = yuv[i+1];
        u = yuv[i];
        v = yuv[i+2];

        rgb->pixels[j]   = red[y][v];
        rgb->pixels[j+1] = green[y][u][v];
        rgb->pixels[j+2] = blue[y][u];

        y = yuv[i+3];

        rgb->pixels[j+3] = red[y][v];       
        rgb->pixels[j+4] = green[y][u][v];
        rgb->pixels[j+5] = blue[y][u];
    }

    /*
    // fixed point math implementation - superceded by the lookup table method
	unsigned int i, j;
    unsigned int boundry = offset + chunk_size;
	int yy, uu, vv, ug_plus_vg, ub, vr;
	int r,g,b;
    for(i=offset, j=(offset/4)*6; i<boundry; i+=4, j+=6){
		yy = yuv[i+1] << 8;
		uu = yuv[i] - 128;
		vv = yuv[i+2] - 128;
		ug_plus_vg = uu * 88 + vv * 183;
		ub = uu * 454;
		vr = vv * 359;
		r = (yy + vr) >> 8;
		g = (yy - ug_plus_vg) >> 8;
		b = (yy + ub) >> 8;
		rgb->pixels[j]   = r < 0 ? 0 : (r > 255 ? 255 : (unsigned char)r);
		rgb->pixels[j+1] = g < 0 ? 0 : (g > 255 ? 255 : (unsigned char)g);
		rgb->pixels[j+2] = b < 0 ? 0 : (b > 255 ? 255 : (unsigned char)b);
		yy = yuv[i+3] << 8;
		r = (yy + vr) >> 8;
		g = (yy - ug_plus_vg) >> 8;
		b = (yy + ub) >> 8;
		rgb->pixels[j+3] = r < 0 ? 0 : (r > 255 ? 255 : (unsigned char)r);
		rgb->pixels[j+4] = g < 0 ? 0 : (g > 255 ? 255 : (unsigned char)g);
		rgb->pixels[j+5] = b < 0 ? 0 : (b > 255 ? 255 : (unsigned char)b);
    }
    */
}

// clamp values between 0 and 255
inline BYTE
DLCapture::Clamp(int value)
{
    if(value > 255) return 255;
    if(value < 0)   return 0;
    return value;
}

// tables are done for all possible values 0 - 255 of yuv
// rather than just "legal" values of yuv.
// two dimensional arrays for red &  blue, three dimensions for green
void
DLCapture::CreateLookupTables(void){

    int yy, uu, vv, ug_plus_vg, ub, vr, val;

    // Red
    for (int y = 0; y < 256; y++) {
        for (int v = 0; v < 256; v++) {
            yy         = y << 8;
            vv         = v - 128;
            vr         = vv * 359;
            val        = (yy + vr) >>  8;
            red[y][v]  = Clamp(val);
        }
    }

    // Blue
    for (int y = 0; y < 256; y++) {
        for (int u = 0; u < 256; u++) {
            yy          = y << 8;
            uu          = u - 128;
            ub          = uu * 454;
            val         = (yy + ub) >> 8;
            blue[y][u]  = Clamp(val);
        }
    }

    // Green
    for (int y = 0; y < 256; y++) {
        for (int u = 0; u < 256; u++) {
            for (int v = 0; v < 256; v++) {
                yy              = y << 8;
                uu              = u - 128;
                vv              = v - 128;
                ug_plus_vg      = uu * 88 + vv * 183;
                val             = (yy - ug_plus_vg) >> 8;
                green[y][u][v]  = Clamp(val);
            }
        }
    }
}

shared_ptr<DLFrame>
DLCapture::Resize(shared_ptr<DLFrame> src, int targetWidth, int targetHeight)
{
    // check if we need to resize or not
    if(src->height == targetHeight || src->width == targetWidth){
        return src;
    }

    // wrap in a OpenCV matrix
    CvMat src_mat;
    cvInitMatHeader(&src_mat, src->height, src->width, src->getOpenCVType(), src->pixels);

    long row_bytes = (src->getNativeType() == DLFrame::DL_RGB) ? targetWidth*3 : targetWidth;
    // allocate space for the return image
    shared_ptr<DLFrame> resized(new DLFrame(targetWidth, targetHeight, row_bytes, src->getNativeType()));

    // wrap return image in a OpenCV matrix
    CvMat dest_mat;
    cvInitMatHeader(&dest_mat, resized->height, resized->width, src->getOpenCVType(), resized->pixels);

    // resize
    cvResize(&src_mat, &dest_mat, CV_INTER_AREA);

    return resized;
 
}

HRESULT STDMETHODCALLTYPE
DLCapture::VideoInputFrameArrived(IDeckLinkVideoInputFrame* pArrivedFrame, IDeckLinkAudioInputPacket*)
{
    {
        mutex::scoped_lock l(mFramerateMutex);
        mFramerateTimestamps.push_back(ofGetElapsedTimef());
    }
    mFrameCount++;

    // precompute some junk
    if(mDimensionsInitialized == false) {
        InitialiseDimensions(pArrivedFrame);
    }
    
    // don't post process this frame if we're overwhelming the preview buffer
//    if(10 > mFrames.size()){
        // increase the refcount since we don't know when the thread will execute
        pArrivedFrame->AddRef();
        // push it to the thread pool for background processing
        //capture_workers.schedule(bind(&DLCapture::PostProcess, this, pArrivedFrame));  
		PostProcess(pArrivedFrame);
	//} else {
	//	cout << "Dropped frame" << endl;
	//}

	return S_OK;
}



// BMDTimeValue frameTime, frameDuration;
// int              hours, minutes, seconds, frames;
// HRESULT          theResult = S_OK;
// long width;
// long height;
// long row_bytes;
// BMDPixelFormat pixel_format;
// BMDFrameFlags flags;
// BYTE* pbDestData = NULL;

// pArrivedFrame->GetBytes((void**)&pbDestData);
// pArrivedFrame->GetStreamTime(&frameTime, &frameDuration, 600);

// width         = pArrivedFrame->GetWidth();
// height        = pArrivedFrame->GetHeight();
// row_bytes     = pArrivedFrame->GetRowBytes();
// pixel_format  = pArrivedFrame->GetPixelFormat();
// flags         = pArrivedFrame->GetFlags();

// hours = (int)(frameTime / (600 * 60*60));
// minutes = (int)((frameTime / (600 * 60)) % 60);
// seconds = (int)((frameTime / 600) % 60);
// frames = (int)((frameTime / 6) % 100);
