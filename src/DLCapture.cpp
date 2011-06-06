#include "DLCapture.h"
#include <iostream>
#include "cv.h"

using namespace boost;

DLCapture::DLCapture()
{
    mNumCores = thread::hardware_concurrency();
    mRefCount = 1;
    mFrameCount = 0;
    mDimensionsInitialized = false;
    mDropNumFrames = 0;
    mPreviewWidth = -1;
    mPreviewHeight = -1;
    capture_workers.size_controller().resize(3);
    conversion_workers.size_controller().resize(mNumCores);
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

void
DLCapture::InitialiseDimensions(IDeckLinkVideoInputFrame* pArrivedFrame)
{
    mRawWidth             = pArrivedFrame->GetWidth();
    mRawHeight            = pArrivedFrame->GetHeight();
    mRawRowBytes          = pArrivedFrame->GetRowBytes();
    //mPixelFormat          = pArrivedFrame->GetPixelFormat();
	// TODO: change these so they're settable?
	mCaptureWidth         = mRawWidth;
	mCaptureHeight        = mRawHeight;

    mRawTotalBytes        = mRawRowBytes  * mRawHeight;
    mGrayscaleTotalBytes  = mRawWidth     * mRawHeight;
    mRgbTotalBytes        = mRawWidth * 3 * mRawHeight;
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
    // if(mRawHeight == mPreviewHeight || mRawWidth == mPreviewWidth){
    //     fifo.Produce(YuvToGrayscale(pArrivedFrame));
    // } else {
    //     fifo.Produce(Resize(YuvToGrayscale(pArrivedFrame), mPreviewWidth, mPreviewHeight));
    // }

    if(mRawHeight == mPreviewHeight || mRawWidth == mPreviewWidth){
        fifo.Produce(YuvToRgb(pArrivedFrame));
    } else {
       fifo.Produce(Resize(YuvToRgb(pArrivedFrame), mPreviewWidth, mPreviewHeight));
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

    shared_ptr<DLFrame> grayscale(new DLFrame(mRawWidth, mRawHeight, mRawWidth, DLFrame::DL_GRAYSCALE));
    
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
// Check out bit layout for "bmdFormat8BitYUV : ‘UYVY’ 4:2:2 Representation"
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
    boost::shared_ptr<DLFrame> rgb(new DLFrame(mRawWidth, mRawHeight, mRawWidth*3, DLFrame::DL_RGB));

    unsigned int chunk_size = mRawTotalBytes/mNumCores;

    for(int i=0; i<mNumCores; i++)
        conversion_workers.schedule(bind(&DLCapture::YuvToRgbChunk, this, yuv, rgb, chunk_size*i, chunk_size));

    conversion_workers.wait();

    return rgb;
}

void 
DLCapture::YuvToRgbChunk(BYTE *yuv, boost::shared_ptr<DLFrame> rgb, unsigned int offset, unsigned int chunk_size)
{
    // convert 4 YUV macropixels to 6 RGB pixels
    BYTE u, y1, v, y2;
    // unsigned int j = 0;
	unsigned int i, j;
    unsigned int boundry = offset + chunk_size;
    double val = 0;
    for(i=offset, j=(offset/4)*6; i<boundry; i+=4, j+=6){
        u = yuv[i];
        y1  = yuv[i+1];
        v = yuv[i+2];
        y2  = yuv[i+3];

        // bytes are reversed BGR
        rgb->pixels[j]   = Clamp(1.164 * (y1 - 16) + 2.115 * (v - 128));
        rgb->pixels[j+1] = Clamp(1.164 * (y1 - 16) - 0.534 * (u - 128) - 0.213 * (v - 128));
        rgb->pixels[j+2] = Clamp(1.164 * (y1 - 16) + 1.793 * (u - 128));

        rgb->pixels[j+3] = Clamp(1.164 * (y2 - 16) + 2.115 * (v - 128));
        rgb->pixels[j+4] = Clamp(1.164 * (y2 - 16) - 0.534 * (u - 128) - 0.213 * (v - 128));
        rgb->pixels[j+5] = Clamp(1.164 * (y2 - 16) + 1.793 * (u - 128));
        // j+=6;
    }
}


unsigned int
DLCapture::Clamp(double value)
{
    if(value > 255.0) return 255;
    if(value < 0.0)   return 0;
    return (unsigned int) value;
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

void
DLCapture::setPreviewSize(int width, int height)
{
    mPreviewWidth  = width;
    mPreviewHeight = height;
}

bool
DLCapture::getPreviewFrame(boost::shared_ptr<DLFrame> &frame)
{
	return fifo.Consume(frame);
}

//bool
//DLCapture::isPreviewQueueEmpty(void)
//{
//	//boost::mutex::scoped_lock l(mPreviewFramesMutex);
//	return mPreviewFrames.empty();
//}

//int 
//DLCapture::getPreviewQueueSize(void)
//{
//	//boost::mutex::scoped_lock l(mPreviewFramesMutex);
//	return mPreviewFrames.size();
//}


HRESULT STDMETHODCALLTYPE
DLCapture::VideoInputFrameArrived(IDeckLinkVideoInputFrame* pArrivedFrame, IDeckLinkAudioInputPacket*)
{
    mFrameCount++;

    // precompute some junk
    if(mDimensionsInitialized == false) {
        InitialiseDimensions(pArrivedFrame);
    }
    
    // don't post process this frame if we're overwhelming the preview buffer
//    if(10 > mPreviewFrames.size()){
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
