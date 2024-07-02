#include "priv.h"
#include "bindcb.h"
#include "resource.h"
#include <vrsscan.h>
#include "iface.h"
#include "security.h"
#include <wintrust.h>
#include "iehelpid.h"
#include <shlwapi.h>
#include "inetreg.h"
#include <varutil.h>
#include "dochost.h"
#include <mluisupp.h>
#include <downloadmgr.h>
#include "apithk.h"
#include "richedit.h"
#include <brutil.h>
#include <winsaferp.h>


#define MIME
#include "filetype.h"


#define ALLFILE_WILDCARD TEXT("*.*")

#define MAX_BYTES_STRLEN 64
#define CALC_NOW 5  // Recalcs Estimated time left every this many'th call to OnProgress

//
//  Enable WinVerifyTrust
//
#define CALL_WVT

#ifdef CALL_WVT
#include "wvtp.h"

//
//  Note that this is a global variable. It means we don't call LoadLibrary
// everytime we download an EXE (good), but the user need to reboot if
// WINTRUST.DLL is added later (bad). Since WINTRUST.DLL is part of IE 3.0,
// this is sufficient at this point.
//
Cwvt g_wvt;

HWND g_hDlgActive = NULL;   // get rid of this, not needed

//
// A named mutex is being used to determine if a critical operation exist, such as a file download.
// When we detect this we can prevent things like going offline while a download is in progress.
// To start the operation Create the named mutex. When the op is complete, close the handle.
// To see if any pending operations are in progress, Open the named mutex.  Success/fail will indicate
// if any pending operations exist.  This mechanism is being used to determine if a file download is
// in progress when the user attempts to go offline.  If so, we prompt them to let them know that going 
// offline will cancel the download(s).
HANDLE g_hCritOpMutex = NULL;


// SafeOpen dialog

UINT _VerifyTrust(HWND hwnd, LPCTSTR pszFileName, LPCWSTR pszStatusText);
#endif // CALL_WVT

// Do strong typechecking on the parameters
#ifdef SAFECAST
#undef SAFECAST
#endif
#define SAFECAST(_src, _type) (((_type)(_src)==(_src)?0:0), (_type)(_src))

extern HRESULT _GetRequestFlagFromPIB(IBinding *pib, DWORD *pdwOptions);
extern HRESULT _PrepareURLForDisplayUTF8W(LPCWSTR pwz, LPWSTR pwzOut, LPDWORD pcchOut, BOOL fUTF8Enabled, UINT uiCP);


UINT IE_ErrorMsgBox(IShellBrowser* psb,
                    HWND hwnd, HRESULT hrError, LPCWSTR szError, LPCTSTR szURL,
                    UINT idResource, UINT wFlags);
BOOL IsAssociatedWithIE(LPCWSTR pszFileName);

extern "C" EXECUTION_STATE WINAPI pSetThreadExecutionState(EXECUTION_STATE esFlags);  // Win2k+, Win98+ kernel32 API

#define DM_DOWNLOAD             TF_SHDPROGRESS
#define DM_PROGRESS             TF_SHDPROGRESS
#define DM_WVT                  TF_SHDPROGRESS

#define DWNLDMSG(psz, psz2)     TraceMsg(DM_DOWNLOAD, "shd TR-DWNLD::%s %s", psz, psz2)
#define DWNLDMSG2(psz, x)       TraceMsg(DM_DOWNLOAD, "shd TR-DWNLD::%s %x", psz, x)
#define DWNLDMSG3(psz, x, y)    TraceMsg(DM_DOWNLOAD, "shd TR-DWNLD::%s %x %x", psz, x, y)
#define DWNLDMSG4(psz, x, y, z) TraceMsg(DM_DOWNLOAD, "shd TR-DWNLD::%s %x %x %x", psz, x, y, z)

#define SAFEMSG(psz, psz2)      TraceMsg(0, "shd TR-SAFE::%s %s", psz, psz2)
#define SAFEMSG2(psz, x)        TraceMsg(0, "shd TR-SAFE::%s %x", psz, x)
#define EXPMSG(psz, psz2)       TraceMsg(0, "shd TR-EXP::%s %s", psz, psz2)
#define MDLGMSG(psz, x)         TraceMsg(0, "shd TR-MODELESS::%s %x", psz, x)
#define MSGMSG(psz, x)          TraceMsg(TF_SHDTHREAD, "ief MMSG::%s %x", psz, x)
#define PARKMSG(psz, x)         TraceMsg(TF_SHDTHREAD, "ief MPARK::%s %x", psz, x)

// File name and 32 for the rest of the title string
#define TITLE_LEN    (256 + 32)
#define MAX_DISPLAY_LEN 96
#define MAX_SCHEME_STRING 16
class CDownload : public IBindStatusCallback
            , public IAuthenticate
            , public IServiceProvider
            , public IHttpNegotiate
            , public IWindowForBindingUI
{
public:
    // *** IUnknown methods ***
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

    // *** IAuthenticate ***
    STDMETHODIMP Authenticate(
        HWND *phwnd,
        LPWSTR *pszUsername,
        LPWSTR *pszPassword);

    // *** IServiceProvider ***
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObj);

    // *** IBindStatusCallback ***
    STDMETHODIMP OnStartBinding(
        /* [in] */ DWORD grfBSCOption,
        /* [in] */ IBinding *pib);

    STDMETHODIMP GetPriority(
        /* [out] */ LONG *pnPriority);

    STDMETHODIMP OnLowResource(
        /* [in] */ DWORD reserved);

    STDMETHODIMP OnProgress(
        /* [in] */ ULONG ulProgress,
        /* [in] */ ULONG ulProgressMax,
        /* [in] */ ULONG ulStatusCode,
        /* [in] */ LPCWSTR szStatusText);

    STDMETHODIMP OnStopBinding(
        /* [in] */ HRESULT hresult,
        /* [in] */ LPCWSTR szError);

    STDMETHODIMP GetBindInfo(
        /* [out] */ DWORD *grfBINDINFOF,
        /* [unique][out][in] */ BINDINFO *pbindinfo);

    STDMETHODIMP OnDataAvailable(
        /* [in] */ DWORD grfBSCF,
        /* [in] */ DWORD dwSize,
        /* [in] */ FORMATETC *pformatetc,
        /* [in] */ STGMEDIUM *pstgmed);

    STDMETHODIMP OnObjectAvailable(
        /* [in] */ REFIID riid,
        /* [iid_is][in] */ IUnknown *punk);

    /* *** IHttpNegotiate ***  */
    STDMETHODIMP BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders,
            DWORD dwReserved, LPWSTR *pszAdditionalHeaders);

    STDMETHODIMP OnResponse(DWORD dwResponseCode,
                        LPCWSTR szResponseHeaders,
                        LPCWSTR szRequestHeaders,
                        LPWSTR *pszAdditionalRequestHeaders);

    STDMETHODIMP GetWindow(REFGUID RefGUI, HWND* phWnd);

protected:
    LONG        _cRef;
    LONG        _cRefDLD;
    IBinding*   _pib;
    IBindCtx*   _pbc;
    CDocObjectHost *_pdoh;
    HWND        _hDlg;
    HWND        _hwndToolTips;
    BOOL        _fSaveAs : 1;
    BOOL        _fGotFile : 1;
    BOOL        _fFirstTickValid : 1;
    BOOL        _fEndDialogCalled : 1;
    BOOL        _fDontPostQuitMsg : 1;  // Posts WM_QUIT message in destructor
    BOOL        _fCallVerifyTrust : 1;
    BOOL        _fStrsLoaded : 1;
    BOOL        _fSafe : 1;             // no need to call IsSafe dialog
    BOOL        _fDownloadStarted : 1; // Have we started receiving data
    BOOL        _fDownloadCompleted : 1;  // We have received BSCF_LASTDATANOTIFICATION
    BOOL        _fDeleteFromCache : 1; // Delete the file from cache when done
    BOOL        _fWriteHistory : 1;  // Should it be written to history? (SECURITY)
    BOOL        _fDismissDialog : 1;
    BOOL        _fUTF8Enabled : 1;
    DWORD       _dwFirstTick;
    DWORD       _dwFirstSize;
    DWORD       _dwTotalSize;           // Size of file downloaded so far
    DWORD       _dwFileSize;            // Size of file to download
    HICON       _hicon;
    TCHAR       _szPath[MAX_PATH];      // ok with MAX_PATH
    TCHAR       _szSaveToFile[MAX_PATH];    // File to Save to
    TCHAR       _szEstimateTime[MAX_PATH];  // ok with MAX_PATH
    TCHAR       _szBytesCopied[MAX_PATH];  // ok with MAX_PATH
    TCHAR       _szTitlePercent[TITLE_LEN];
    TCHAR       _szTitleBytes[TITLE_LEN];
    TCHAR       _szTransferRate[TITLE_LEN];
    TCHAR       _szURL[MAX_URL_STRING];
    TCHAR       _szDisplay[MAX_DISPLAY_LEN];   // URL to be displayed
    TCHAR       _szDefDlgTitle[256];
    TCHAR       _szExt[10];
    DWORD       _grfBINDF;
    BINDINFO*   _pbinfo;
    LPWSTR      _pwzHeaders;
    IMoniker*   _pmk;                   // WARNING: No ref-count (only for modal)
    LPWSTR      _pwszDisplayName;
    DWORD       _dwVerb;
    UINT        _uiCP;                  // Code page
    DWORD       _dwOldEst;
    ULONG       _ulOldProgress;
    DWORD       _dwOldRate;
    DWORD       _dwOldPcent;
    DWORD       _dwOldCur;
    BOOL        _fConfirmed;


    void SetMoniker(IMoniker* pmk) { _pmk=pmk; }
    BOOL _IsModal(void) { return (bool)_pmk; }

    virtual ~CDownload();
    friend INT_PTR CALLBACK DownloadDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend INT_PTR CALLBACK SafeOpenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    UINT _MayAskUserIsFileSafeToOpen(LPCTSTR pszMime);
    BOOL _GetSaveLocation(void);
    BOOL _SaveFile(void);
    void _DeleteFromCache(void);
    ULONG AddRefDLD(void);
    ULONG ReleaseDLD(void);
    HRESULT PerformVirusScan(LPCTSTR szFileName);

public:
    CDownload(BOOL fSaveAs = FALSE, LPWSTR pwzHeaders = NULL,
              DWORD grfBINDF = BINDF_ASYNCHRONOUS, BINDINFO* pbinfo = NULL,
              BOOL fSafe = FALSE, DWORD dwVerb = BINDVERB_GET, LPCTSTR pszRedir=NULL, UINT uiCP=CP_ACP, BOOL fConfirmed = FALSE);

    static void OpenUI(IMoniker* pmk, IBindCtx *pbc, BOOL fSaveAs = FALSE, BOOL fSafe = FALSE, LPWSTR pwzHeaders = NULL, DWORD dwVerb = BINDVERB_GET, DWORD grfBINDF = 0, BINDINFO* pbinfo = NULL, LPCTSTR pszRedir=NULL, UINT uiCP=CP_ACP, BOOL fConfirmed = FALSE);

    HRESULT StartBinding(IMoniker* pmk, IBindCtx *pbc = NULL);
    void EndDialogDLD(UINT id);
    void ShowStats(void);
    BOOL SetDismissDialogFlag(BOOL fDismiss) { return(_fDismissDialog = fDismiss); }
    BOOL GetDismissDialogFlag(void) { return(_fDismissDialog); }
#ifdef USE_LOCKREQUEST
    HRESULT LockRequestHandle(void);
#endif
};

CDownload::CDownload(BOOL fSaveAs, LPWSTR pwzHeaders, DWORD grfBINDF, BINDINFO* pbinfo, BOOL fSafe, DWORD dwVerb, LPCTSTR pszRedir, UINT uiCP, BOOL fConfirmed)
    : _cRef(1), _fSaveAs(fSaveAs), _fWriteHistory(1),
      _grfBINDF(grfBINDF), _pbinfo(pbinfo), _fSafe(fSafe), _pwzHeaders(pwzHeaders), _dwVerb(dwVerb), _uiCP(uiCP), _fConfirmed(fConfirmed)
{
    ASSERT(_fStrsLoaded == FALSE);
    ASSERT(_fDownloadStarted == FALSE);
    ASSERT(_fDownloadCompleted == FALSE);
    ASSERT(_fGotFile == FALSE);
    ASSERT(_fUTF8Enabled == FALSE);
    ASSERT(_hDlg == NULL);
    ASSERT(_pwszDisplayName == NULL);
    ASSERT(_dwTotalSize == 0);
    ASSERT(_dwFileSize == 0);
    ASSERT(_dwFirstTick == 0);
    ASSERT(_ulOldProgress == 0);
    ASSERT(_dwOldRate == 0);
    ASSERT(_dwOldPcent == 0);
    ASSERT(_dwOldCur == 0);

    _dwOldEst = 0xffffffff;
    
    if (pszRedir && lstrlen(pszRedir))
        StrCpyN(_szURL, pszRedir, ARRAYSIZE(_szURL) - 1); // -1 ???

    TraceMsg(TF_SHDLIFE, "CDownload::CDownload being constructed");
}

void ProcessStartbindingError(HWND hWnd, LPTSTR pszTitle, LPTSTR pszText, UINT uiFlag, HRESULT hres, LPCTSTR pszUrl)
{
    int nRet = -1;
    if (E_ACCESSDENIED == hres)
    {
        if(IEHardened() && pszUrl && pszUrl[0] != _T('\0'))
        {
            // Specialized dialog Windows DCR # 771532
            HMODULE hmod = LoadLibrary(TEXT("urlmon.dll"));

            if(hmod)
            {
                int (*pfnShowTrustAlertDialog)(HWND, IInternetSecurityManager *, LPVOID, LPCWSTR) = 
                    (int (*)(HWND, IInternetSecurityManager *, LPVOID, LPCWSTR))GetProcAddress(hmod, "ShowTrustAlertDialog");
                    
                if(pfnShowTrustAlertDialog)
                {
                    if(-1 == (nRet = pfnShowTrustAlertDialog(hWnd, NULL, NULL, pszUrl )) )
                    {
                        TraceMsg(DM_WARNING, "ShowTrustAlertDialog call from ProcessStartbindingError failed %x", GetLastError());
                    }
                }
                FreeLibrary(hmod);
            }
        }
        else
        {
            pszText = MAKEINTRESOURCE(IDS_DOWNLOADDISALLOWED);
            pszTitle = MAKEINTRESOURCE(IDS_SECURITYALERT);
            uiFlag = MB_ICONWARNING;
        }
    }

    if(-1 == nRet)
    {
        MLShellMessageBox(hWnd, pszText, pszTitle, MB_OK | MB_SETFOREGROUND | uiFlag );
    }

    if (IsValidHWND(hWnd))
    {
        FORWARD_WM_COMMAND(hWnd, IDCANCEL, NULL, 0, PostMessage);
    }
}

HRESULT SelectPidlInSFV(IShellFolderViewDual *psfv, LPCITEMIDLIST pidl, DWORD dwOpts)
{
    VARIANT var;
    HRESULT hr = InitVariantFromIDList(&var, pidl);
    if (SUCCEEDED(hr))
    {
        hr = psfv->SelectItem(&var, dwOpts);
        VariantClear(&var);
    }

    return hr;
}

void OpenFolderPidl(LPCITEMIDLIST pidl)
{
    SHELLEXECUTEINFO shei = { 0 };

    shei.cbSize     = sizeof(shei);
    shei.fMask      = SEE_MASK_INVOKEIDLIST;
    shei.nShow      = SW_SHOWNORMAL;
    shei.lpIDList   = (LPITEMIDLIST)pidl;
    ShellExecuteEx(&shei);
}

STDAPI OpenContainingFolderAndGetShellFolderView(HWND hwnd, LPCITEMIDLIST pidlFolder, IShellFolderViewDual **ppsfv)
{
    *ppsfv = NULL;
    
    IWebBrowserApp *pauto;
    HRESULT hr = SHGetIDispatchForFolder(pidlFolder, &pauto);
    if (SUCCEEDED(hr))
    {
        // We have IDispatch for window, now try to get one for
        // the folder object...
        HWND hwnd;
        if (SUCCEEDED(pauto->get_HWND((LONG_PTR *)&hwnd)))
        {
            // Make sure we make this the active window
            SetForegroundWindow(hwnd);
            ShowWindow(hwnd, SW_SHOWNORMAL);

        }
        IDispatch * pautoDoc;
        hr = pauto->get_Document(&pautoDoc);
        if (SUCCEEDED(hr))
        {
            hr = pautoDoc->QueryInterface(IID_PPV_ARG(IShellFolderViewDual, ppsfv));
            pautoDoc->Release();
        }
        pauto->Release();
    }
    return hr;
}

//
// Stolen (and modified) from shell\ext\mydocs2\prop.cpp which was from link.c in shell32.dll
//
void FindTarget(HWND hDlg, LPTSTR pPath)
{
    USHORT uSave;

    LPITEMIDLIST pidl = ILCreateFromPath( pPath );
    if (!pidl)
        return;

    LPITEMIDLIST pidlLast = ILFindLastID(pidl);

    // get the folder, special case for root objects (My Computer, Network)
    // hack off the end if it is not the root item
    if (pidl != pidlLast)
    {
        uSave = pidlLast->mkid.cb;
        pidlLast->mkid.cb = 0;
    }
    else
        uSave = 0;

    LPITEMIDLIST pidlDesk;
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &pidlDesk)))
    {
        BOOL fIsDesktopDir = pidlDesk && ILIsEqual(pidl, pidlDesk);

        if (fIsDesktopDir || !uSave)  // if it's in the desktop dir or pidl == pidlLast (uSave == 0 from above)
        {
            //
            // It's on the desktop...
            //

            MLShellMessageBox(hDlg, (LPTSTR)IDS_ON_DESKTOP, (LPTSTR)IDS_FIND_TITLE,
                             MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_TOPMOST);
        }
        else
        {
            if (WhichPlatform() == PLATFORM_BROWSERONLY)
            {
                OpenFolderPidl(pidl);
            }
            else
            {
                IShellFolderViewDual *psfv;
                if (SUCCEEDED(OpenContainingFolderAndGetShellFolderView(hDlg, uSave ? pidl : pidlDesk, &psfv)))
                {
                    if (uSave)
                        pidlLast->mkid.cb = uSave;
                    SelectPidlInSFV(psfv, pidlLast, SVSI_SELECT | SVSI_FOCUSED | SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE);
                    psfv->Release();
                }
            }
        }
        ILFree(pidlDesk);
    }

    ILFree(pidl);
}

BOOL SetExemptDelta(LPCTSTR pszURL, DWORD dwExemptDelta)
{
    BOOL fRC;
    INTERNET_CACHE_ENTRY_INFO icei;
    icei.dwStructSize = sizeof(icei);
    icei.dwExemptDelta = dwExemptDelta;    // Number of seconds from last access time to keep entry
    // Retry setting the exempt delta if it fails since wininet may have either not have created the
    //    entry yet or might have it locked.
    for (int i = 0; i < 5; i++)
    {
        if (fRC = SetUrlCacheEntryInfo(pszURL, &icei, CACHE_ENTRY_EXEMPT_DELTA_FC))
            break;
        Sleep(1000);
    }
    return fRC;
}

INT_PTR CALLBACK DownloadDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static fInBrowseDir = FALSE;
    CDownload* pdld = (CDownload*) GetWindowLongPtr(hDlg, DWLP_USER);
    DWORD dwExStyle = 0;
    TCHAR szURL[MAX_URL_STRING];    // make copies since EndDialog will delete CDownload obj
    BOOL fDownloadAborted;

    DWNLDMSG4("DownloadDlgProc ", uMsg, wParam, lParam);

    if ((pdld == NULL) && (uMsg != WM_INITDIALOG))
    {
        RIPMSG(TRUE, "CDownload freed (pdld == NULL) && (uMsg != WM_INITDIALOG)");
        return FALSE;
    }
    
    switch (uMsg) {
    case WM_INITDIALOG:
    {
        TCHAR szYesNo[20];
        DWORD dwType = REG_SZ;
        DWORD dwSize = ARRAYSIZE(szYesNo);

        if (lParam == NULL)
            return FALSE;
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pdld = (CDownload*)lParam;
        pdld->_hDlg = hDlg;

        EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_SIZE, MF_BYCOMMAND | MF_GRAYED);

        EnableWindow(GetDlgItem(hDlg, IDD_OPENFILE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_BROWSEDIR), FALSE);

        // On BiDi Loc Win98 & NT5 mirroring will take care of that
        // Need to fix only on BiBi Win95 Loc
        if (g_bBiDiW95Loc)
        {
            SetWindowBits(GetDlgItem(hDlg, IDD_DIR), GWL_EXSTYLE, WS_EX_RTLREADING, WS_EX_RTLREADING);
        }
        MLLoadString(IDS_DEFDLGTITLE, pdld->_szDefDlgTitle, ARRAYSIZE(pdld->_szDefDlgTitle));

        if (pdld->_hwndToolTips = CreateWindowEx(dwExStyle, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP,
                                  CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
                                  hDlg, NULL, HINST_THISDLL, NULL))
        {
            TOOLINFO ti;

            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
            ti.hwnd = hDlg;
            ti.uId = (UINT_PTR) GetDlgItem(hDlg, IDD_NAME);
            ti.lpszText = LPSTR_TEXTCALLBACK;
            ti.hinst = HINST_THISDLL;
            GetWindowRect((HWND)ti.uId, &ti.rect);
            SendMessage(pdld->_hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
        }
        // make sure we support cross-lang platform
        SHSetDefaultDialogFont(hDlg, IDD_NAME);

        pdld->SetDismissDialogFlag(FALSE);
        if ( SHRegGetUSValue( TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                              TEXT("NotifyDownloadComplete"),
                              &dwType, (void *)szYesNo, &dwSize, FALSE, NULL, 0 ) == ERROR_SUCCESS )
        {
            pdld->SetDismissDialogFlag(!StrCmpI(szYesNo, TEXT("No")));
        }
        CheckDlgButton(hDlg, IDD_DISMISS, pdld->GetDismissDialogFlag());

        DWNLDMSG("DownloadDlgProc", "Got WM_INITDIALOG");
        Animate_OpenEx(GetDlgItem(hDlg, IDD_ANIMATE), HINST_THISDLL, MAKEINTRESOURCE(IDA_DOWNLOAD));
        ShowWindow(GetDlgItem(hDlg, IDD_DOWNLOADICON), SW_HIDE);

        g_hCritOpMutex = CreateMutexA(NULL, TRUE, "CritOpMutex");
        
        // Automatically start binding if we are posting synchronously.
        if (pdld->_IsModal()) 
        {
            HRESULT hres = pdld->StartBinding(pdld->_pmk);
            ASSERT(pdld->_pmk);
            if (FAILED(hres))
            {
                ProcessStartbindingError(hDlg, MAKEINTRESOURCE(IDS_DOWNLOADFAILED),
                                         pdld->_szDisplay, MB_ICONWARNING, hres, pdld->_szURL);
            }
        }

        return TRUE;
    }

    case WM_SIZE:
        if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED))
            SetWindowText(hDlg, pdld->_szDefDlgTitle);
        break;

    case WM_NOTIFY:
    {
        LPTOOLTIPTEXT lpTT = (LPTOOLTIPTEXT) lParam;
        if (lpTT->hdr.code == TTN_NEEDTEXT)
        {
            lpTT->lpszText = pdld->_szURL;
            lpTT->hinst = NULL;
        }
    }
    break;

    case WM_COMMAND:
        DWNLDMSG2("DownloadDlgProc WM_COMMAND id =", GET_WM_COMMAND_ID(wParam, lParam));
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
        {
        case IDD_SAVEAS:
            if (pdld) 
            {
                pdld->AddRefDLD();
                BOOL fSuccess = FALSE;

                // Prevent someone from canceling dialog while the shell copy etc. is going on
                EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
                
                // If zone check fails or if we found virus, bail out and remove file from cache.
                if (pdld->PerformVirusScan(pdld->_szPath) != S_OK) 
                {
                    pdld->_fDeleteFromCache = TRUE;
                    pdld->EndDialogDLD(IDCANCEL);
                    break;
                }

                fSuccess = pdld->_SaveFile();

                AddUrlToUrlHistoryStg(pdld->_pwszDisplayName, NULL, NULL, pdld->_fWriteHistory, NULL, NULL, NULL);
                // -- BharatS --- Only add to history if Visible ?

                IEPlaySound(TEXT("SystemAsterisk"), TRUE);
                
                if (fSuccess)
                {
                    if (pdld->SetDismissDialogFlag(IsDlgButtonChecked(hDlg, IDD_DISMISS) == BST_CHECKED))
                    {
                        StrCpyN(szURL, pdld->_szURL, ARRAYSIZE(szURL));
                        pdld->EndDialogDLD(IDCANCEL);
                        SetExemptDelta(szURL, 0);
                    }
                    else
                    {
                        TCHAR szStr[MAX_PATH];

                        if (MLLoadString(IDS_CLOSE, szStr, ARRAYSIZE(szStr)))
                        {
                            SetWindowText(GetDlgItem(hDlg, IDCANCEL), szStr);
                        }

                        ShowWindow(GetDlgItem(hDlg, IDD_ANIMATE), SW_HIDE);
                        ShowWindow(GetDlgItem(hDlg, IDD_DNLDESTTIME), SW_HIDE);
                        ShowWindow(GetDlgItem(hDlg, IDD_DNLDCOMPLETEICON), SW_SHOW);
                        ShowWindow(GetDlgItem(hDlg, IDD_DNLDCOMPLETETEXT), SW_SHOW);
                        ShowWindow(GetDlgItem(hDlg, IDD_DNLDTIME), SW_SHOW);
                        
                        MLLoadString(IDS_SAVED, szStr, ARRAYSIZE(szStr));
                        SetDlgItemText(hDlg, IDD_OPENIT, szStr);

                        MLLoadString(IDS_DOWNLOADCOMPLETE, szStr, ARRAYSIZE(szStr));
                        SetWindowText(hDlg, szStr);

                        EnableWindow(GetDlgItem(hDlg, IDD_OPENFILE), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDD_BROWSEDIR), TRUE);

                        pdld->ShowStats();
                        pdld->ReleaseDLD();
                    }
                }
                else
                {
                    pdld->ReleaseDLD();
                }
                
                EnableWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);
            }
            break;

        case IDCANCEL:  // Cancel on abort, Close on dismiss
            if (pdld && IsWindowEnabled(GetDlgItem(hDlg, IDCANCEL))) 
            {
                pdld->AddRefDLD();
                fDownloadAborted  = pdld->_fDownloadStarted && !pdld->_fDownloadCompleted;
                StrCpyN(szURL, pdld->_szURL, ARRAYSIZE(szURL));
                
                if (pdld->_pib) 
                {
                    HRESULT hresT;
                    hresT = pdld->_pib->Abort();
                    TraceMsg(DM_DOWNLOAD, "DownloadDlgProc::IDCANCEL: called _pib->Abort() hres=%x", pdld->_pib, hresT);
                }
                pdld->EndDialogDLD(IDCANCEL);
                
                // Download was canceled.  Increase exempt time to keep downloaded a bit in case download is resumed
                SetExemptDelta(szURL, fDownloadAborted ?60*60*24 :0);
            }
            break;

        case IDD_BROWSEDIR:
                if (!fInBrowseDir) 
                {
                    pdld->AddRefDLD();  
                    fInBrowseDir = TRUE;    
                    
                    FindTarget(hDlg, pdld->_szSaveToFile);
                    
                    // Since EndDialogDLD will probably release our structure...
                    HWND hwndToolTips = pdld->_hwndToolTips;
                    pdld->_hwndToolTips = NULL;
                    pdld->EndDialogDLD(IDOK);

                    if (IsWindow(hwndToolTips))
                        DestroyWindow(hwndToolTips);
                        
                    fInBrowseDir = FALSE;
                }
#if DEBUG
                else
                {
                    TraceMsg(DM_DOWNLOAD, "DownloadDlgProc rcvd IDD_BROWSEDIR msg while already processing IDD_BROWSEDIR");
                }
#endif
                break;

        case IDD_OPENFILE:
            StrCpyN(pdld->_szPath, pdld->_szSaveToFile, ARRAYSIZE(pdld->_szPath));
        case IDOK:
            ShowWindow(GetDlgItem(hDlg, IDD_DISMISS), SW_HIDE);
            
            if (pdld)
            {
                pdld->AddRefDLD();
                
                if (pdld->_fGotFile) 
                {
                    // If zone check fails or if we found virus, bail out and remove file from cache.
                    if ( pdld->PerformVirusScan(pdld->_szPath) != S_OK ) 
                    {
                        pdld->_fDeleteFromCache = TRUE;
                    }
                    else
                    {
                        TCHAR  szQuotedPath[MAX_PATH];
                        StrCpyN(szQuotedPath, pdld->_szPath, MAX_PATH);

                        if (PLATFORM_INTEGRATED == WhichPlatform())
                        {
                            PathQuoteSpaces(szQuotedPath);
                        }

                        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO),
                                                 SEE_MASK_NOZONECHECKS, hDlg, NULL, szQuotedPath, NULL, NULL, SW_SHOWNORMAL, NULL};
                        if (!ShellExecuteEx(&sei))
                        {
                            DWNLDMSG2("ShellExecute failed", GetLastError());
                        }
                    }
                }

                if (!pdld->_fDeleteFromCache)
                    AddUrlToUrlHistoryStg(pdld->_pwszDisplayName, NULL, NULL, pdld->_fWriteHistory, NULL, NULL, NULL);

                // Since EndDialogDLD will probably release our structure...
                HWND hwndToolTips = pdld->_hwndToolTips;
                pdld->_hwndToolTips = NULL;
                StrCpyN(szURL, pdld->_szURL, ARRAYSIZE(szURL));
                
                pdld->EndDialogDLD(!pdld->_fDeleteFromCache ?IDOK :IDCANCEL);

                if (IsWindow(hwndToolTips))
                    DestroyWindow(hwndToolTips);
                SetExemptDelta(szURL, 0);
            }
            break;
        }
        break;


    case WM_ACTIVATE:
        if (pdld && pdld->_IsModal())
            return FALSE;
        else 
        {
            // There can be race conditions here. If the WA_ACTIVATE messages came in reverse
            // order, we might end up setting up the wrong hDlg as the active window. As of right now,
            // the only thing g_hDlgActive is being used for is for the IsDialogMessage in
            // CDownload_MayProcessMessage. And since there is only one tab-able control in this
            // dialog, a wrong hDlg in the g_hDlgActive should not hurt.
            ENTERCRITICAL;
            if (LOWORD(wParam) == WA_INACTIVE) 
            {
                if (g_hDlgActive == hDlg)
                {
                    MDLGMSG(TEXT("being inactivated"), hDlg);
                    g_hDlgActive = NULL;
                }
            } 
            else 
            {
                MDLGMSG(TEXT("being activated"), hDlg);
                g_hDlgActive = hDlg;
            }
            LEAVECRITICAL;
        }
        break;

    case WM_NCDESTROY:
        MDLGMSG(TEXT("being destroyed"), hDlg);
        ASSERT((pdld && pdld->_IsModal()) || (g_hDlgActive != hDlg));
        SetWindowLongPtr(hDlg, DWLP_USER, NULL);
        return FALSE;

    case WM_DESTROY:
        SHRemoveDefaultDialogFont(hDlg);
        return FALSE;

    default:
        return FALSE;
    }

    return TRUE;
}

void CDownload::ShowStats(void)
{
    TCHAR szStr[MAX_PATH];
    TCHAR szBytes[MAX_BYTES_STRLEN];
    TCHAR szTime[MAX_BYTES_STRLEN];
    TCHAR *pszTime = szTime;
    DWORD dwSpent = (GetTickCount() - _dwFirstTick);

    SetDlgItemText(_hDlg, IDD_NAME, _szDisplay);
    
    MLLoadString(IDS_BYTESTIME, _szBytesCopied, ARRAYSIZE(_szBytesCopied));
    StrFromTimeInterval(szTime, ARRAYSIZE(szTime), (dwSpent < 1000)  ?1000 :dwSpent, 3);
    while(pszTime && *pszTime && *pszTime == TEXT(' '))
        pszTime++;
    _FormatMessage(_szBytesCopied, szStr, ARRAYSIZE(szStr),
                StrFormatByteSize(_dwTotalSize, szBytes, MAX_BYTES_STRLEN), pszTime);
    SetDlgItemText(_hDlg, IDD_TIMEEST, szStr);

    // division below requires at least 1/2 second to have elapsed.
    if (dwSpent < 500)
        dwSpent = 500;
    _FormatMessage(_szTransferRate, szStr, ARRAYSIZE(szStr), 
                StrFormatByteSize(_dwTotalSize / ((dwSpent+500)/1000), szBytes, MAX_BYTES_STRLEN));
    SetDlgItemText(_hDlg, IDD_TRANSFERRATE, szStr);

    SetForegroundWindow(_hDlg);
}

void CDownload::EndDialogDLD(UINT id)
{
    if (ReleaseDLD() != 0)
        return;
        
    ASSERT(!_fEndDialogCalled);
    _fEndDialogCalled = TRUE;

    DWNLDMSG2("EndDialogDLD cRef ==", _cRef);
    TraceMsg(TF_SHDREF, "CDownload::EndDialogDLD called when _cRef=%d", _cRef);

    _fDismissDialog = (IsDlgButtonChecked(_hDlg, IDD_DISMISS) == BST_CHECKED);
    if (SHRegSetUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"), 
                    TEXT("NotifyDownloadComplete"), 
                    REG_SZ, _fDismissDialog ?TEXT("no") :TEXT("yes"), _fDismissDialog ?sizeof(TEXT("no")-sizeof(TCHAR)) :sizeof(TEXT("yes")-sizeof(TCHAR)), SHREGSET_FORCE_HKCU) != ERROR_SUCCESS)
    {
        DWNLDMSG2("SHRegSetUSValue NotifyDownloadComplete failed", GetLastError());
    }

    // HACK: USER does not send us WM_ACTIVATE when this dialog is
    //  being destroyed when it was activated. We need to work around
    //  this bug(?) by cleaning up g_hDlgActive.
    if (g_hDlgActive == _hDlg) 
    {
        MDLGMSG(TEXT("EndDialogDLD putting NULL in g_hDlgActive"), _hDlg);
        g_hDlgActive = NULL;
    }

    DestroyWindow(_hDlg);
    Release();
}

#define SZEXPLORERKEY  TEXT("Software\\Microsoft\\Internet Explorer")
#define SZDOWNLOADDIRVAL  TEXT("Download Directory")
// _GetSaveLocation
//      -   Tries to get the current download directory from the registry
//          default is the Desktop
//      -   Shows the FileSave Dialog
//      -   If the user changed the download location, save that off into
//          the registry for future downloads
//      -   _szSaveToFile is updated (this will be used by _SaveFile()
//
// Returns TRUE, if successfully done.
//
BOOL _GetSaveLocation(HWND hDlg, LPTSTR pszPath, LPTSTR pszExt, LPTSTR pszSaveToFile, UINT cchSaveToFile, BOOL fUTF8Enabled, UINT uiCP)
{
    BOOL fRet = FALSE;
    TCHAR * pszSaveTo =  NULL;
    HKEY hKey;
    BOOL fRegFileType = FALSE;
    TCHAR szDownloadDir[MAX_PATH];
    TCHAR szBuffer[MAX_PATH];
    TCHAR szTemp[40];
    LPTSTR pszWalk = szBuffer;
    int    cchWalk = ARRAYSIZE(szBuffer);
    int    cch;

    szDownloadDir[0] = 0;

    // If we don't have a download directory in the registry, download to the desktop
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, SZEXPLORERKEY, 0, KEY_READ, &hKey))
    {
        DWORD dwType, cbData = sizeof(szDownloadDir);
        RegQueryValueEx(hKey, SZDOWNLOADDIRVAL, NULL, &dwType, (LPBYTE)szDownloadDir, &cbData);
        RegCloseKey(hKey);
    }

    if (szDownloadDir[0] == 0)
        SHGetSpecialFolderPath(NULL, szDownloadDir, CSIDL_DESKTOPDIRECTORY, FALSE);

    // Get the file name. If there is no filename. create one called using the string resource in IDS_DOCUMENT

    pszSaveTo = PathFindFileName(pszPath);
    if (pszSaveTo)
    {
        DWORD cchData = cchSaveToFile;

        // Unescape the filename suggested by wininet.
        if (_PrepareURLForDisplayUTF8W(pszSaveTo, pszSaveToFile, &cchData, fUTF8Enabled, uiCP) != S_OK)
            StrCpyN(pszSaveToFile, pszSaveTo, cchSaveToFile);
            
        // Strip out any path that may have been encoded
        TCHAR * pszSaveToDst = pszSaveToFile;
        pszSaveTo = PathFindFileName(pszSaveToFile);
        if (pszSaveTo != pszSaveToFile)
        {
            while(*pszSaveTo)
                *pszSaveToDst++ = *pszSaveTo++;
            *pszSaveToDst = *pszSaveTo;
        }

        // Strip out the the cache's typical decoration of "(nn)"
        PathUndecorate (pszSaveToFile);
    }
    else
        MLLoadString(IDS_DOCUMENT, pszSaveToFile, cchSaveToFile);

    if (!g_fRunningOnNT) // Win9x isn't able to deal with DBCS chars in edit controls when UI lang is non-native OS lang
    {
        CHAR szBufA[MAX_PATH*2];
        int iRC = WideCharToMultiByte(CP_ACP, 0, pszSaveToFile, -1, szBufA, ARRAYSIZE(szBufA), NULL, NULL);
        if (iRC == 0)    // If we are unable to convert using system code page
            *pszSaveToFile = TEXT('\0');    // make suggested file name blank
    }
    
    OPENFILENAME OFN = {0};
    OFN.lStructSize        = sizeof(OFN);
    OFN.hwndOwner          = hDlg;
    OFN.nMaxFile           = cchSaveToFile;
    OFN.lpstrInitialDir    = szDownloadDir;

    OFN.lpstrFile = pszSaveToFile;
    OFN.Flags = OFN_HIDEREADONLY  | OFN_OVERWRITEPROMPT | OFN_EXPLORER |
                OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (!pszExt || !*pszExt)
        pszExt = PathFindExtension(pszPath);

    if (pszExt && *pszExt)
        OFN.lpstrDefExt = pszExt;

    // Try to get the file type name from the registry. To add to the filter pair strings
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, pszExt, 0, KEY_READ, &hKey))
    {
        DWORD dwType, cbData = sizeof(szBuffer);
        fRegFileType = (ERROR_SUCCESS == RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)szBuffer, &cbData));
        RegCloseKey(hKey);
    }

    if (fRegFileType)
    {
        fRegFileType = FALSE;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szBuffer, 0, KEY_READ, &hKey))
        {
            DWORD dwType, cbData = sizeof(szBuffer);
            szBuffer[0] = 0;

            fRegFileType = ERROR_SUCCESS == RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)szBuffer, &cbData);
            if (fRegFileType)
            {
                // Now tack on the second part of the filter pair
                int cchBuffer = lstrlen(szBuffer) + 1;
                pszWalk = szBuffer + cchBuffer;
                cchWalk = ARRAYSIZE(szBuffer) - cchBuffer;
                StrCpyN(pszWalk, TEXT("*"), cchWalk);
                StrCatBuff(pszWalk, pszExt, --cchWalk); // sub 1 for * above
            }
            RegCloseKey(hKey);
        }
        cch = lstrlen(pszWalk);
    }

    // There was no registry entry for the file type or the entry did not have a default value
    // So create the file name type - "<file extension> DOCUMENT"
    if (!fRegFileType || !(*szBuffer))
    {
        szBuffer[0] = 0;
        pszWalk = szBuffer;
        cchWalk = ARRAYSIZE(szBuffer);
        MLLoadString(IDS_EXTDOCUMENT, szTemp, ARRAYSIZE(szTemp));
        cch = wnsprintf(pszWalk, cchWalk, szTemp, pszExt, TEXT('\0'), pszExt);
    }

    // Add in the pair for "*.* All files"
    pszWalk += (cch + 1);
    cchWalk -= (cch + 1);

    MLLoadString(IDS_ALLFILES, szTemp, ARRAYSIZE(szTemp));
    StrCpyN(pszWalk, szTemp, cchWalk);

    cch = lstrlen(pszWalk) + 1;
    pszWalk += cch;
    cchWalk -= cch;

    StrCpyN(pszWalk, ALLFILE_WILDCARD, cchWalk);

    cch = (lstrlen( ALLFILE_WILDCARD )+1); //Add the second NULL to the end of the string
    pszWalk += cch;
    cchWalk -= cch;

    if (cchWalk > 0)
        *pszWalk = 0; //because we had some garbage put after memset.

    OFN.lpstrFilter = szBuffer;

    if ((fRet = (!SHIsRestricted2W(hDlg, REST_NoSelectDownloadDir, NULL, 0))) 
        && (fRet = GetSaveFileName(&OFN)))
    {
        // If the download location was changed, save that off to the registry
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, SZEXPLORERKEY, 0, KEY_WRITE, &hKey))
        {
            StrCpyN(szBuffer, pszSaveToFile, ARRAYSIZE(szBuffer));
            PathRemoveFileSpec(szBuffer);

            if (szBuffer[0])
                RegSetValueEx(hKey, SZDOWNLOADDIRVAL, 0, REG_SZ, (LPBYTE)szBuffer, CbFromCch(lstrlen(szBuffer) + 1));

            RegCloseKey(hKey);
        }
    }

    return fRet;
}


BOOL CDownload::_GetSaveLocation()
{
    return ::_GetSaveLocation(_hDlg, _szPath, _szExt, _szSaveToFile, ARRAYSIZE(_szSaveToFile), _fUTF8Enabled, _uiCP);
}

BOOL CDownload::_SaveFile()
{
    SHFILEOPSTRUCT fo = { _hDlg, FO_COPY, _szPath, _szSaveToFile, FOF_NOCONFIRMATION | FOF_NOCOPYSECURITYATTRIBS};

    // Be sure the strings are double terminated
    DWORD dwLen = (DWORD)min(ARRAYSIZE(_szPath), lstrlen(_szPath) + 1);
    if (dwLen == 0)  // Not likely, but better to fail than trash someone else's data
        return FALSE;
    _szPath[dwLen] = TEXT('\0');
    _szPath[dwLen-1] = TEXT('\0');
    
    dwLen = (DWORD)min(ARRAYSIZE(_szSaveToFile), lstrlen(_szSaveToFile) + 1);
    if (dwLen == 0)
        return FALSE;
    _szSaveToFile[dwLen] = TEXT('\0');
    _szSaveToFile[dwLen-1] = TEXT('\0');

    // If the file is in the cache, we probably want to delete it from the
    // cache to free up some disk space rather than wait for it to be scavenged.
    // This is best done after _pib->Release called from ~CDownload.
    _fDeleteFromCache = TRUE;

    // Copy the file (which is locked, so can't move it) to its target destination.
    return !SHFileOperation(&fo);
}

void CDownload::_DeleteFromCache()
{
    INTERNET_CACHE_CONFIG_INFO CCInfo;
    DWORD dwCCIBufSize = sizeof(CCInfo);

    // Obtain the cache directory path.

    if (!GetUrlCacheConfigInfo (&CCInfo, &dwCCIBufSize, CACHE_CONFIG_CONTENT_PATHS_FC))
    {
        ASSERT(FALSE);
    }
    else if (0 == StrCmpNI (_szPath,
        CCInfo.CachePaths[0].CachePath,
        lstrlen(CCInfo.CachePaths[0].CachePath)))
    {
        // Attempt to delete the file from the cache only if resides under
        // the cache directory, otherwise we could in theory nuke a preinstalled
        // or edited cache entry.  Here a prefix match is also a string prefix
        // match since .CachePath will have a trailing slash ('/')

        DeleteUrlCacheEntry(_szURL);
    }
}


void CDownload::OpenUI(IMoniker* pmk, IBindCtx *pbc, BOOL fSaveAs, BOOL fSafe, LPWSTR pwzHeaders, DWORD dwVerb, DWORD grfBINDF, BINDINFO* pbinfo, LPCTSTR pszRedir, UINT uiCP, BOOL fConfirmed)
{
    TraceMsg(DM_DOWNLOAD, "CDownLoad::OpenUI called with fSaveAs=%d, verb=%d", fSaveAs, dwVerb);

    // CDownload will take ownership pbinfo.
    CDownload* pdld = new CDownload(fSaveAs, pwzHeaders, grfBINDF, pbinfo, fSafe, dwVerb, pszRedir, uiCP, fConfirmed);
    if (pdld) 
    {
        HWND hwnd = CreateDialogParam(MLGetHinst(), 
            MAKEINTRESOURCE(DLG_DOWNLOADPROGRESS), NULL, DownloadDlgProc, (LPARAM)pdld);
        pwzHeaders = NULL;   // Owner is now CDownload
        DWNLDMSG2("CDownLoad_OpenUI dialog created", hwnd);
        if (hwnd)
        {
            HRESULT hres = pdld->StartBinding(pmk, pbc);
            if (FAILED(hres))
            {
                TraceMsg(DM_DOWNLOAD, "CDownLoad::OpenUI() - StartBinding() Failed with hres=0x%x!", hres );

                ProcessStartbindingError(hwnd, MAKEINTRESOURCE(IDS_DOWNLOADFAILED),
                                         pdld->_szDisplay, MB_ICONWARNING, hres, pdld->_szURL);
            }
            else
            {
                ShowWindow(hwnd, SW_SHOWNORMAL);
            }
        }
        else
        {
            delete pdld;
            pdld = NULL;
        }
    }

    if (pwzHeaders)
    {
        CoTaskMemFree(pwzHeaders);
        pwzHeaders = NULL;
    }
}

BOOL CDownload_MayProcessMessage(MSG* pmsg)
{
    if (g_hDlgActive)
        return IsDialogMessage(g_hDlgActive, pmsg);

    return FALSE;       // not processed
}

class CDownloadThreadParam {
#ifdef DEBUG
    const DWORD* _pdwSigniture;
    static const DWORD s_dummy;
#endif
public:
    DWORD   _dwVerb;
    DWORD   _grfBINDF;
    BINDINFO *_pbinfo;
    LPWSTR  _pszDisplayName;
    LPWSTR  _pwzHeaders;
    BOOL    _fSaveAs;
    BOOL    _fSafe;
    BOOL    _fConfirmed;
    IStream *_pStream;
    TCHAR   _szRedirURL[MAX_URL_STRING];
    UINT    _uiCP;

    ~CDownloadThreadParam() 
    {
        OleFree(_pszDisplayName);
        if (_pwzHeaders) 
            CoTaskMemFree(_pwzHeaders);
        if (_pStream)
            _pStream->Release();
        // CDownload releases our _pbinfo.
    }

    CDownloadThreadParam(LPWSTR pszDisplayName, LPWSTR pwzHeaders, BOOL fSaveAs, BOOL fSafe=FALSE, DWORD dwVerb=BINDVERB_GET, DWORD grfBINDF = 0, BINDINFO* pbinfo = NULL, LPCTSTR pszRedir=NULL, UINT uiCP=CP_ACP, BOOL fConfirmed=FALSE )
        : _pszDisplayName(pszDisplayName), _fSaveAs(fSaveAs), _fSafe(fSafe), _pwzHeaders(pwzHeaders), _pStream(NULL), _dwVerb(dwVerb), _grfBINDF(grfBINDF), _pbinfo(pbinfo), _uiCP(uiCP), _fConfirmed(fConfirmed)
    {
#ifdef DEBUG
        _pdwSigniture = &s_dummy;
#endif
        if (pszRedir && lstrlen(pszRedir))
            StrCpyN(_szRedirURL, pszRedir, MAX_URL_STRING - 1);
        // CDownload releases our _pbinfo.
    }

    void SetStream(IStream *pStm)
    {
        if (_pStream)
        {
            _pStream->Release();
        }
        _pStream = pStm;

        if (_pStream)
        {
            _pStream->AddRef();
        }
    }
};

DWORD CALLBACK IEDownload_ThreadProc(void *pv)
{
    CDownloadThreadParam* pdtp = (CDownloadThreadParam*)pv;

    HRESULT hr;

    IBindCtx *pbc = NULL;
    if (pdtp->_pStream)
    {
        pdtp->_pStream->AddRef();
        hr = pdtp->_pStream->Seek(c_li0,STREAM_SEEK_SET,0);
        hr = CoGetInterfaceAndReleaseStream(pdtp->_pStream, IID_PPV_ARG(IBindCtx, &pbc));
        pdtp->SetStream(NULL);
    }

    if (pbc == NULL)
        CreateBindCtx(0, &pbc);

    //winse#12726:Give other thread a chance to finish its work.
    Sleep(100);

    hr = CDownLoad_OpenUIURL(pdtp->_pszDisplayName, pbc, pdtp->_pwzHeaders, TRUE, pdtp->_fSaveAs, pdtp->_fSafe,
                             pdtp->_dwVerb, pdtp->_grfBINDF, pdtp->_pbinfo, pdtp->_szRedirURL, pdtp->_uiCP, NULL, pdtp->_fConfirmed);

    if (SUCCEEDED(hr)) 
    {
        pdtp->_pwzHeaders = NULL;   // CDownload owns freeing headers now
        pdtp->_pbinfo = NULL;       // CDownload owns freeing pbinfo now.
    }

    delete pdtp;
    pdtp = NULL;

    if (pbc)
    {
        pbc->Release();
        pbc = NULL;
    }

    while (1)
    {
        MSG msg;

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            // Note that for IE 3.0, the parking thread is also
            //  the owner of all modeless download dialog.
            if (CDownload_MayProcessMessage(&msg)) 
                continue;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        WaitMessage();
    }

    return 0;
}

void CDownLoad_OpenUI(IMoniker *pmk, IBindCtx *pbc, BOOL fSync, BOOL fSaveAs, BOOL fSafe, LPWSTR pwzHeaders, DWORD dwVerb, DWORD grfBINDF, BINDINFO* pbinfo, LPCTSTR pszRedir, UINT uiCP, IUnknown *punk, BOOL fConfirmed)
{
    TraceMsg(DM_DOWNLOAD, "CDownLoad_OpenUI called with fSync=%d fSaveAs=%d", fSync, fSaveAs);
    ASSERT(dwVerb == BINDVERB_GET || dwVerb == BINDVERB_POST);

    if (fSync) 
    {
        CDownload::OpenUI(pmk, pbc, fSaveAs, fSafe, pwzHeaders, dwVerb, grfBINDF, pbinfo, pszRedir, uiCP, fConfirmed);
        pwzHeaders = NULL;  // CDownload now owns headers
        return;
    }

    IDownloadManager *pdlm;
    HRESULT hr = IUnknown_QueryService(punk, SID_SDownloadManager, IID_PPV_ARG(IDownloadManager, &pdlm));
    if (FAILED(hr))
    {
        hr = CreateFromRegKey(TSZIEPATH, TEXT("DownloadUI"), IID_PPV_ARG(IDownloadManager, &pdlm));
    }

    if (SUCCEEDED(hr))
    {
        hr = pdlm->Download(pmk, pbc, dwVerb, grfBINDF, pbinfo, pwzHeaders, pszRedir, uiCP);
        pdlm->Release();
    }

    if (FAILED(hr))
    {
        if (pbc == NULL)
        {
            hr = CreateBindCtx(0, &pbc);
        }
        else
        {
            hr = S_OK;
            pbc->AddRef();
        }

        if (SUCCEEDED(hr))
        {
            LPWSTR pszDisplayName = NULL;
            hr = pmk->GetDisplayName(pbc, NULL, &pszDisplayName);
            if (SUCCEEDED(hr)) 
            {
                CDownloadThreadParam* pdtp = new CDownloadThreadParam(pszDisplayName, pwzHeaders, fSaveAs, fSafe, dwVerb, grfBINDF, pbinfo, pszRedir, uiCP, fConfirmed);
                if (pdtp) 
                {
                    pwzHeaders = NULL;  // ownership is to CDTP

                    // Note: IAsyncBindCtx has identicial interface as IBindCtx
                    IBindCtx *pbcAsync;
                    hr = pbc->QueryInterface(IID_IAsyncBindCtx, (void **)&pbcAsync);
                    if (SUCCEEDED(hr))
                    {
                        // This introduces a double bind, but only for the mk: protocol and
                        // the fix is needed for displaying pdfs and other special mime types.
                        if (_tcsnicmp(pszDisplayName, _T("mk:"), 3) == 0)
                        {
                            pbcAsync->Release();
                            pbcAsync = NULL;
                            hr = CreateBindCtx(0, &pbcAsync);
                        }

                        if (SUCCEEDED(hr))
                        {
                            IStream *pStm;
                            hr = CoMarshalInterThreadInterfaceInStream(IID_IBindCtx, pbcAsync, &pStm);
                            if (hr == S_OK)
                            {
                                pdtp->SetStream(pStm);
                                pStm->Release();
                            }
                            pbcAsync->Release();
                        }
                    }

                    if (!SHCreateThread(IEDownload_ThreadProc, pdtp, CTF_PROCESS_REF | CTF_REF_COUNTED | CTF_COINIT, NULL))
                    {
                        delete pdtp;
                        pdtp = NULL;
                    }
                } 
                else 
                {
                    OleFree(pszDisplayName);
                }
            }
            pbc->Release();
        }
    }

    CoTaskMemFree(pwzHeaders);  // may be NULL, we consume this in all cases
}

HRESULT CDownLoad_OpenUIURL(LPCWSTR pwszURL, IBindCtx *pbc, LPWSTR pwzHeaders, BOOL fSync,BOOL fSaveAs, BOOL fSafe, DWORD dwVerb, DWORD grfBINDF, BINDINFO* pbinfo, LPCTSTR pszRedir, UINT uiCP, IUnknown *punk, BOOL fConfirmed)
{
    HRESULT hr;
    ASSERT(pwszURL);
    if (pwszURL) 
    {
        IMoniker* pmk = NULL;
        hr = CreateURLMoniker(NULL, pwszURL, &pmk);
        if (SUCCEEDED(hr)) 
        {
            CDownLoad_OpenUI(pmk, pbc, fSync, fSaveAs, fSafe, pwzHeaders, dwVerb, grfBINDF, pbinfo, pszRedir, uiCP, punk, fConfirmed);
            pwzHeaders = NULL;  // CDownload now owns headers
            pmk->Release();
            hr = S_OK;
        }
        if (pwzHeaders)
            CoTaskMemFree(pwzHeaders);
    }
    else
        hr = E_INVALIDARG;
    return hr;
}

HRESULT CDownload::StartBinding(IMoniker* pmk, IBindCtx *pbc)
{
    ASSERT(_pbc==NULL);
    HRESULT hr = S_OK;

    if (pbc == NULL)
    {
        hr = CreateBindCtx(0, &_pbc);
    }
    else
    {
        _pbc = pbc;
        _pbc->AddRef();
    }

    if (SUCCEEDED(hr))
    {
        hr = RegisterBindStatusCallback(_pbc, this, 0, 0);
        if (SUCCEEDED(hr)) 
        {
            hr = pmk->GetDisplayName(_pbc, NULL, &_pwszDisplayName);
            if (SUCCEEDED(hr))
            {
                TCHAR szBuf[MAX_PATH];
                DWORD dwSize = ARRAYSIZE(szBuf);
                DWORD dwPUAF = PUAF_NOUI;

                DWORD dwPolicy = 0, dwContext = 0;
                int cch = lstrlen(_szURL);

                if (!cch)
                {
                    SHUnicodeToTChar(_pwszDisplayName, _szURL, ARRAYSIZE(_szURL));
                }

                TraceMsg(TF_SHDNAVIGATE, "CDld::StartBinding SHUnicodeToTChar returns %d (%s)", cch, _szURL);

                // The URL from GetDisplayName() is always fully
                // canonicalized and escaped.  Prepare it for display.
                if (PrepareURLForDisplay(_szURL, szBuf, &dwSize))
                    FormatUrlForDisplay(szBuf, _szDisplay, ARRAYSIZE(_szDisplay), NULL, 0, TRUE, _uiCP, NULL);
                else
                    FormatUrlForDisplay(_szURL, _szDisplay, ARRAYSIZE(_szDisplay), NULL, 0, TRUE, _uiCP, NULL);

                SetWindowText(GetDlgItem(_hDlg, IDD_NAME), _szDisplay);

                if (_grfBINDF & BINDF_ENFORCERESTRICTED)
                {
                    dwPUAF |= PUAF_ENFORCERESTRICTED;
                }

                ZoneCheckUrlEx(_szURL, &dwPolicy, sizeof(dwPolicy), &dwContext, sizeof(dwContext),
                               URLACTION_SHELL_FILE_DOWNLOAD, dwPUAF, NULL);
                dwPolicy = GetUrlPolicyPermissions(dwPolicy);
                if ((dwPolicy == URLPOLICY_ALLOW) || (dwPolicy == URLPOLICY_QUERY)) 
                {
                    IUnknown* punk = NULL;
                    hr = pmk->BindToStorage(_pbc, NULL, IID_PPV_ARG(IUnknown, &punk));
                    DWNLDMSG3("StartBinding pmk->BindToStorage returned", hr, punk);
                    if (SUCCEEDED(hr) || hr == E_PENDING)
                    {
                        hr = S_OK;
                        if (punk)
                        {
                            ASSERT(0);
                            punk->Release();
                        }

                    }
                    else 
                    {
                        TraceMsg(DM_ERROR, "CDld::StartBinding pmk->BindToStorage failed %x", hr);

                        HRESULT hrRevoke = RevokeBindStatusCallback( _pbc, this );

                        ASSERT( SUCCEEDED( hrRevoke ) );
                    }
                } 
                else
                {
                    TraceMsg(DM_ERROR, "CDld::StartBinding: Zone does not allow file download");

                    HRESULT hrRevoke = RevokeBindStatusCallback( _pbc, this );

                    ASSERT( SUCCEEDED( hrRevoke ) );

                    hr = E_ACCESSDENIED;
                }
            }
            else
            {
                TraceMsg(DM_ERROR, "CDld::StartBinding pmk->GetDisplayName failed %x", hr);

                HRESULT hrRevoke = RevokeBindStatusCallback( _pbc, this );

                ASSERT( SUCCEEDED( hrRevoke ) );
            }
        }
        else 
        {
            TraceMsg(DM_ERROR, "CDld::StartBinding RegisterBSC failed %x", hr);
        }
    }
    else
    {
        TraceMsg(DM_ERROR, "CDld::StartBinding CreateBindCtx failed %x", hr);
    }
    return hr;
}

HRESULT CDownload::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { 
        QITABENT(CDownload, IBindStatusCallback),   // IID_IBindStatusCallback
        QITABENT(CDownload, IAuthenticate),         // IID_IAuthenticate
        QITABENT(CDownload, IServiceProvider),      // IID_IServiceProvider
        QITABENT(CDownload, IHttpNegotiate),        // IID_IHttpNegotiate
        QITABENT(CDownload, IWindowForBindingUI),
        { 0 }, 
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CDownload::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDownload::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);

    DWNLDMSG2("CDownload::Release cRef=", cRef);

    if ( 0 == cRef )
    {
        CDownload* pdld = (CDownload*) GetWindowLongPtr(_hDlg, DWLP_USER);
        if (pdld == this)
        {
            SetWindowLongPtr(_hDlg, DWLP_USER, NULL);
        }

        DWNLDMSG3("CDownload::Release delete this", pdld, this);

        delete this;
    }

    return cRef;
}

ULONG CDownload::AddRefDLD()
{
    return InterlockedIncrement(&_cRefDLD);
}

ULONG CDownload::ReleaseDLD()
{
    ASSERT( 0 != _cRefDLD );
    return InterlockedDecrement(&_cRefDLD);
}

CDownload::~CDownload()
{
    if (_pbinfo) {
        ReleaseBindInfo(_pbinfo);
        LocalFree(_pbinfo);
        _pbinfo = NULL;
    }

    if (_pib) {
        _pib->Release();
    }

    if (_pbc) {
        _pbc->Release();
    }

    if (_hicon) {
        DestroyIcon(_hicon);
    }

    if (_pwszDisplayName)
        OleFree(_pwszDisplayName);

    if (_fDeleteFromCache)
        _DeleteFromCache();

    if ( _pwzHeaders )
        CoTaskMemFree( _pwzHeaders );

    TraceMsg(TF_SHDLIFE, "CDownload::~CDownload being destructed");

    TraceMsg(TF_SHDTHREAD, "CDownload::EndDialogDLD calling PostQuitMessage");
    // Post the quit message ONLY if this flag is set. The constructor for the
    // derived class CDownloadURL resets the flag to FALSE because it doesn't
    // need any quit messages.
    if (!_fDontPostQuitMsg)
        PostQuitMessage(0);
}

#ifdef USE_LOCKREQUEST
HRESULT CDownload::LockRequestHandle(void)
{
    HRESULT hres = E_FAIL;
    HANDLE hLock;
    
    if (_pib)
    {
        IWinInetInfo* pwinet;
        hres = _pib->QueryInterface(IID_PPV_ARG(IWinInetInfo, &pwinet));
        if (SUCCEEDED(hres)) 
        {
            DWORD cbSize = sizeof(HANDLE);
            hres = pwinet->QueryOption(WININETINFO_OPTION_LOCK_HANDLE, &hLock, &cbSize);

            pwinet->Release();
        }
    }
    return hres;
}
#endif

HRESULT CDownload::OnStartBinding(DWORD grfBSCOption, IBinding *pib)
{
    DWNLDMSG3("OnStartBinding", _pib, pib);
    if (_pib) 
    {
        _pib->Release();
    }

    _pib = pib;
    if (_pib) 
    {
        _pib->AddRef();
    }

    SetQueryNetSessionCount(SESSION_INCREMENT);

    _fUTF8Enabled = UTF8Enabled();

    return S_OK;
}

HRESULT CDownload::GetPriority(LONG *pnPriority)
{
    DWNLDMSG("GetPriority", "called");
    *pnPriority = NORMAL_PRIORITY_CLASS;
    return S_OK;
}

HRESULT CDownload::OnLowResource(DWORD reserved)
{
    DWNLDMSG("OnLowResource", "called");
    return S_OK;
}


#define WM_DIALMON_FIRST        WM_USER+100

// message sent to dial monitor app window indicating that there has been
// winsock activity and dial monitor should reset its idle timer
#define WM_WINSOCK_ACTIVITY             WM_DIALMON_FIRST + 0

#define MIN_ACTIVITY_MSG_INTERVAL       15000

void IndicateWinsockActivity(void)
{
    // if there is an autodisconnect monitor, send it an activity message
    // so that we don't get disconnected during long downloads.  For perf's sake,
    // don't send a message any more often than once every MIN_ACTIVITY_MSG_INTERVAL
    // milliseconds (15 seconds).  Use GetTickCount to determine interval;
    // GetTickCount is very cheap.
    DWORD dwTickCount = GetTickCount();
    // Sharing this among multiple threads is OK
    static DWORD dwLastActivityMsgTickCount = 0;
    DWORD dwElapsed = dwTickCount - dwLastActivityMsgTickCount;
    
    // have we sent an activity message recently?
    if (dwElapsed > MIN_ACTIVITY_MSG_INTERVAL) 
    {
        HWND hwndMonitorApp = FindWindow(TEXT("MS_AutodialMonitor"), NULL);
        if (hwndMonitorApp) 
        {
            PostMessage(hwndMonitorApp, WM_WINSOCK_ACTIVITY, 0, 0);
        }
        hwndMonitorApp = FindWindow(TEXT("MS_WebcheckMonitor"), NULL);
        if (hwndMonitorApp) 
        {
            PostMessage(hwndMonitorApp, WM_WINSOCK_ACTIVITY, 0, 0);
        }
        
        // record the tick count of the last time we sent an
        // activity message
        dwLastActivityMsgTickCount = dwTickCount;
    }
}

#define MAXCALCCNT 5

HRESULT CDownload::OnProgress(
     ULONG ulProgress,
     ULONG ulProgressMax,
     ULONG ulStatusCode,
     LPCWSTR pwzStatusText)
{
    DWNLDMSG4("OnProgress", ulProgress, ulProgressMax, ulStatusCode);
    TCHAR szBytes[MAX_BYTES_STRLEN];
    TCHAR szBytesMax[MAX_BYTES_STRLEN];
    TCHAR szBuf[MAX_PATH];      // OK with MAX_PATH
    LPTSTR pszFileName = NULL;
    HWND hwndShow;
    DWORD dwCur;

    switch (ulStatusCode)
    {
        case BINDSTATUS_BEGINDOWNLOADDATA:
            hwndShow = GetDlgItem(_hDlg, ulProgressMax ? IDD_PROBAR : IDD_NOFILESIZE);
            if (!IsWindowVisible(hwndShow))
            {
                ShowWindow(GetDlgItem(_hDlg, ulProgressMax ? IDD_NOFILESIZE : IDD_PROBAR), SW_HIDE);
                ShowWindow(hwndShow, SW_SHOW);
            }

            _ulOldProgress = ulProgress;
            // fall thru
        case BINDSTATUS_DOWNLOADINGDATA:
        case BINDSTATUS_ENDDOWNLOADDATA:
            // Prevent machines with APM enabled from suspending during download
            _SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
                
            _dwFileSize = max(ulProgressMax, ulProgress);
            
            // every once in a while, send message
            // to the hidden window that detects inactivity so that it doesn't
            // think we are inactive during a long download
            IndicateWinsockActivity();
            // Sometimes OnProgress is called by folks who do not create a dialog
            if (_hDlg )
            {
                if (!_fStrsLoaded)
                {
                     MLLoadString(IDS_TITLEPERCENT, _szTitlePercent, ARRAYSIZE(_szTitlePercent));
                     MLLoadString(IDS_ESTIMATE, _szEstimateTime, ARRAYSIZE(_szEstimateTime));
                     MLLoadString(IDS_TITLEBYTES, _szTitleBytes, ARRAYSIZE(_szTitleBytes));
                     MLLoadString(IDS_BYTESCOPIED, _szBytesCopied, ARRAYSIZE(_szBytesCopied));
                     MLLoadString(IDS_TRANSFERRATE, _szTransferRate, ARRAYSIZE(_szTransferRate));
                    _fStrsLoaded = TRUE;
                }

                // Get the file name of the file being downloaded
                pszFileName = PathFindFileName(_szURL);

                dwCur = GetTickCount();
                if (_dwOldCur == 0)   // Allow the download to get started before displaying stats
                    _dwOldCur = dwCur;

                if ((ulProgressMax > 0) && _fDownloadStarted)
                {
                    if (_hDlg) 
                    {
                        SendMessage(GetDlgItem(_hDlg, IDD_PROBAR), PBM_SETRANGE32, 0, _dwFileSize);
                        SendMessage(GetDlgItem(_hDlg, IDD_PROBAR), PBM_SETPOS, ulProgress, 0);
                    }

                    if (!_fFirstTickValid) 
                    {
                        _dwFirstSize = ulProgress;
                        _fFirstTickValid = TRUE;

                        SetWindowText(GetDlgItem(_hDlg, IDD_NAME), _szDisplay);
                    } 
                    else
                    {
                        if ((ulProgress - _dwFirstSize) && _hDlg) 
                        {
                            // Recompute and display stats at least every second
                            if ((dwCur - _dwOldCur) >= 1000)
                            {
                                _dwOldCur = dwCur;  // Save current tick count
                                
                                TCHAR szTime[32];
                                DWORD dwSpent = ((dwCur - _dwFirstTick)+500) / 1000;
                                ULONG ulLeft = _dwFileSize - ulProgress;
                                DWORD dwRate = _dwOldRate;
                                dwRate = (ulProgress - _ulOldProgress) / (dwSpent ? dwSpent : 1);

                                TraceMsg(DM_PROGRESS, "OnProgress ulProgress=%d ulGot=%d dwSpent=%d ulLeft=%d", ulProgress, (ulProgress - _dwFirstSize), dwSpent, ulLeft);
                                
                                // Compute & display estimated time left to download, bytes so far, total bytes
                                DWORD dwEst;
                                if (ulLeft > 0x100000L)     // To avoid overflow, use KB for >1MB file.
                                    dwEst = (ulLeft >> 10) / ((dwRate >> 10) ?(dwRate >> 10) :1);
                                else
                                    dwEst = ulLeft / (dwRate ?dwRate :1);
                                    
                                if (dwEst == 0)
                                    dwEst = 1;

                                TraceMsg(DM_PROGRESS, "OnProgress Estimated time left = %d", dwEst);

                                StrFromTimeInterval(szTime, ARRAYSIZE(szTime), dwEst * 1000, 3);
                                LPTSTR pszTime = szTime;
                                while(*pszTime && (*pszTime == ' '))
                                    pszTime++;
                                _FormatMessage(_szEstimateTime, szBuf, ARRAYSIZE(szBuf), pszTime,
                                               StrFormatByteSize(ulProgress, szBytes, MAX_BYTES_STRLEN),
                                               StrFormatByteSize(_dwFileSize, szBytesMax, MAX_BYTES_STRLEN));
                                TraceMsg(DM_PROGRESS, "OnProgress Estimated string = %s", szBuf);
                                SetDlgItemText(_hDlg, IDD_TIMEEST, szBuf);
                                
                                _dwOldEst = dwEst;

                                // Compute & display transfer rate
                                if (dwRate != _dwOldRate)
                                {
                                    _dwOldRate = dwRate;
                                    _FormatMessage(_szTransferRate, szBuf, ARRAYSIZE(szBuf), StrFormatByteSize(dwRate, szBytes, MAX_BYTES_STRLEN));
                                    SetDlgItemText(_hDlg, IDD_TRANSFERRATE, szBuf);
                                }
                            }

                            // Compute & display percentage of download completed
                            DWORD dwPcent = (100 - MulDiv(_dwFileSize - ulProgress, 100, _dwFileSize));
                            if (dwPcent != _dwOldPcent)
                            {
                                _dwOldPcent = dwPcent;
                                if (dwPcent == 100)  // Don't peg the meter until we've completed
                                    dwPcent = 99;
                                    
                                TCHAR szBuf2[MAX_PATH];
                                DWORD dwSize = ARRAYSIZE(szBuf2);
                                if (PrepareURLForDisplay(pszFileName, szBuf2, &dwSize))
                                    _FormatMessage(_szTitlePercent, szBuf, ARRAYSIZE(szBuf), (UINT)dwPcent, szBuf2);
                                else
                                    _FormatMessage(_szTitlePercent, szBuf, ARRAYSIZE(szBuf), (UINT)dwPcent, pszFileName);

                                SetWindowText(_hDlg, szBuf);
                            }
                        }
                    }
                }
                else if (_hDlg && _fDownloadStarted)    // Unknown file size, just show bytes and rate
                {
                    // Recompute and display stats at most every second
                    if ((dwCur - _dwOldCur) >= 1000)
                    {
                        _dwOldCur = dwCur;  // Save current tick count

                        DWORD dwSpent = ((dwCur - _dwFirstTick)+500) / 1000;
                        DWORD dwRate = ulProgress / (dwSpent ? dwSpent : 1);

                        _FormatMessage(_szBytesCopied, szBuf, ARRAYSIZE(szBuf),
                                         StrFormatByteSize(ulProgress, szBytes, MAX_BYTES_STRLEN));
                        TraceMsg(DM_PROGRESS, "OnProgress string = %s", szBuf);
                        SetDlgItemText(_hDlg, IDD_TIMEEST, szBuf);

                        _FormatMessage(_szTransferRate, szBuf, ARRAYSIZE(szBuf), StrFormatByteSize(dwRate, szBytes, MAX_BYTES_STRLEN));
                        SetDlgItemText(_hDlg, IDD_TRANSFERRATE, szBuf);

                        {
                            TCHAR szBuf2[MAX_PATH];
                            DWORD dwSize = ARRAYSIZE(szBuf2);

                            if (PrepareURLForDisplay (pszFileName, szBuf2, &dwSize))
                                _FormatMessage(_szTitleBytes, szBuf, ARRAYSIZE(szBuf),
                                                StrFormatByteSize(ulProgress, szBytes, MAX_BYTES_STRLEN),szBuf2);
                            else
                                _FormatMessage(_szTitleBytes, szBuf, ARRAYSIZE(szBuf),
                                                StrFormatByteSize(ulProgress, szBytes, MAX_BYTES_STRLEN), pszFileName);
                            SetWindowText(_hDlg, szBuf);
                        }
                    }
                }
            }
            break;
        default:    // ulStatusCode
            break;
    }
    return S_OK;
}

HRESULT CDownload::OnStopBinding(HRESULT hrError, LPCWSTR szError)
{
    TraceMsg(DM_DOWNLOAD, "OnStopBinding called with hrError==%x", hrError);

    HRESULT hrDisplay = hrError;
    AddRef(); // Guard against last Release by _RevokeObjectParam

    HRESULT hres = RevokeBindStatusCallback(_pbc, this);
    AssertMsg(SUCCEEDED(hres), TEXT("URLMON bug??? RevokeBindStatusCallback failed %x"), hres);

    if (_pib) 
    {
        CLSID clsid;
        LPWSTR pwszError = NULL;

        HRESULT hresT = _pib->GetBindResult(&clsid, (DWORD *)&hrDisplay, &pwszError, NULL);
        TraceMsg(TF_SHDBINDING, "DLD::OnStopBinding called GetBindResult %x->%x (%x)", hrError, hrDisplay, hresT);
        if (SUCCEEDED(hresT)) 
        {
            //
            // URLMON returns a native Win32 error.
            //
            if (hrDisplay && SUCCEEDED(hrDisplay))
                hrDisplay = HRESULT_FROM_WIN32(hrDisplay);

            if (pwszError)
                OleFree(pwszError);
        }

        // We don't call IBinding::Release until ~CDownload
        // because we need to guarantee the download file
        // exists until we have copied or executed it.
    }

#ifdef DEBUG
    if (hrError==S_OK && GetKeyState(VK_CONTROL) < 0) 
    {
        hrError = E_FAIL;
    }
#endif

    if (FAILED(hrError) && hrError != E_ABORT) 
    {
        IE_ErrorMsgBox(NULL, _hDlg, hrDisplay, szError,_szDisplay, IDS_CANTDOWNLOAD, MB_OK|MB_ICONSTOP);
    }

    if (g_hCritOpMutex != NULL)
    {
        CloseHandle(g_hCritOpMutex);
        g_hCritOpMutex = NULL;
    }

    SetQueryNetSessionCount(SESSION_DECREMENT);
    
    if (!_fGotFile || !_fDownloadCompleted) 
    {
        AssertMsg(FAILED(hrError), TEXT("CDownload::OnStopBinding is called, but we've never got a file -- URLMON bug"));

        if (!_fEndDialogCalled) 
        {
            FORWARD_WM_COMMAND(_hDlg, IDCANCEL, NULL, 0, PostMessage);
        }
    }

    Release(); // Guard against last Release by _RevokeObjectParam
    return S_OK;
}

HRESULT CDownload::GetBindInfo(DWORD* grfBINDINFOF, BINDINFO *pbindinfo)
{
    TraceMsg(DM_DOWNLOAD, "DWNLD::GetBindInfo called when _pbinfo==%x", _pbinfo);

    if ( !grfBINDINFOF || !pbindinfo || !pbindinfo->cbSize )
        return E_INVALIDARG;

    if (_pbinfo) {
        // Give the ownership to URLMON... shallow copy; don't use CopyBindInfo().
        // Don't forget to keep pbindinfo cbSize!
        DWORD cbSize = pbindinfo->cbSize;
        CopyMemory( pbindinfo, _pbinfo, min(_pbinfo->cbSize, cbSize) );
        pbindinfo->cbSize = cbSize;

        if (pbindinfo->cbSize > _pbinfo->cbSize)
        {
            ZeroMemory((BYTE *)pbindinfo + _pbinfo->cbSize, pbindinfo->cbSize - _pbinfo->cbSize);
        }

        LocalFree(_pbinfo);
        _pbinfo = NULL;

    } else {
        // We don't have a BINDINFO our selves so
        // clear BINDINFO except cbSize
        DWORD cbSize = pbindinfo->cbSize;
        ZeroMemory( pbindinfo, cbSize );
        pbindinfo->cbSize = cbSize;
        if (UTF8Enabled())
            pbindinfo->dwOptions = BINDINFO_OPTIONS_ENABLE_UTF8;
    }

    // #52524. With post build ~1100, If we do not return the following flags when URLMon calls
    // GetBindInfo(), It will bind to the storage synchronously. (judej, danpoz)
    *grfBINDINFOF = _grfBINDF | BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA;
    return S_OK;
}

HRESULT CDownload::OnDataAvailable(DWORD grfBSC, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed)
{
    DWORD dwOptions = 0;

    DWNLDMSG3("OnDataAvailable (grf,pstg)", grfBSC, pstgmed);

    _dwTotalSize = dwSize; // keep track of number of bytes downloaded
    
    if (SUCCEEDED(_GetRequestFlagFromPIB(_pib, &dwOptions)) && (dwOptions & INTERNET_REQFLAG_CACHE_WRITE_DISABLED)) 
    {
        _fWriteHistory = FALSE;
    }
    
    if (grfBSC & BSCF_LASTDATANOTIFICATION) 
    {
        _fDownloadCompleted = TRUE;
    }
    //
    //   This code gets the file name from pstgmed, when it became
    //  available. URLMon is supposed to pass it even though the file
    //  is not completely ready yet.
    //
    if (!_fGotFile && pstgmed) 
    {
        Animate_Stop(GetDlgItem(_hDlg, IDD_ANIMATE));
        if (pstgmed->tymed == TYMED_FILE) 
        {
            TCHAR szBuf[MAX_PATH];  // ok with MAX_PATH (because we truncate)

            SHUnicodeToTChar(pstgmed->lpszFileName, _szPath, ARRAYSIZE(_szPath));

            // Because of redirection the _szURL could be http://.../redir.dll or query.exe.
            // Whereas the actual filename would be something else. The Cache filename is generated
            // by wininet after it has figured out what the real filename is. However, it might contain
            // a "(1)" or a "(2)" at the end of the file name.

            TCHAR szURL[MAX_URL_STRING];

            StrCpyN(szURL, _szURL, ARRAYSIZE(szURL));

            TCHAR * pszURLFName = PathFindFileName(szURL);
            TCHAR * pszCacheFName = PathFindFileName(_szPath);

            // Unescape the filename suggested by wininet.
            DWORD cch = ARRAYSIZE(szBuf);
            if (_PrepareURLForDisplayUTF8W(pszCacheFName, szBuf, &cch, _fUTF8Enabled, _uiCP) != S_OK)
                StrCpyN(szBuf, pszCacheFName, ARRAYSIZE(szBuf));


            // Strip out any path that may have been encoded
            pszCacheFName = szBuf;
            TCHAR *pszSrc = PathFindFileName(szBuf);
            if (pszSrc != szBuf)
            {
                while(*pszSrc)
                    *pszCacheFName++ = *pszSrc++;
                *pszCacheFName = *pszSrc;
            }

            // Use the Cache name. pszURLFName point to the file name in szURL. Just overwrite it
            if (pszURLFName && szBuf)
            {
                *pszURLFName = 0;
                StringCchCat(szURL, ARRAYSIZE(szURL), szBuf);

                FormatUrlForDisplay(szURL, _szDisplay, ARRAYSIZE(_szDisplay), NULL, 0, TRUE, _uiCP, NULL);
            }

            DWNLDMSG("OnDataAvailable got TYMED_FILE", _szPath);
            _fGotFile = TRUE;

            TCHAR szMime[MAX_PATH];
            if (GetClipboardFormatName(pformatetc->cfFormat, szMime, sizeof(szMime)/sizeof(szMime[0])))
            {
                MIME_GetExtension(szMime, (LPTSTR) _szExt, SIZECHARS(_szExt));
            }

            SetWindowText(GetDlgItem(_hDlg, IDD_NAME), _szDisplay);

            UINT uRet = _MayAskUserIsFileSafeToOpen(szMime);
            switch (uRet) {
            case IDOK:
                MLLoadString(IDS_OPENING, szBuf, ARRAYSIZE(szBuf));
                break;

            case IDD_SAVEAS:
                _fSaveAs = TRUE;
                _fCallVerifyTrust = FALSE;
                MLLoadString(IDS_SAVING, szBuf, ARRAYSIZE(szBuf));
                break;

            case IDCANCEL:
                FORWARD_WM_COMMAND(_hDlg, IDCANCEL, NULL, 0, PostMessage);

                //
                // HACK: Under a certain condition, we receive one more
                //  OnDataAvailable from URLMON with BSCF_LASTDATANOTIFICATION
                //  before this posted message is dispatched. It causes
                //  WinVerifyTrust call, which is wrong. To prevent it,
                //  we unset this flag.
                //
                //  We still assumes that OnStopBinding will not happen before
                //  this message is dispatched. In IE 4.0, we should introduce
                //  another flag (_fCancelled) to make it more robust.
                //
                _fCallVerifyTrust = FALSE;
                return S_OK;

            }

            SetDlgItemText(_hDlg, IDD_OPENIT, szBuf);

            if (_fSaveAs)
            {
                if (!_GetSaveLocation())
                {
                    FORWARD_WM_COMMAND(_hDlg, IDCANCEL, NULL, 0, PostMessage);
                    return S_OK;
                }
                StrCpyN(szBuf, _szSaveToFile, ARRAYSIZE(szBuf));

                RECT rect;
                GetClientRect(GetDlgItem(_hDlg, IDD_DIR), &rect);
                PathCompactPath(NULL, szBuf, rect.right);
            }
            else
                MLLoadString(IDS_DOWNLOADTOCACHE, szBuf, ARRAYSIZE(szBuf));

            SetDlgItemText(_hDlg, IDD_DIR, szBuf);
            Animate_Play(GetDlgItem(_hDlg, IDD_ANIMATE),0, -1, -1);
            
            if (_dwFirstTick == 0)   // Start the timer
                _dwFirstTick = GetTickCount();
        }
        else
        {
            TraceMsg(DM_WARNING, "CDownload::OnDataAvailable pstgmed->tymed (%d) != TYMED_FILE", pstgmed->tymed);
        }
        _fDownloadStarted = TRUE;
    }

    if (_fDownloadCompleted) 
    {
#ifdef CALL_WVT
        if (_fCallVerifyTrust)
        {
            ShowWindow(_hDlg, SW_HIDE);
            UINT uRet = _VerifyTrust(_hDlg, _szPath, _szDisplay);
            switch (uRet) {
            case IDOK:
                break;

            default:
                // We assume _VerifyTrust always is able to open the file
                // passed from URLMON. If it fails, we bail with no UI.
                ASSERT(0);
                // Fall through
            case IDCANCEL:
                _fDeleteFromCache = TRUE;
                FORWARD_WM_COMMAND(_hDlg, IDCANCEL, NULL, 0, PostMessage);
                return S_OK;
            }
        }
#endif // CALL_WVT

        DWNLDMSG3("OnDataAvailable calling Animate_Stop", _hDlg, GetDlgItem(_hDlg, IDD_ANIMATE));
        Animate_Stop(GetDlgItem(_hDlg, IDD_ANIMATE));

        SendMessage(GetDlgItem(_hDlg, IDD_PROBAR), PBM_SETRANGE32, 0, 100);
        SendMessage(GetDlgItem(_hDlg, IDD_PROBAR), PBM_SETPOS, 100, 0);

        if (_fSaveAs) {
            FORWARD_WM_COMMAND(_hDlg, IDD_SAVEAS, NULL, 0, PostMessage);
        } else {
#ifdef USE_LOCKREQUEST
            LockRequestHandle();  // Tell wininet that we want the file locked to allow the app to open it.
                                  // This prevents wininet from deleting the file from the cache before the
                                  // app gets a chance to use it.  When wininet sees this file is locked, it
                                  // will add the file to the scavenger leak list and attempt to delete the
                                  // file in the future.
#endif
            
            FORWARD_WM_COMMAND(_hDlg, IDOK, NULL, 0, PostMessage);
        }
    }
    return S_OK;
}

HRESULT CDownload::OnObjectAvailable(REFIID riid, IUnknown *punk)
{
    DWORD dwOptions = 0;

    DWNLDMSG3("OnObjectAvailable (riid,punk)", riid, punk);

    if (SUCCEEDED(_GetRequestFlagFromPIB(_pib, &dwOptions)) && (dwOptions & INTERNET_REQFLAG_CACHE_WRITE_DISABLED)) 
    {
        _fWriteHistory = FALSE;
    }

    return S_OK;
}

/* *** IHttpNegotiate ***  */
HRESULT CDownload::BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders,
        DWORD dwReserved, LPWSTR *pszAdditionalHeaders)
{
    if ((!_pwzHeaders) || (!pszAdditionalHeaders))
        return S_OK;

    DWORD cbHeaders = (lstrlenW(_pwzHeaders) + 1) * sizeof(WCHAR);
    LPWSTR pwzHeaders = (LPWSTR)CoTaskMemAlloc(cbHeaders + sizeof(WCHAR));

    if (pwzHeaders)
    {
        memcpy (pwzHeaders, _pwzHeaders, cbHeaders);
        *pszAdditionalHeaders = pwzHeaders;
    }
    // Caller owns freeing *pszAdditionalHeaders
    return S_OK;
}

HRESULT CDownload::OnResponse(DWORD dwResponseCode, LPCWSTR szResponseHeaders, 
                              LPCWSTR szRequestHeaders, LPWSTR *pszAdditionalRequestHeaders)
{
    return S_OK;
}


BOOL _RememberFileIsSafeToOpen(LPCTSTR szFileClass)
{
    DWORD dwEditFlags;
    ULONG cb = sizeof(dwEditFlags);

    HRESULT hr;
    IQueryAssociations *passoc = NULL;
    BOOL bRet = FALSE;

    hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &passoc));
    if (SUCCEEDED(hr) && passoc)
    {
        hr = passoc->Init(NULL, szFileClass, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            hr = passoc->GetData(NULL, ASSOCDATA_EDITFLAGS, NULL, &dwEditFlags, &cb);
            if (SUCCEEDED(hr))
            {
                dwEditFlags &= ~FTA_NoEdit;
                dwEditFlags |= FTA_OpenIsSafe;
            }
        }
        passoc->Release();
    }

    if (FAILED(hr))
        dwEditFlags = FTA_OpenIsSafe;   

    return (SHSetValue(HKEY_CLASSES_ROOT, szFileClass, TEXT("EditFlags"),
                             REG_BINARY, (BYTE*)&dwEditFlags,
                             sizeof(dwEditFlags)) == ERROR_SUCCESS);
}


struct SAFEOPENDLGPARAM {
    LPCTSTR pszFileClass;
    LPCTSTR pszFriendlyURL;
    LPCTSTR pszURL;
    HWND    hwndTT;
    TCHAR*  pszTTText;
    LPCTSTR pszCacheFile;
    DWORD   uiCP;
    BOOL    fTypeMismatch;
    BOOL    fShellExecPrompt;
    BOOL    fPackagerCommandPrompt;
};

INT_PTR CALLBACK SafeOpenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UINT id;
    static BOOL bCancelled;
    SAFEOPENDLGPARAM* param = (SAFEOPENDLGPARAM*) GetWindowLongPtr(hDlg, DWLP_USER);

    if ((param == NULL) && (uMsg != WM_INITDIALOG))
        return FALSE;
        
    switch (uMsg) {
    case WM_INITDIALOG:
    {
        BOOL fUnsafeFile;
        TCHAR szFriendlyName[MAX_DISPLAY_LEN] = {TEXT('\0')};
        TCHAR szFriendlyFrom[MAX_DISPLAY_LEN] = {TEXT('\0')};
        TCHAR szProcessedURL[MAX_URL_STRING] = {TEXT('\0')};
        DWORD dwSize = ARRAYSIZE(szProcessedURL);

        if (lParam == NULL)
            return FALSE;
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        param = (SAFEOPENDLGPARAM*)lParam;

        // init unsafe file to mismatch between progid and file
        fUnsafeFile = param->fTypeMismatch;

        // Determine whether or not to gray out the Always ask checkbox. We wil gray out in the following cases
        // 1. If we were not told what the file class is
        // 2. If the file class is in the unsafe extensions list
        // 3. if the file extension in the URL is in the unsafe extensions list
        // 4. if the cache file extension is in the unsafe extensions list (if we are redirected)
        TCHAR * pszExt = NULL;
        TCHAR * pszCacheExt = NULL;

        if (param->pszURL)
            pszExt = PathFindExtension(param->pszURL);

        if (param->pszCacheFile)
            pszCacheExt = PathFindExtension(param->pszCacheFile);

        if(param->fPackagerCommandPrompt)
        {
            fUnsafeFile = TRUE;
        }
        else if (pszExt || pszCacheExt)
        {
            if (pszExt && AssocIsDangerous(pszExt))
                fUnsafeFile = TRUE;
            else if (pszCacheExt && AssocIsDangerous(pszCacheExt))
                fUnsafeFile = TRUE;
        }
        else
        {
            fUnsafeFile = TRUE;
        }
        
        if (fUnsafeFile || SHRestricted2(REST_AlwaysPromptWhenDownload, NULL, 0))
            EnableWindow(GetDlgItem(hDlg, IDC_SAFEOPEN_ALWAYS), FALSE);
        // The check box is always checked by default
        CheckDlgButton(hDlg, IDC_SAFEOPEN_ALWAYS, TRUE);
        
        // adjust the warning
        if (fUnsafeFile)
        {
            HICON hIcon = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_PRIVACY_WARN),
                            IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
            if (hIcon != NULL)
                SendDlgItemMessage(hDlg, IDC_SAFEOPEN_WARNICON, STM_SETICON, (WPARAM)hIcon, 0);
        }
        else
        {
            ShowWindow(GetDlgItem(hDlg, IDC_SAFEOPEN_WARNTEXT), SW_HIDE);
        }

        // cross-lang platform support
        SHSetDefaultDialogFont(hDlg, IDC_SAFEOPEN_FILENAME);
        SHSetDefaultDialogFont(hDlg, IDC_SAFEOPEN_FILETYPE);
        SHSetDefaultDialogFont(hDlg, IDC_SAFEOPEN_FILEFROM);
        
        // Get the URL for the tooltip. Also get URL for the display string if we weren't passed one
        if (param->pszURL)
        {
            if (!PrepareURLForDisplay(param->pszURL, szProcessedURL, &dwSize))
            {
                dwSize = ARRAYSIZE(szProcessedURL);
                StrCpyN(szProcessedURL, param->pszURL, dwSize);
            }
        }

        // Now figure out what we want to display
        if(param->fPackagerCommandPrompt)
        {
            // If this was a packager command line, then just display the full command as passed in param->pszURL
            StrCpyN(szFriendlyName, param->pszURL, ARRAYSIZE(szFriendlyName));
        }
        else
        {
            FormatUrlForDisplay((LPTSTR)param->pszURL, szFriendlyName, ARRAYSIZE(szFriendlyName), szFriendlyFrom, ARRAYSIZE(szFriendlyFrom),
                            TRUE, param->uiCP, (PWSTR)param->pszCacheFile);
        }

        SetDlgItemText(hDlg, IDC_SAFEOPEN_FILENAME, szFriendlyName);

        if(param->fPackagerCommandPrompt)
        {
            // If it was a packager command line, then display "Unknown" for the from
            MLLoadString(IDS_VALUE_UNKNOWN, szFriendlyFrom, ARRAYSIZE(szFriendlyFrom)); 
            SetDlgItemText(hDlg, IDC_SAFEOPEN_FILEFROM, szFriendlyFrom);
        }
        else
        {
            if (szFriendlyFrom[0] != '\0')
                SetDlgItemText(hDlg, IDC_SAFEOPEN_FILEFROM, szFriendlyFrom);
        }

        if (param->pszFileClass || pszCacheExt)
        {
            DWORD cchName = ARRAYSIZE(szFriendlyName);

            if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_FRIENDLYDOCNAME,
                (param->pszFileClass ? param->pszFileClass : pszCacheExt), NULL, szFriendlyName, &cchName)))
            {
                SetDlgItemText(hDlg, IDC_SAFEOPEN_FILETYPE, szFriendlyName);
            }
        }

        int cch = lstrlen(szProcessedURL) + 1;
        param->pszTTText = (TCHAR*)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        if (param->pszTTText)
        {
            StrCpyN(param->pszTTText, szProcessedURL, cch);
            if (param->hwndTT = CreateWindow(TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP,
                                      CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
                                      hDlg, NULL, HINST_THISDLL, NULL))
            {
                TOOLINFO ti;

                ti.cbSize = sizeof(ti);
                ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
                ti.hwnd = hDlg;
                ti.uId = (UINT_PTR) GetDlgItem(hDlg, IDC_SAFEOPEN_FILENAME);
                ti.lpszText = LPSTR_TEXTCALLBACK;
                ti.hinst = HINST_THISDLL;
                GetWindowRect((HWND)ti.uId, &ti.rect);
                SendMessage(param->hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
                SendMessage(param->hwndTT, TTM_SETMAXTIPWIDTH, 0, 300);
            }
        }

        if (param->fShellExecPrompt)
        {
            EnableWindow(GetDlgItem(hDlg, IDC_SAFEOPEN_AUTOSAVE), FALSE);
            // make Cancel the default action
            SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDCANCEL), TRUE);
        }
        else
        {
            // make Save the default action
            SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDC_SAFEOPEN_AUTOSAVE), TRUE);
        }
        return FALSE;
    }

    case WM_NOTIFY:
    {
        LPTOOLTIPTEXT lpTT = (LPTOOLTIPTEXT) lParam;
        if (lpTT->hdr.code == TTN_NEEDTEXT)
        {
            lpTT->lpszText = param->pszTTText;
            lpTT->hinst = NULL;
        }
        break;
    }

   case WM_DESTROY:

        //deal with checkbox
        if ((!bCancelled) &&  (!IsDlgButtonChecked(hDlg, IDC_SAFEOPEN_ALWAYS)) && param->pszURL)
        {
            // Now save EditFlags at the key value value that the filetypes dialog will get/set.
            TCHAR * pszExt = PathFindExtension(param->pszURL);
            if (*pszExt)
            {
                TCHAR szFileClass[MAX_PATH];
                ULONG cb = sizeof(szFileClass);
                *szFileClass = TEXT('\0');
                SHGetValue(HKEY_CLASSES_ROOT, pszExt, NULL, NULL, szFileClass, &cb);
                if (*szFileClass)
                    _RememberFileIsSafeToOpen(szFileClass);
            }
        }

        SHRemoveDefaultDialogFont(hDlg);

        if (IsWindow(param->hwndTT))
            DestroyWindow(param->hwndTT);

        if (param->pszTTText)
        {
            LocalFree(param->pszTTText);
            param->pszTTText = NULL;
        }

        return FALSE;

    case WM_COMMAND:
        id = GET_WM_COMMAND_ID(wParam, lParam);
        switch (id) 
        {
        case IDC_SAFEOPEN_AUTOOPEN:
            EndDialog(hDlg, IDOK);
            break;

        case IDC_SAFEOPEN_AUTOSAVE:
            EndDialog(hDlg, IDD_SAVEAS);
            break;
        
        case IDM_MOREINFO:
            SHHtmlHelpOnDemandWrap(hDlg, TEXT("iexplore.chm > iedefault"), HH_DISPLAY_TOPIC, (DWORD_PTR) TEXT("filedown.htm"), ML_CROSSCODEPAGE);
            break;

        case IDCANCEL:
            bCancelled = TRUE;
            EndDialog(hDlg, id);
            break;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

UINT OpenSafeOpenDialog(HWND hwnd, UINT idRes, LPCTSTR pszFileClass, LPCTSTR pszURL, LPCTSTR pszRedirURL, LPCTSTR pszCacheName, LPCTSTR pszDisplay, UINT uiCP, IUnknown *punk, BOOL fTypeMismatch)
{
    UINT uRet;
    IDownloadManager *pdlm;
    HRESULT hr = IUnknown_QueryService(punk, SID_SDownloadManager, IID_PPV_ARG(IDownloadManager, &pdlm));
    if (SUCCEEDED(hr))
    {
        pdlm->Release();
        return IDD_SAVEAS;
    }
    
    LPCTSTR pszTemp = pszURL;
    if (pszRedirURL && lstrlen(pszRedirURL))
        pszTemp = pszRedirURL;

    SAFEOPENDLGPARAM param = { pszFileClass, pszDisplay, pszTemp, 0, 0, pszCacheName, uiCP, fTypeMismatch, FALSE, FALSE};
    uRet = (UINT) DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(idRes), hwnd, SafeOpenDlgProc, (LPARAM)&param);
    return uRet;
}

STDAPI_(BOOL) SafeOpenPromptForShellExec(HWND hwnd, PCWSTR pszFile)
{
    SAFEOPENDLGPARAM param = { PathFindExtension(pszFile), NULL, pszFile, 0, 0, NULL, CP_ACP, TRUE, TRUE, FALSE};
    return IDOK == (UINT) DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(DLG_SAFEOPEN), hwnd, SafeOpenDlgProc, (LPARAM)&param);
}

STDAPI_(BOOL) SafeOpenPromptForPackager(HWND hwnd, PCWSTR pszFile, BOOL bFromCommandLine)
{
    SAFEOPENDLGPARAM param = { PathFindExtension(pszFile), NULL, pszFile, 0, 0, NULL, CP_ACP, TRUE, TRUE, bFromCommandLine};
    return IDOK == (UINT) DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(DLG_SAFEOPEN), hwnd, SafeOpenDlgProc, (LPARAM)&param);
}

BOOL _OpenIsSafe(LPCTSTR pszClass)
{
    BOOL bRet = FALSE;
    IQueryAssociations *passoc;
    HRESULT hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &passoc));
    if (SUCCEEDED(hr))
    {
        hr = passoc->Init(NULL, pszClass, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            DWORD dwEditFlags;
            ULONG cb = sizeof(dwEditFlags);
            hr = passoc->GetData(NULL, ASSOCDATA_EDITFLAGS, NULL, &dwEditFlags, &cb);
            if (SUCCEEDED(hr))
            {
                if (dwEditFlags & FTA_OpenIsSafe)
                    bRet = TRUE;
            }
        }
        passoc->Release();
    }
    return bRet;
}

UINT MayOpenSafeOpenDialog(HWND       hwnd,
                           LPCTSTR    pszFileClass,
                           LPCTSTR    pszURL,
                           LPCTSTR    pszCacheName,
                           LPCTSTR    pszDisplay,
                           UINT       uiCP,
                           IUnknown * punk,
                           IOleCommandTarget * pCmdTarget = NULL,
                           BOOL       fDisableOpen = FALSE)
{
    // Has some association
    UINT uiRet = IDIGNORE;  // default for no dlg displayed
    const LPCTSTR c_szExcluded[] = {TEXT(".ins"),TEXT(".isp")};
    const LPCTSTR c_szNoZoneCheckExtns[] = {TEXT(".cdf")};

    BOOL fSafe = _OpenIsSafe(pszFileClass);

    // We will not do Zone check on CDF files..#56297.
    if (!IsTypeInList(pszFileClass, c_szNoZoneCheckExtns, ARRAYSIZE(c_szNoZoneCheckExtns)))
    {
        DWORD dwPolicy = 0, dwContext = 0;
        ZoneCheckUrlEx(pszURL, &dwPolicy, sizeof(dwPolicy), &dwContext, sizeof(dwContext),
                    URLACTION_SHELL_FILE_DOWNLOAD, PUAF_NOUI, NULL);
        dwPolicy = GetUrlPolicyPermissions(dwPolicy);
        if ((dwPolicy != URLPOLICY_ALLOW) && (dwPolicy != URLPOLICY_QUERY))
        {
            ProcessStartbindingError(hwnd, NULL, NULL, MB_ICONWARNING, E_ACCESSDENIED, pszURL);
            return IDCANCEL;
        }
    }


    // Always ask for certain the types that we know to be unsafe. We will allow .ins and .isp
    // files through for the ICW folks.
    if (AssocIsDangerous(pszFileClass) &&
        !IsTypeInList(pszFileClass, c_szExcluded, ARRAYSIZE(c_szExcluded)))
        fSafe = FALSE;

    if (!fSafe || SHRestricted2(REST_AlwaysPromptWhenDownload, NULL,0))
    {
        VARIANT varOut = {0};

        if (pCmdTarget)
        {
            pCmdTarget->Exec(&CGID_ShellDocView, SHDVID_FIREFILEDOWNLOAD, 0, NULL, &varOut);
        }

        if ((V_VT(&varOut) != VT_BOOL) || (VARIANT_FALSE == V_BOOL(&varOut)))
        { 
            uiRet = OpenSafeOpenDialog(hwnd,
                                       DLG_SAFEOPEN,
                                       pszFileClass,
                                       pszURL,
                                       NULL,
                                       pszCacheName,
                                       pszDisplay,
                                       uiCP,
                                       punk,
                                       fDisableOpen);
        }
    }

    if (uiRet != IDOK && uiRet != IDD_SAVEAS && uiRet != IDIGNORE)
        DeleteUrlCacheEntry(pszURL);

    return(uiRet);
}

#ifdef CALL_WVT
// Returns:
//
//  IDOK     -- If it's trusted
//  IDNO     -- If it's not known (warning dialog requried)
//  IDCANCEL -- We need to stop download it
//
UINT _VerifyTrust(HWND hwnd, LPCTSTR pszFileName, LPCWSTR pszStatusText)
{
    UINT uRet = IDNO; // assume unknown
    HANDLE hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ,
                    NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        HRESULT hres =  g_wvt.VerifyTrust(hFile, hwnd, pszStatusText);
        if (SUCCEEDED(hres)) 
        {
            uRet = IDOK;
        } 
        else 
        {
            ASSERT((hres != HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND)) &&
                   (hres != HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND)));

            uRet = IDCANCEL;
        }

        CloseHandle(hFile);
    } 
    else
    {
        TraceMsg(DM_WARNING, "_VerifyTrust CreateFile failed %x", GetLastError());
    }

    TraceMsg(DM_WVT, "_VerifyTrust returning %d", uRet);
    return uRet;
}
#endif // CALL_WVT

//
// Returns:
//  IDOK: Continue download and open it
//  IDD_SAVEAS: Save it as a file
//  IDCANCEL: Stop downloading
//
UINT CDownload::_MayAskUserIsFileSafeToOpen(LPCTSTR pszMime)
{
    if (_fSaveAs || _fSafe) 
    {
        return (_fSaveAs ? IDD_SAVEAS : IDOK);    // no need to ask
    }

    // Force save as dialog if we are using SSL and 
    // HKCU\software\microsoft\windows\currentversion\internet settings\DisableCachingOfSSLPages is set
    DWORD dwValue;
    DWORD dwDefault = 0;
    DWORD dwSize;
    dwSize = sizeof(dwValue);
    SHRegGetUSValue(TSZWININETPATH, TEXT("DisableCachingOfSSLPages"), NULL, (LPBYTE)&dwValue, &dwSize, FALSE, (void *) &dwDefault, sizeof(dwDefault));

    if (dwValue != 0 && URL_SCHEME_HTTPS == GetUrlScheme(_szURL))
    {
        return(IDD_SAVEAS);
    }

    if(_fConfirmed)
    {
        return IDOK;
    }

    BOOL fUnknownType = TRUE;
    UINT uRet = IDNO;   // assume no extension or no association
    LPTSTR pszExt = PathFindExtension(_szPath);

    if (*pszExt) 
    {
        TCHAR szFileClass[MAX_PATH];
        memset(szFileClass, 0, ARRAYSIZE(szFileClass));
#ifdef CALL_WVT
        //
        //  If this is an EXE and we have WINTRUST ready to call,
        // don't popup any UI here at this point.
        if ((StrCmpI(pszExt, TEXT(".exe"))==0) && SUCCEEDED(g_wvt.Init()))
        {
            TraceMsg(DM_WVT, "_MayAskUIFSTO this is EXE, we call _VerifyTrust later");
            _fCallVerifyTrust = TRUE;
        }
#endif // CALL_WVT

        ULONG cb = SIZEOF(szFileClass);
        if ((RegQueryValue(HKEY_CLASSES_ROOT, pszExt, szFileClass, (LONG*)&cb)
                == ERROR_SUCCESS) && * szFileClass)
        {
            fUnknownType = FALSE;
            uRet = MayOpenSafeOpenDialog(_hDlg, szFileClass, _szURL, _szPath, _szDisplay, _uiCP, NULL, NULL, FALSE);
            if (uRet == IDIGNORE)   // caller doesn't recognize IDIGNORE
                uRet = IDOK;
        }
    }

    if (fUnknownType) 
    {
        uRet = OpenSafeOpenDialog(_hDlg, DLG_SAFEOPEN, NULL, _szURL, NULL, _szPath, _szDisplay, _uiCP, NULL, FALSE);
    }

    return uRet;
}

// *** IAuthenticate ***
HRESULT CDownload::Authenticate(HWND *phwnd, LPWSTR *pszUsername, LPWSTR *pszPassword)
{
    if (!phwnd || !pszUsername || !pszPassword)
        return E_POINTER;

    *phwnd = _hDlg;
    *pszUsername = NULL;
    *pszPassword = NULL;
    return S_OK;
}

HRESULT CDownload::GetWindow(REFGUID RefGUID, HWND *phWnd)
{
    if (IsEqualGUID(RefGUID, IID_IHttpSecurity))
    {
        *phWnd = _hDlg;
        return S_OK;
    }
    else
        return E_FAIL;
}

// *** IServiceProvider ***
HRESULT CDownload::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualGUID(guidService, IID_IAuthenticate))
    {
        return QueryInterface(riid, ppvObj);
    }
    return E_FAIL;
}

//   S_OK : continue with operation
//   S_FALSE : cancel operation.

HRESULT CDownload::PerformVirusScan(LPCTSTR szFileName)
{
    HRESULT hr = S_OK;  // default to accepting the file

    IVirusScanner *pvs;
    if (SUCCEEDED(CreateFromRegKey(TSZIEPATH, TEXT("VirusScanner"), IID_PPV_ARG(IVirusScanner, &pvs))))
    {
        STGMEDIUM stg;
        WCHAR wszFileName[MAX_PATH];

        VIRUSINFO vi;
        vi.cbSize = sizeof(VIRUSINFO);

        //
        // VIRUSINFO lpszFileName is not defined as 'const' so we need to copy
        // szFileName into a buffer.  If it really should be const get rid of
        // this copy and use a cast.
        //
        StrCpyN(wszFileName, szFileName, ARRAYSIZE(wszFileName));
        stg.tymed = TYMED_FILE;
        stg.lpszFileName = wszFileName;
        stg.pUnkForRelease = NULL;

        hr = pvs->ScanForVirus(_hDlg, &stg, _pwszDisplayName, SFV_DELETE, &vi);

        switch (hr) {

        case S_OK:
            break;

        case VSCAN_E_NOPROVIDERS:   //No virus scanning providers
        case VSCAN_E_CHECKPARTIAL:  //Atleast one of providers didn't work.
        case VSCAN_E_CHECKFAIL:     //No providers worked.
            hr = S_OK;
            break;

        case VSCAN_E_DELETEFAIL:    //Tried to delete virus file but failed.
        case S_FALSE:               // Virus found.
            hr = E_FAIL;
            break;

        // If some bizarre result, continue on.
        default:
            hr = S_OK;
            break;
        }

        pvs->Release();
    }

    return hr;
}

// Starts a download of a file in its own window.

// This function is exported and called by HTML doc object.
// Someday we probably want to put this in a COM interface.
// Currently it just calls the internal function CDownLoad_OpenUIURL.

STDAPI DoFileDownload(LPCWSTR pwszURL)
{
    return CDownLoad_OpenUIURL(pwszURL, NULL, NULL, FALSE,TRUE);
}

STDAPI DoFileDownloadEx(LPCWSTR pwszURL, BOOL fSaveAs)
{
    return CDownLoad_OpenUIURL(pwszURL, NULL, NULL, FALSE, fSaveAs);
}

#ifdef DEBUG
const DWORD CDownloadThreadParam::s_dummy = 0;
#endif
