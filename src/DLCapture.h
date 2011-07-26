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
	bool								getPreviewFrame(boost::shared_ptr<DLFrame> &frame);
    void                                setPreviewSize(int width, int height);

    // callback interfaces
	virtual ULONG STDMETHODCALLTYPE		AddRef(void);
	virtual HRESULT STDMETHODCALLTYPE	QueryInterface(REFIID iid, LPVOID *ppv);
	virtual ULONG STDMETHODCALLTYPE		Release(void);
	virtual HRESULT STDMETHODCALLTYPE	VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents,
                                                                IDeckLinkDisplayMode* newDisplayMode,
                                                                BMDDetectedVideoInputFormatFlags detectedSignalFlags);
	virtual HRESULT STDMETHODCALLTYPE	VideoInputFrameArrived(IDeckLinkVideoInputFrame* pArrivedFrame,
                                                               IDeckLinkAudioInputPacket*);

    long                                mFrameCount;
	long            					mRawWidth;
    long            					mRawHeight;
	long            					mCaptureWidth;
    long            					mCaptureHeight;
	long                                mPreviewWidth;
    long                                mPreviewHeight;
    

private:
    unsigned int                        Clamp(double value);
    void                        		InitialiseDimensions(IDeckLinkVideoInputFrame* pArrivedFrame);
    void                        		PostProcess(IDeckLinkVideoInputFrame* pArrivedFrame);
    boost::shared_ptr<DLFrame>  		Resize(boost::shared_ptr<DLFrame> src, int targetWidth, int targetHeight);
    boost::shared_ptr<DLFrame>  		YuvToGrayscale(IDeckLinkVideoInputFrame* pArrivedFrame);
    boost::shared_ptr<DLFrame>          YuvToRgb(IDeckLinkVideoInputFrame* pArrivedFrame);
    void                                YuvToRgbChunk(BYTE *yuv, boost::shared_ptr<DLFrame> rgb, unsigned int offset, unsigned int chunk_size);
    void                                CreateLookupTables(void);
    
    DLFrameQueue                        fifo;
    boost::mutex                        mFramerateMutex;
    boost::circular_buffer<float>       mFramerateTimestamps;

    int                                 mRefCount;
    long            					mRawRowBytes;
    long            					mRawTotalBytes;
    long            					mGrayscaleTotalBytes;
    long            					mRgbTotalBytes;
    bool                                mDimensionsInitialized;
    int                                 mNumCores;
    unsigned int                        mFramerateNumFrames;
    float                               mFramerateElapsedTime;
    
    unsigned char                       red[256][256];
    unsigned char                       blue[256][256];
    unsigned char                       green[256][256][256];
    
    boost::threadpool::pool             conversion_workers;

};
