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
    
    boost::threadpool::pool             conversion_workers;

};
