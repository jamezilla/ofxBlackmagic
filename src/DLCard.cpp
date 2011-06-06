#include "DLCard.h"
#include <comutil.h>
#include <iostream>
#include <iomanip>
#include <exception>
#include "boost/thread/thread.hpp"
#include "boost/bind.hpp"

#define COLUMN_WIDTH 35

using namespace std;

// List of known pixel formats and their matching display names
const BMDPixelFormat	DLCard::gKnownPixelFormats[]		= {bmdFormat8BitYUV, bmdFormat10BitYUV, bmdFormat8BitARGB, bmdFormat8BitBGRA, bmdFormat10BitRGB, (BMDPixelFormat)0};
const char *			DLCard::gKnownPixelFormatNames[]	= {"8-bit YUV", "10-bit YUV", "8-bit ARGB", "8-bit BGRA", "10-bit RGB", NULL};

DLCard::DLCard(IDeckLink* deckLink)
{
    m_pDeckLink = deckLink;
    // increase the ref count so we don't lose it
	deckLink->AddRef();

    // we're not running yet.. hopefully
    m_bRunning = false;

    // Obtain the input and output interfaces
	if (m_pDeckLink->QueryInterface(IID_IDeckLinkInput, (void**)&m_pInputCard) != S_OK)
		throw;

	//	m_pOutputCard = NULL;

    // lowest common denominator? should be
    // set to something real by the user
    m_tDisplayMode = bmdModeHD1080i5994;
    m_tPixelFormat = bmdFormat8BitYUV;

	m_pDelegate = new DLCapture();
}

DLCard::~DLCard()
{
    // Release the IDeckLink instance when we've finished with it to prevent leaks
    m_pDeckLink->Release();
}

void DLCard::print_name(void)
{
	HRESULT result;
    BSTR    deviceNameBSTR = NULL;

    result = m_pDeckLink->GetModelName(&deviceNameBSTR);
    if (result == S_OK)
    {
        _bstr_t		deviceName(deviceNameBSTR, false);
		cout << "Decklink " << (char*) deviceName << endl;
    }
}

void DLCard::print_attributes(void)
{
	IDeckLinkAttributes*				deckLinkAttributes = NULL;
	BSTR								name = NULL;
	BOOL								supported;
	HRESULT								result;
	LONGLONG							count;

	// Query the DeckLink for its attributes interface
	result = m_pDeckLink->QueryInterface(IID_IDeckLinkAttributes, (void**)&deckLinkAttributes);
	if (result != S_OK)
	{
		cout << "Could not obtain the IDeckLinkAttributes interface - result = " << result << endl;
		goto bail;
	}

	// List attributes and their value
	cout << endl << "ATTRIBUTES" << endl;
    cout << "----------------------------------------------------------------------" << endl;
	result = deckLinkAttributes->GetFlag(BMDDeckLinkHasSerialPort, &supported);
	if (result == S_OK)
	{
		cout << setw(COLUMN_WIDTH) << left << "Serial port present:" << ((supported == TRUE) ? "Yes" : "No") << endl;
		
		if(supported == TRUE)
		{
			result = deckLinkAttributes->GetString(BMDDeckLinkSerialPortDeviceName, &name);
			if (result == S_OK)
			{
				_bstr_t		portName(name, false);
				cout << setw(COLUMN_WIDTH) << left << "Serial port name:" << (char *) portName << endl;
			}
			else
			{
				cout << "Could not query the serial port name attribute- result = " << result << endl;
			}	
		}
	}
	else
	{
		cout << "Could not query the serial port presence attribute- result = " << result << endl;
	}

	result = deckLinkAttributes->GetInt(BMDDeckLinkNumberOfSubDevices, &count);
	if (result == S_OK)
	{
		cout << setw(COLUMN_WIDTH) << left << "Number of sub-devices:" << count << endl;
		if (count != 0)
		{
			result = deckLinkAttributes->GetInt(BMDDeckLinkSubDeviceIndex, &count);
			if (result == S_OK)
			{
				cout << setw(COLUMN_WIDTH) << left << "Sub-device index:" << count << endl;
			}
			else
			{
				cout << "Could not query the sub-device index attribute- result = " << result << endl;
			}
		}
	}
	else
	{
		cout << "Could not query the number of sub-device attribute- result = " << result << endl;
	}

	result = deckLinkAttributes->GetInt(BMDDeckLinkMaximumAudioChannels, &count);
	if (result == S_OK)
	{
		cout << setw(COLUMN_WIDTH) << left << "Maximum number of audio channels:" << count << endl;
	}
	else
	{
		cout << "Could not query the internal keying attribute- result = " << result << endl;
	}

	result = deckLinkAttributes->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &supported);
	if (result == S_OK)
	{
		cout << setw(COLUMN_WIDTH) << left << "Input mode detection supported:" << ((supported == TRUE) ? "Yes" : "No")  << endl;
	}
	else
	{
		cout << "Could not query the input mode detection attribute- result = " << result << endl;
	}

	result = deckLinkAttributes->GetFlag(BMDDeckLinkSupportsInternalKeying, &supported);
	if (result == S_OK)
	{
		cout << setw(COLUMN_WIDTH) << left << "Internal keying supported:" << ((supported == TRUE) ? "Yes" : "No") << endl;
	}
	else
	{
		cout << "Could not query the internal keying attribute- result = " << result << endl;
	}

	result = deckLinkAttributes->GetFlag(BMDDeckLinkSupportsExternalKeying, &supported);
	if (result == S_OK)
	{
		cout << setw(COLUMN_WIDTH) << left << "External keying supported:" << ((supported == TRUE) ? "Yes" : "No") << endl;
	}
	else
	{
		cout << "Could not query the external keying attribute- result = " << result << endl;
	}

	result = deckLinkAttributes->GetFlag(BMDDeckLinkSupportsHDKeying, &supported);
	if (result == S_OK)
	{
		cout << setw(COLUMN_WIDTH) << left << "HD-mode keying supported:" << ((supported == TRUE) ? "Yes" : "No") << endl;
	}
	else
	{
		cout << "Could not query the HD-mode keying attribute- result = " << result << endl;
	}

bail:
	cout << endl;
}

void DLCard::print_output_modes(void)
{
	IDeckLinkInput*					    deckLinkInput = NULL;
	IDeckLinkDisplayModeIterator*		displayModeIterator = NULL;
	IDeckLinkDisplayMode*				displayMode = NULL;
	HRESULT								result;	
	
	// Query the DeckLink for its configuration interface
	result = m_pDeckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput);
	if (result != S_OK)
	{
		cout << "Could not obtain the IDeckLinkInput interface - result = " << result << endl;
		goto bail;
	}
	
	// Obtain an IDeckLinkDisplayModeIterator to enumerate the display modes supported on output
	result = deckLinkInput->GetDisplayModeIterator(&displayModeIterator);
	if (result != S_OK)
	{
		cout << "Could not obtain the video input display mode iterator - result = " << result << endl;
		goto bail;
	}
	
	// List all supported output display modes
	cout << endl << "INPUT DISPLAY MODES" << endl;
	cout << setw(20) << left << "Name"
		 << setw(7)  << left << "Width" 
		 << setw(7)  << left << "Height"
		 << setw(11)  << left << "Framerate"
		 << "Pixel Formats" << endl;
	cout << "----------------------------------------------------------------------" << endl;
	while (displayModeIterator->Next(&displayMode) == S_OK)
	{
		BSTR			displayModeBSTR = NULL;
		
		result = displayMode->GetName(&displayModeBSTR);
		if (result == S_OK)
		{
			_bstr_t					modeName(displayModeBSTR, false);
			int						modeWidth;
			int						modeHeight;
			BMDTimeValue			frameRateDuration;
			BMDTimeScale			frameRateScale;
			int						pixelFormatIndex = 0; // index into the gKnownPixelFormats / gKnownFormatNames arrays
			BMDDisplayModeSupport	displayModeSupport;
			
			// Obtain the display mode's properties
			modeWidth = displayMode->GetWidth();
			modeHeight = displayMode->GetHeight();
			displayMode->GetFrameRate(&frameRateDuration, &frameRateScale);
			cout << setw(20) << left << (char*)modeName 
				 << setw(7)  << left << modeWidth 
				 << setw(7)  << left << modeHeight
				 << setw(11) << left << ((double)frameRateScale/(double)frameRateDuration);

			// Print the supported pixel formats for this display mode
			while ((gKnownPixelFormats[pixelFormatIndex] != 0) && (gKnownPixelFormatNames[pixelFormatIndex] != NULL))
			{
				if ((deckLinkInput->DoesSupportVideoMode(displayMode->GetDisplayMode(), gKnownPixelFormats[pixelFormatIndex], bmdVideoInputFlagDefault, &displayModeSupport, NULL) == S_OK)
					&& (displayModeSupport != bmdDisplayModeNotSupported))					
				{
					cout << gKnownPixelFormatNames[pixelFormatIndex] << "\t";					
				}
				pixelFormatIndex++;
			}
			
			cout << endl;
		}
		
		// Release the IDeckLinkDisplayMode object to prevent a leak
		displayMode->Release();
	}
	
	cout << endl;
	
bail:
	// Ensure that the interfaces we obtained are released to prevent a memory leak
	if (displayModeIterator != NULL)
		displayModeIterator->Release();
	
	if (deckLinkInput != NULL)
		deckLinkInput->Release();
}


void DLCard::print_capabilities(void)
{
	IDeckLinkAttributes*		deckLinkAttributes = NULL;
	LONGLONG					ports;
	int							itemCount;
	HRESULT						result;	
	
	// Query the DeckLink for its configuration interface
	result = m_pDeckLink->QueryInterface(IID_IDeckLinkAttributes, (void**)&deckLinkAttributes);
	if (result != S_OK)
	{
		cout << "Could not obtain the IDeckLinkAttributes interface - result = " << result << endl;
		goto bail;
	}
		
	cout << "Supported video output connections:" << endl;
	itemCount = 0;
	result = deckLinkAttributes->GetInt(BMDDeckLinkVideoOutputConnections, &ports);
	if (result == S_OK)
	{
		if (ports & bmdVideoConnectionSDI)
		{
			itemCount++;
			cout << "SDI";
		}
		
		if (ports & bmdVideoConnectionHDMI)
		{
			if (itemCount++ > 0)
				cout << ", ";
			cout << "HDMI";
		}
		
		if (ports & bmdVideoConnectionOpticalSDI)
		{
			if (itemCount++ > 0)
				cout << ", ";
			cout << "Optical SDI";
		}
		
		if (ports & bmdVideoConnectionComponent)
		{
			if (itemCount++ > 0)
				cout << ", ";
			cout << "Component";
		}
		
		if (ports & bmdVideoConnectionComposite)
		{
			if (itemCount++ > 0)
				cout << ", ";
			cout << "Composite";
		}
		
		if (ports & bmdVideoConnectionSVideo)
		{
			if (itemCount++ > 0)
				cout << ", ";
			cout << "S-Video";
		}
	}
	else
	{
		cout << "Could not obtain the list of output ports - result = " << result << endl;
		goto bail;
	}
	
	cout << endl << endl;
	
	cout << "Supported video input connections:" << endl;
	itemCount = 0;
	result = deckLinkAttributes->GetInt(BMDDeckLinkVideoInputConnections, &ports);
	if (result == S_OK)
	{
		if (ports & bmdVideoConnectionSDI)
		{
			itemCount++;
			cout << "SDI";
		}
		
		if (ports & bmdVideoConnectionHDMI)
		{
			if (itemCount++ > 0)
				cout << ", ";
			cout << "HDMI";
		}
		
		if (ports & bmdVideoConnectionOpticalSDI)
		{
			if (itemCount++ > 0)
				cout << ", ";
			cout << "Optical SDI";
		}
		
		if (ports & bmdVideoConnectionComponent)
		{
			if (itemCount++ > 0)
				cout << ", ";
			cout << "Component";
		}
		
		if (ports & bmdVideoConnectionComposite)
		{
			if (itemCount++ > 0)
				cout << ", ";
			cout << "Composite";
		}
		
		if (ports & bmdVideoConnectionSVideo)
		{
			if (itemCount++ > 0)
				cout << ", ";
			cout << "S-Video";
		}
	}
	else
	{
		cout << "Could not obtain the list of input ports - result = " << result << endl;
		goto bail;
	}	
	cout << endl;
	
bail:
	if (deckLinkAttributes != NULL)
		deckLinkAttributes->Release();
}

// Video output mode supported results:
//  bmdDisplayModeNotSupported			Display mode is not supported
//  bmdDisplayModeSupported				Display mode is supported natively
//  bmdDisplayModeSupportedWithConversion Display mode is supported with conversion
//
// BMDVideoInputFlags:
//  bmdVideoInputFlagDefault				No other flags applicable
//  bmdVideoInputEnableFormatDetection		Enable video input mode detection. (See IDeckLinkInputCallback::VideoInputFormatChanged for details)
bool DLCard::setDisplayMode(BMDDisplayMode displayMode)
{
	// check to see if the requested display mode is supported
    BMDDisplayModeSupport	displayModeSupport;
	IDeckLinkDisplayMode*   newDisplayMode = NULL;
    m_pInputCard->DoesSupportVideoMode(displayMode,
                                       m_tPixelFormat,
                                       bmdVideoInputFlagDefault,
                                       &displayModeSupport,
                                       &newDisplayMode);

    if(displayModeSupport != bmdDisplayModeSupported)
        return false;

	// ok, actually set the display mode
	m_tDisplayMode = displayMode;

    BSTR displayModeBSTR = NULL;
	HRESULT result = newDisplayMode->GetName(&displayModeBSTR);
    if (result == S_OK)
    {
        _bstr_t					modeName(displayModeBSTR, false);
        int						modeWidth;
        int						modeHeight;
        BMDTimeValue			frameRateDuration;
        BMDTimeScale			frameRateScale;
			
        // Obtain the display mode's properties
        modeWidth = newDisplayMode->GetWidth();
        modeHeight = newDisplayMode->GetHeight();
        newDisplayMode->GetFrameRate(&frameRateDuration, &frameRateScale);

        // set the callback's display size here so it is something valid
        m_pDelegate->setPreviewSize(modeWidth, modeHeight);

        cout << endl;
        cout << "INPUT DISPLAY MODE SETTINGS" << endl;
        cout << "---------------------------" << endl;
        cout << setw(11) << left << "Name:" << (char*)modeName << endl;
        cout << setw(11) << left << "Width:" << modeWidth << endl;
        cout << setw(11) << left << "Height:" << modeHeight << endl;
        cout << setw(11) << left << "Framerate:" << ((double)frameRateScale/(double)frameRateDuration) << endl;
        cout << endl;
    }
		
    // Release the IDeckLinkDisplayMode object to prevent a leak
    newDisplayMode->Release();

	return true;
}

void DLCard::initGrabber(BMDDisplayMode displayMode, bool bTexture)
{
    if (m_bRunning) {
        cout << "initGrabber - can't change settings while running" << endl;
        return;
    }

    if(!setDisplayMode(displayMode)) {
        cout << "Couldn't set display mode... bailing" << endl;
        return;
    }

    boost::thread pp(boost::bind(&DLCard::runThreadedCapture, this));
}

void DLCard::runThreadedCapture(void)
{
	HRESULT result;

    // Turn on video input
    result = m_pInputCard->SetCallback(m_pDelegate);
    if (result != S_OK)
        cout << "SetDelegate failed with result " << result << endl;
        

    result = m_pInputCard->EnableVideoInput(m_tDisplayMode, m_tPixelFormat, 0);
    if (result != S_OK) {
        cout << "EnableVideoInput failed with result " << result << endl;
        return;
    }
		
    // Sart the input stream running
    result = m_pInputCard->StartStreams();
    if (result != S_OK) {
        cout << "Input StartStreams failed with result " << result << endl;
        return;
    }
    
    m_bRunning = TRUE;

    while(m_bRunning) { /* loop */ }
}

bool DLCard::running(void)
{
    return m_bRunning;
}

void DLCard::close()
{
	m_bRunning = FALSE;
	m_pInputCard->StopStreams();
	//m_pOutputCard->DisableVideoOutput();
	m_pInputCard->DisableVideoInput();
}
