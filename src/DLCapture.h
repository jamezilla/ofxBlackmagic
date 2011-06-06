#pragma once

#include "boost/shared_ptr.hpp"
#include "boost/threadpool.hpp"
#include "DeckLinkAPI_h.h"
#include "DLFrame.h"
#include "DLFrameQueue.hpp"

class DLCapture : public IDeckLinkInputCallback
{

public:

	//typedef std::queue<boost::shared_ptr<DLFrame>> DLFrames_t;
	//typedef boost::shared_ptr<DLFrame> DLFrame_pt;

	DLCapture ();
	~DLCapture ();

	virtual HRESULT STDMETHODCALLTYPE	QueryInterface(REFIID iid, LPVOID *ppv);
	virtual ULONG STDMETHODCALLTYPE		AddRef(void);
	virtual ULONG STDMETHODCALLTYPE		Release(void);
	virtual HRESULT STDMETHODCALLTYPE	VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents,
                                                                IDeckLinkDisplayMode* newDisplayMode,
                                                                BMDDetectedVideoInputFormatFlags detectedSignalFlags);
	virtual HRESULT STDMETHODCALLTYPE	VideoInputFrameArrived(IDeckLinkVideoInputFrame* pArrivedFrame,
                                                               IDeckLinkAudioInputPacket*);
    void                                setPreviewSize(int width, int height);
	//DLFrame_pt							getPreviewFrame(void);
	//bool								isPreviewQueueEmpty(void);
	//int									getPreviewQueueSize(void);
	bool								getPreviewFrame(boost::shared_ptr<DLFrame> &frame);


    long            mFrameCount;
	long            mRawWidth;
    long            mRawHeight;
	long            mCaptureWidth;
    long            mCaptureHeight;
	long            mPreviewWidth;
    long            mPreviewHeight;
    

private:

	//boost::lockfree::fifo<void *> fifo;
	DLFrameQueue    fifo;

	/*boost::mutex	mPreviewFramesMutex;
	DLFrames_t		mPreviewFrames;*/

    void            PostProcess(IDeckLinkVideoInputFrame* pArrivedFrame);
    boost::shared_ptr<DLFrame>      YuvToGrayscale(IDeckLinkVideoInputFrame* pArrivedFrame);
    boost::shared_ptr<DLFrame>      YuvToRgb(IDeckLinkVideoInputFrame* pArrivedFrame);
	void                            YuvToRgbChunk(BYTE *yuv, boost::shared_ptr<DLFrame> rgb, unsigned int offset, unsigned int chunk_size);
    boost::shared_ptr<DLFrame>      Resize(boost::shared_ptr<DLFrame> src, int targetWidth, int targetHeight);
    void            InitialiseDimensions(IDeckLinkVideoInputFrame* pArrivedFrame);
	void			CreateLookupTables(void);
    unsigned int    Clamp(double value);

    int             mDropNumFrames;
    int             mDropFramesCount;
	int             mRefCount;
    long            mRawRowBytes;
	long            mRawTotalBytes;
	long            mGrayscaleTotalBytes;
	long            mRgbTotalBytes;
    // BMDPixelFormat  mPixelFormat;
    // BMDFrameFlags   mFlags;
    bool            mDimensionsInitialized;
    int             mNumCores;
	
	double				mYLookup[256];
	double				mU1Lookup[256];
	double				mU2Lookup[256];
	double				mV1Lookup[256];
	double				mV2Lookup[256];

    boost::threadpool::pool capture_workers;
    boost::threadpool::pool conversion_workers;

};
