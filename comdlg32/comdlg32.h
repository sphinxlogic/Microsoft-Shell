/*++

Copyright (c) 1990-1998,  Microsoft Corporation  All rights reserved.

Module Name:

    comdlg32.h

Abstract:

    This module contains the private header information for the Win32
    common dialogs.

Revision History:

--*/



#ifndef COMDLG_COMDLG32
#define COMDLG_COMDLG32

//
//  Include Files.
//
#include <w4warn.h>

/*
 *   Level 4 warnings to be turned on.
 *   Do not disable any more level 4 warnings.
 */
#pragma warning(disable:4306) // 'type cast' : conversion from 'LANGID' to 'LPVOID ' of greater size
#pragma warning(disable:4245) // 'initializing' : conversion from 'HRESULT' to 'DWORD', signed/unsigned mismatch
#pragma warning(disable:4213) // nonstandard extension used : cast on l-value
#pragma warning(disable:4305) // 'type cast' : truncation from 'LPVOID ' to 'LANGID'
#pragma warning(disable:4127) // conditional expression is constant
#pragma warning(disable:4189) // 'hEnum' : local variable is initialized but not referenced
#pragma warning(disable:4057) // 'function' : 'const LPCSTR ' differs in indirection to slightly different base types f 'BYTE [32]'
#pragma warning(disable:4706) // assignment within conditional expression
#pragma warning(disable:4701) // local variable 'lFract' may be used without having been initialized
#pragma warning(disable:4702) // unreachable code

#include "isz.h"
#include "cderr.h"

#ifdef __cplusplus
extern "C" {
#endif

//
//  Constant Declarations.
//

#define SEM_NOERROR               0x8003

#define CCHNETPATH                358

#define MAX_THREADS               128

#define CHAR_A                    TEXT('a')
#define CHAR_CAP_A                TEXT('A')
#define CHAR_C                    TEXT('c')
#define CHAR_Z                    TEXT('z')
#define CHAR_NULL                 TEXT('\0')
#define CHAR_COLON                TEXT(':')
#define CHAR_BSLASH               TEXT('\\')
#define CHAR_DOT                  TEXT('.')
#define CHAR_QMARK                TEXT('?')
#define CHAR_STAR                 TEXT('*')
#define CHAR_SLASH                TEXT('/')
#define CHAR_SPACE                TEXT(' ')
#define CHAR_QUOTE                TEXT('"')
#define CHAR_PLUS                 TEXT('+')
#define CHAR_LTHAN                TEXT('<')
#define CHAR_BAR                  TEXT('|')
#define CHAR_COMMA                TEXT(',')
#define CHAR_LBRACKET             TEXT('[')
#define CHAR_RBRACKET             TEXT(']')
#define CHAR_EQUAL                TEXT('=')
#define CHAR_SEMICOLON            TEXT(';')

#define STR_BLANK                 TEXT("")
#define STR_SEMICOLON             TEXT(";")

#define IS_DOTEND(ch)   ((ch) == CHAR_DOT || (ch) == 0 || ((ch) != CHAR_STAR))

#define PARSE_DIRECTORYNAME       -1
#define PARSE_INVALIDDRIVE        -2
#define PARSE_INVALIDPERIOD       -3
#define PARSE_MISSINGDIRCHAR      -4
#define PARSE_INVALIDCHAR         -5
#define PARSE_INVALIDDIRCHAR      -6
#define PARSE_INVALIDSPACE        -7
#define PARSE_EXTENSIONTOOLONG    -8
#define PARSE_FILETOOLONG         -9
#define PARSE_EMPTYSTRING         -10
#define PARSE_WILDCARDINDIR       -11
#define PARSE_WILDCARDINFILE      -12
#define PARSE_INVALIDNETPATH      -13
#define PARSE_NOXMEMORY           -14

#define OF_FILENOTFOUND           2
#define OF_PATHNOTFOUND           3
#define OF_NOFILEHANDLES          4
#define OF_ACCESSDENIED           5         // OF_NODISKINFLOPPY
#define OF_BUFFERTRUNCATED        6
#define OF_WRITEPROTECTION        19
#define OF_SHARINGVIOLATION       32
#define OF_NETACCESSDENIED        65
#define OF_DISKFULL               82
#define OF_INT24FAILURE           83
#define OF_CREATENOMODIFY         96
#define OF_NODRIVE                97
#define OF_PORTNAME               98
#define OF_LAZYREADONLY           99
#define OF_DISKFULL2              112

#ifndef DCE_UNICODIZED
  #define DeviceCapabilitiesExA DeviceCapabilitiesEx
#endif

//
//  Used to determine which type of message to send to the app.
//
#define COMDLG_ANSI               0x0
#define COMDLG_WIDE               0x1

#define HNULL                     ((HANDLE) 0)

#define cbResNameMax              32
#define cbDlgNameMax              32




//
//  Platform specific definitions.
//

#ifdef WINNT
  #define IS16BITWOWAPP(p) ((p)->Flags & CD_WOWAPP)
#else
  #define IS16BITWOWAPP(p) (GetProcessDword(0, GPD_FLAGS) & GPF_WIN16_PROCESS)
#endif

#ifdef WX86
  #define ISWX86APP(p)            ((p)->Flags & CD_WX86APP)
  #define GETGENERICHOOKFN(p,fn)  (ISWX86APP(p) ? Wx86GetX86Callback((p)->fn) : (p)->fn)
#else
  #define ISWX86APP(p)            (FALSE)
  #define GETGENERICHOOKFN(p,fn)  ((p)->fn)
#endif

#define GETHOOKFN(p)            GETGENERICHOOKFN(p,lpfnHook)
#define GETPRINTHOOKFN(p)       GETGENERICHOOKFN(p,lpfnPrintHook)
#define GETSETUPHOOKFN(p)       GETGENERICHOOKFN(p,lpfnSetupHook)
#define GETPAGEPAINTHOOKFN(p)   GETGENERICHOOKFN(p,lpfnPagePaintHook)

#ifndef CD_WX86APP
  #define CD_WX86APP      (0)     // Nothing special if we don't have it defined
#endif




//
//  Typedef Declarations.
//




//
//  External Declarations.
//

extern HINSTANCE g_hinst;              // instance handle of library

extern SHORT cyCaption, cyBorder, cyVScroll;
extern SHORT cxVScroll, cxBorder, cxSize;

extern TCHAR szNull[];
extern TCHAR szStar[];
extern TCHAR szStarDotStar[];

extern BOOL bMouse;                    // system has a mouse
extern BOOL bCursorLock;
extern BOOL bWLO;                      // running with WLO
extern BOOL bDBCS;                     // running DBCS
extern WORD wWinVer;                   // windows version
extern WORD wDOSVer;                   // DOS version
extern BOOL g_bUserPressedCancel;        // user pressed cancel button

//
//  initialized via RegisterWindowMessage
//
extern UINT msgWOWLFCHANGE;
extern UINT msgWOWDIRCHANGE;
extern UINT msgWOWCHOOSEFONT_GETLOGFONT;

extern UINT msgLBCHANGEA;
extern UINT msgSHAREVIOLATIONA;
extern UINT msgFILEOKA;
extern UINT msgCOLOROKA;
extern UINT msgSETRGBA;
extern UINT msgHELPA;

extern UINT msgLBCHANGEW;
extern UINT msgSHAREVIOLATIONW;
extern UINT msgFILEOKW;
extern UINT msgCOLOROKW;
extern UINT msgSETRGBW;
extern UINT msgHELPW;

extern UINT g_cfCIDA;
extern DWORD g_tlsLangID;



//
//  Function Prototypes.
//

VOID TermFind(void);
VOID TermColor(void);
VOID TermFont(void);
VOID TermFile(void);
VOID TermPrint(void);

void FreeImports(void);

//
//  dlgs.c
//
VOID
HourGlass(
    BOOL bOn);

void
StoreExtendedError(
    DWORD dwError);

DWORD
GetStoredExtendedError(void);

HBITMAP WINAPI
LoadAlterBitmap(
    int id,
    DWORD rgbReplace,
    DWORD rgbInstead);

VOID
AddNetButton(
    HWND hDlg,
    HANDLE hInstance,
    int dyBottomMargin,
    BOOL bAddAccel,
    BOOL bTryLowerRight,
    BOOL bTryLowerLeft);

BOOL
IsNetworkInstalled(void);

int CDLoadStringEx(UINT uiCP, HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax);
int CDLoadString(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax);

LANGID 
GetDialogLanguage(
    HWND hwndOwner, 
    HANDLE hDlgTemplate);

//
//  parse.c
//
int
ParseFileNew(
    LPTSTR pszPath,
    int *pnExtOffset,
    BOOL bWowApp,
    BOOL bNewStyle);

int
ParseFileOld(
    LPTSTR pszPath,
    int *pnExtOffset,
    int *pnOldExt,
    BOOL bWowApp,
    BOOL bNewStyle);

DWORD
ParseFile(
    LPTSTR lpstrFileName,
    BOOL bLFNFileSystem,
    BOOL bWowApp,
    BOOL bNewStyle);

LPTSTR
PathRemoveBslash(
    LPTSTR lpszPath);

BOOL
IsWild(
    LPCTSTR lpsz);

BOOL
AppendExt(
    LPTSTR lpszPath,
    DWORD cchPath,
    LPCTSTR lpExtension,
    BOOL bWildcard);

BOOL
IsUNC(
    LPCTSTR lpszPath);

BOOL
PortName(
    LPTSTR lpszFileName);

BOOL
IsDirectory(
    LPTSTR pszPath);

int
WriteProtectedDirCheck(
    LPCTSTR lpszFile);

BOOL
FOkToWriteOver(
    HWND hDlg,
    LPTSTR szFileName);

int
CreateFileDlg(
    HWND hDlg,
    LPTSTR szPath);




//
//  Fileopen specific stuff stashed here so we can free it upon
//  a DLL_PROCESS_DETACH.
//
typedef struct _OFN_DISKINFO {
    UINT   cchLen;           // number of chars allocated in 4 lptstrs
    LPTSTR lpAbbrName;       // single line form
    LPTSTR lpMultiName;      // drop-down form
    LPTSTR lpName;           // true form (for comparisons)
    LPTSTR lpPath;           // path prefix (a:, or \\server\share) for file searches
    TCHAR  wcDrive;          // drive letter, 0 for unc
    BOOL   bCasePreserved;
    DWORD  dwType;
    BOOL   bValid;
} OFN_DISKINFO;

#define MAX_DISKS                 100
#define WNETENUM_BUFFSIZE         0x4000

//
//  Defines for AddNetButton.
//
#define FILE_LEFT_MARGIN          5
#define FILE_RIGHT_MARGIN         3
#define FILE_TOP_MARGIN           0
#define FILE_BOTTOM_MARGIN        3


#ifdef WX86
  //
  // Wx86 support for calling from RISC into x86 hooks
  //
  PVOID
  Wx86GetX86Callback(
      PVOID lpfnHook);

  typedef PVOID
  (*PALLOCCALLBX86)(
      PVOID pfnx86,
      ULONG CBParamType,
      PVOID ThunkDebug,
      PULONG  pLogFlags);

  extern PALLOCCALLBX86 pfnAllocCallBx86;
#endif


#ifdef __cplusplus
};  // extern "C"
#endif


// For WOW support on WINNT
#ifdef WINNT
  VOID Ssync_ANSI_UNICODE_Struct_For_WOW(HWND hDlg, BOOL fDirection, DWORD dwID);
  VOID Ssync_ANSI_UNICODE_CC_For_WOW(HWND hDlg, BOOL f_ANSI_to_UNICODE);
  VOID Ssync_ANSI_UNICODE_CF_For_WOW(HWND hDlg, BOOL f_ANSI_to_UNICODE);
  VOID Ssync_ANSI_UNICODE_OFN_For_WOW(HWND hDlg, BOOL f_ANSI_to_UNICODE);
  VOID Ssync_ANSI_UNICODE_PD_For_WOW(HWND hDlg, BOOL f_ANSI_to_UNICODE);
#endif

// For nested FileOpen/Save common dialog support (something several 16-bit apps
// are known to do).  We keep a list of all the dialogs that are active for each
// thread in a process.  We make the assumption that common dialogs are THREAD
// modal -- so the first CURDLG struct in the list for a given thread is the
// currently active dialog (has the focus) for that thread.  The ptr to the head
// of the list is stored in the thread local storage (TLS) for the thread --
// indexed by g_tlsiCurDlg.  Bug #100453 et. al.
typedef struct _CURDLG {
  DWORD           dwCurDlgNum;     // incremental dlg number (per process)
  LPTSTR          lpstrCurDir;     // current directory for the current dialog
  struct _CURDLG *next;
} CURDLG;
typedef CURDLG *LPCURDLG;


//Macro to check if the given structure is the new structure.
#define IS_NEW_OFN(pOFN)  (pOFN->lStructSize >= sizeof(OPENFILENAME))

// moved from prnsetup.h
#define DN_PADDINGCHARS           16             // extra devnames padding

//
// Some local constants taken from printui.dll
//
enum 
{
    //
    // INTERNET_MAX_HOST_NAME_LENGTH is decalred in wininet.h
    //
    kDNSMax = INTERNET_MAX_HOST_NAME_LENGTH,
    kServerBufMax = kDNSMax + 2 + 1,

    //
    // Max printer name should really be MAX_PATH, but if you create
    // a max path printer and connect to it remotely, win32spl prepends
    // "\\server\" to it, causing it to exceed max path.  The new UI
    // therefore makes the max path MAX_PATH-kServerLenMax, but we still
    // allow the old case to work.
    //
    kPrinterBufMax = MAX_PATH + kServerBufMax + 1,

    //
    // The initial hint for EnumPrinters API.
    //
    kInitialPrinterHint = 0x400,
};

#ifdef __cplusplus
extern "C" {
#endif

HRESULT
ThunkDevNamesA2W(
    IN      HGLOBAL hDevNamesA,
    IN OUT  HGLOBAL *phDevNamesW
    );

HRESULT
ThunkDevNamesW2A(
    IN      HGLOBAL hDevNamesW,
    IN OUT  HGLOBAL *phDevNamesA
    );

HRESULT 
InvokeAddPrinterWizardModal(
    IN  HWND hwnd,
    OUT BOOL *pbPrinterAdded
    );

#ifdef __cplusplus
};  // extern "C"
#endif

#endif // !COMDLG_COMDLG32
