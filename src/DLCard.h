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

  // duplicating these types here for type safety reasons
  typedef enum _bmdImageType {
    BMD_IMAGE_GRAYSCALE,
    BMD_IMAGE_COLOR
  } BMDImageType;

  DLCard() {};
  DLCard(IDeckLink* deckLink);
  ~DLCard();

  bool initGrabber(void);                                                            // start up decklink capture
  bool setDisplayMode(BMDDisplayMode displayMode);                                   // set the hardware display mode
  bool setPixelFormat(BMDPixelFormat pixelFormat);                                   // set the hardware pixel format
  bool setColorspace(BMDImageType imageType);                                        // set the image color space conversion
  bool getDisplayModeParams(long &modeWidth, long &modeHeight);                      // get the hardware width/height
  bool isVideoModeSupported(BMDDisplayMode displayMode, BMDPixelFormat pixelFormat); // query the hardware for mode and format support
  void close(void);                                                                  // shut down decklink capture
  void print_name(void);
  void print_attributes(void);
  void print_output_modes(void);
  void print_capabilities(void);
  bool running(void);                                                                // whether capture is running or not

  DLCapture*     m_pDelegate;                                                        // the capture callback delegate

private:
  void runThreadedCapture(void);

  // List of known pixel formats and their matching display names
  static const BMDPixelFormat   gKnownPixelFormats[];
  static const char *           gKnownPixelFormatNames[];

  IDeckLink*        m_pDeckLink;
  bool              m_bRunning;
  IDeckLinkInput*   m_pInputCard;
  BMDDisplayMode    m_tDisplayMode;
  BMDPixelFormat    m_tPixelFormat;
};
