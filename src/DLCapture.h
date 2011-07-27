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

#include "boost/circular_buffer.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/threadpool.hpp"
#include "DeckLinkAPI_h.h"
#include "DLFrame.h"
#include "DLFrameQueue.hpp"

class DLCapture : public IDeckLinkInputCallback
{

public:

    DLCapture ();
    ~DLCapture ();

    float                               getFrameRate(void);
    long                                getFrameCount(void);
    bool                                getFrame(boost::shared_ptr<DLFrame> &frame);
    void                                setSize(int width, int height);
    unsigned int                        getWidth(void);
    unsigned int                        getHeight(void);
    unsigned int                        getCaptureWidth(void);
    unsigned int                        getCaptureHeight(void);
    unsigned int                        getThreadpoolSize(void);
    void                                setThreadpoolSize(unsigned int size);
    
    // callback interfaces
    virtual ULONG STDMETHODCALLTYPE     AddRef(void);
    virtual HRESULT STDMETHODCALLTYPE   QueryInterface(REFIID iid, LPVOID *ppv);
    virtual ULONG STDMETHODCALLTYPE     Release(void);
    virtual HRESULT STDMETHODCALLTYPE   VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents,
                                                                IDeckLinkDisplayMode* newDisplayMode,
                                                                BMDDetectedVideoInputFormatFlags detectedSignalFlags);
    virtual HRESULT STDMETHODCALLTYPE   VideoInputFrameArrived(IDeckLinkVideoInputFrame* pArrivedFrame,
                                                               IDeckLinkAudioInputPacket*);
    

private:
    BYTE                                Clamp(int value);
    void                                InitialiseDimensions(IDeckLinkVideoInputFrame* pArrivedFrame);
    void                                PostProcess(IDeckLinkVideoInputFrame* pArrivedFrame);
    boost::shared_ptr<DLFrame>          Resize(boost::shared_ptr<DLFrame> src, int targetWidth, int targetHeight);
    boost::shared_ptr<DLFrame>          YuvToGrayscale(IDeckLinkVideoInputFrame* pArrivedFrame);
    boost::shared_ptr<DLFrame>          YuvToRgb(IDeckLinkVideoInputFrame* pArrivedFrame);
    void                                YuvToRgbChunk(BYTE *yuv, boost::shared_ptr<DLFrame> rgb, unsigned int offset, unsigned int chunk_size);
    void                                CreateLookupTables(void);
    
    DLFrameQueue                        fifo;                   // producer/consumer queue to hold captured frames
    boost::circular_buffer<float>       mFramerateTimestamps;   // buffer to calculate current framerate
    boost::mutex                        mFramerateMutex;        // mutex to protect mFramerateTimestamps buffer access

    unsigned int                        mRefCount;

    long                                mFrameCount;            // number of frames we've captured
    long                                mCaptureWidth;          // width of the raw captured frame
    long                                mCaptureHeight;         // height of the raw captured frame
	long                                mCaptureRowBytes;
    long                                mCaptureTotalBytes;     // 
    long                                mGrayscaleTotalBytes;
    long                                mRgbRowBytes;
    unsigned int                        mWidth;                 // width after any resizing
    unsigned int                        mHeight;                // height after any resizing
    
    bool                                mDimensionsInitialized;
    unsigned int                        mFramerateNumFrames;
    float                               mFramerateElapsedTime;
    
    BYTE                                red[256][256];
    BYTE                                blue[256][256];
    BYTE                                green[256][256][256];
    
    boost::threadpool::pool             conversion_workers;
    long                                mConversionChunkSize;
    long                                mConversionChunkSizeLeftover;
};
