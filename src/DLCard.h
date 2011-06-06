#pragma once

#include <cstring>
#include <vector>

#include "DeckLinkAPI_h.h"
#include "DLCapture.h"

  // SD Modes
  //   bmdModeNTSC
  //   bmdModeNTSC2398
  //   bmdModePAL
  //   bmdModeNTSCp
  //   bmdModePALp
  //
  // HD 1080 Modes
  //   bmdModeHD1080p2398
  //   bmdModeHD1080p24
  //   bmdModeHD1080p25
  //   bmdModeHD1080p2997
  //   bmdModeHD1080p30
  //   bmdModeHD1080i50
  //   bmdModeHD1080i5994
  //   bmdModeHD1080i6000
  //   bmdModeHD1080p50
  //   bmdModeHD1080p5994
  //   bmdModeHD1080p6000
  //
  // HD 720 Modes
  //   bmdModeHD720p50
  //   bmdModeHD720p5994
  //   bmdModeHD720p60
  //
  // 2k Modes
  //   bmdMode2k2398
  //   bmdMode2k24
  //   bmdMode2k25

class DLCard
{
public:
	DLCard() {};
    DLCard(IDeckLink* deckLink);
	~DLCard();

    void initGrabber(BMDDisplayMode displayMode, bool bTexture = true);
    bool setDisplayMode(BMDDisplayMode displayMode);
	void close(void);

    void print_name(void);
    void print_attributes(void);
	void print_output_modes(void);
	void print_capabilities(void);
    bool running(void);

    //    std::vector
    DLCapture*     m_pDelegate;

private:
    void runThreadedCapture(void);

	// List of known pixel formats and their matching display names
	static const BMDPixelFormat	gKnownPixelFormats[];
	static const char *			gKnownPixelFormatNames[];

    IDeckLink*			m_pDeckLink;
	bool				m_bRunning;
	IDeckLinkInput*		m_pInputCard;
	// IDeckLinkOutput*	m_pOutputCard;
    BMDDisplayMode      m_tDisplayMode;
    BMDPixelFormat      m_tPixelFormat;
};
