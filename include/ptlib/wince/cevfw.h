//
// (c) 2002 Yuri Kiryanov, openh323@kiryanov.com
// 
// Windows CE port of OpenH323 Open Source Project, www.openh323.org
// Video For Windows Definitions

#ifndef _CEVFW_H
#define _CEVFW_H

// video data block header
typedef struct videohdr_tag {
    LPBYTE      lpData;                 /* pointer to locked data buffer */
    DWORD       dwBufferLength;         /* Length of data buffer */
    DWORD       dwBytesUsed;            /* Bytes actually used */
    DWORD       dwTimeCaptured;         /* Milliseconds from start of stream */
    DWORD       dwUser;                 /* for client's use */
    DWORD       dwFlags;                /* assorted flags (see defines) */
    DWORD       dwReserved[4];          /* reserved for driver */
} VIDEOHDR, NEAR *PVIDEOHDR, FAR * LPVIDEOHDR;

/* dwFlags field of VIDEOHDR */
#define VHDR_DONE       0x00000001  /* Done bit */
#define VHDR_PREPARED   0x00000002  /* Set if this header has been prepared */
#define VHDR_INQUEUE    0x00000004  /* Reserved for driver */
#define VHDR_KEYFRAME   0x00000008  /* Key Frame */

typedef struct tagCapStatus {
    UINT        uiImageWidth;               // Width of the image
    UINT        uiImageHeight;              // Height of the image
    BOOL        fLiveWindow;                // Now Previewing video?
    BOOL        fOverlayWindow;             // Now Overlaying video?
    BOOL        fScale;                     // Scale image to client?
    POINT       ptScroll;                   // Scroll position
    BOOL        fUsingDefaultPalette;       // Using default driver palette?
    BOOL        fAudioHardware;             // Audio hardware present?
    BOOL        fCapFileExists;             // Does capture file exist?
    DWORD       dwCurrentVideoFrame;        // # of video frames cap'td
    DWORD       dwCurrentVideoFramesDropped;// # of video frames dropped
    DWORD       dwCurrentWaveSamples;       // # of wave samples cap'td
    DWORD       dwCurrentTimeElapsedMS;     // Elapsed capture duration
    HPALETTE    hPalCurrent;                // Current palette in use
    BOOL        fCapturingNow;              // Capture in progress?
    DWORD       dwReturn;                   // Error value after any operation
    UINT        wNumVideoAllocated;         // Actual number of video buffers
    UINT        wNumAudioAllocated;         // Actual number of audio buffers
} CAPSTATUS, *PCAPSTATUS, FAR *LPCAPSTATUS;

typedef struct tagCaptureParms {
    DWORD       dwRequestMicroSecPerFrame;  // Requested capture rate
    BOOL        fMakeUserHitOKToCapture;    // Show "Hit OK to cap" dlg?
    UINT        wPercentDropForError;       // Give error msg if > (10%)
    BOOL        fYield;                     // Capture via background task?
    DWORD       dwIndexSize;                // Max index size in frames (32K)
    UINT        wChunkGranularity;          // Junk chunk granularity (2K)
    BOOL        fUsingDOSMemory;            // Use DOS buffers?
    UINT        wNumVideoRequested;         // # video buffers, If 0, autocalc
    BOOL        fCaptureAudio;              // Capture audio?
    UINT        wNumAudioRequested;         // # audio buffers, If 0, autocalc
    UINT        vKeyAbort;                  // Virtual key causing abort
    BOOL        fAbortLeftMouse;            // Abort on left mouse?
    BOOL        fAbortRightMouse;           // Abort on right mouse?
    BOOL        fLimitEnabled;              // Use wTimeLimit?
    UINT        wTimeLimit;                 // Seconds to capture
    BOOL        fMCIControl;                // Use MCI video source?
    BOOL        fStepMCIDevice;             // Step MCI device?
    DWORD       dwMCIStartTime;             // Time to start in MS
    DWORD       dwMCIStopTime;              // Time to stop in MS
    BOOL        fStepCaptureAt2x;           // Perform spatial averaging 2x
    UINT        wStepCaptureAverageFrames;  // Temporal average n Frames
    DWORD       dwAudioBufferSize;          // Size of audio bufs (0 = default)
    BOOL        fDisableWriteCache;         // Attempt to disable write cache
    UINT        AVStreamMaster;             // Which stream controls length?
} CAPTUREPARMS, *PCAPTUREPARMS, FAR *LPCAPTUREPARMS;

typedef struct tagCapDriverCaps {
    UINT        wDeviceIndex;               // Driver index in system.ini
    BOOL        fHasOverlay;                // Can device overlay?
    BOOL        fHasDlgVideoSource;         // Has Video source dlg?
    BOOL        fHasDlgVideoFormat;         // Has Format dlg?
    BOOL        fHasDlgVideoDisplay;        // Has External out dlg?
    BOOL        fCaptureInitialized;        // Driver ready to capture?
    BOOL        fDriverSuppliesPalettes;    // Can driver make palettes?

// following always NULL on Win32.
    HANDLE      hVideoIn;                   // Driver In channel
    HANDLE      hVideoOut;                  // Driver Out channel
    HANDLE      hVideoExtIn;                // Driver Ext In channel
    HANDLE      hVideoExtOut;               // Driver Ext Out channel
} CAPDRIVERCAPS, *PCAPDRIVERCAPS, FAR *LPCAPDRIVERCAPS;

// Functions
DWORD capGetVideoFormatSize(HWND hwnd);
BOOL capGetStatus(HWND hwnd, LPCAPSTATUS s, int wSize);

DWORD capGetVideoFormat(HWND hwnd, LPVOID s, int wSize);
BOOL capSetVideoFormat(HWND hwnd, LPVOID s, int wSize);

BOOL capGrabFrameNoStop(HWND hwnd);

BOOL capGetDriverDescription (UINT wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer);

HWND capCreateCaptureWindow (
        LPCSTR lpszWindowName,
        DWORD dwStyle,
        int x, int y, int nWidth, int nHeight,
        HWND hwndParent, int nID);

typedef LRESULT (CALLBACK* CAPERRORCALLBACK)  (HWND hWnd, int nID, LPCSTR lpsz);
BOOL capSetCallbackOnError(HWND hwnd, CAPERRORCALLBACK fpProc);

typedef LRESULT (CALLBACK* CAPVIDEOCALLBACK)  (HWND hWnd, LPVIDEOHDR lpVHdr);
BOOL capSetCallbackOnFrame(HWND hwnd, CAPVIDEOCALLBACK fpProc);
BOOL capSetCallbackOnVideoStream(HWND hwnd, CAPVIDEOCALLBACK fpProc);

BOOL capDriverConnect(HWND hwnd, int i); 
BOOL capDriverDisconnect(HWND hwnd);

BOOL capSetUserData(HWND hwnd, LPVOID lUser);
LPVOID capGetUserData(HWND hwnd);

BOOL capCaptureGetSetup(HWND hwnd, LPCAPTUREPARMS s, int wSize);
BOOL capCaptureSetSetup(HWND hwnd, LPCAPTUREPARMS s, int wSize);

BOOL capDriverGetCaps(HWND hwnd, LPCAPDRIVERCAPS s, int wSize);
BOOL capPreview(HWND hwnd, BOOL f); 

#endif // _CEVFW_H
