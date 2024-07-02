/*****************************************************************************\
    FILE: util.h

    DESCRIPTION:
        Shared stuff that operates on all classes.

    BryanSt 8/13/1999
    Copyright (C) Microsoft Corp 1999-1999. All rights reserved.
\*****************************************************************************/

#ifndef _UTIL_H
#define _UTIL_H

#include "regutil.h"

#define HINST_THISDLL       g_hinst

extern BOOL g_fInSetup;

enum enumTSPerfFlag
{
    TSPerFlag_NoADWallpaper = 0,
    TSPerFlag_NoWallpaper,
    TSPerFlag_NoVisualStyles,
    TSPerFlag_NoWindowDrag,
    TSPerFlag_NoAnimation,
};

// String Helpers
HRESULT HrSysAllocStringW(IN const OLECHAR * pwzSource, OUT BSTR * pbstrDest);
HRESULT HrSysAllocString(IN const OLECHAR * pwzSource, OUT BSTR * pbstrDest);
void PathUnExpandEnvStringsWrap(LPTSTR pszString, DWORD cchSize);
void PathExpandEnvStringsWrap(LPTSTR pszString, DWORD cchSize);

#define SysAllocStringT(pszString)    SysAllocString(pszString)

HRESULT StrReplaceToken(IN LPCTSTR pszToken, IN LPCTSTR pszReplaceValue, IN LPTSTR pszString, IN DWORD cchSize);
HRESULT HrWritePrivateProfileStringW(LPCWSTR pszAppName, LPCWSTR pszKeyName, LPCWSTR pszString, LPCWSTR pszFileName);

// File System Helpers
HRESULT HrSHFileOpDeleteFile(HWND hwnd, FILEOP_FLAGS dwFlags, LPTSTR pszPath);

// Registry Helpers
HRESULT HrRegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
HRESULT HrRegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, 
       REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
HRESULT HrRegQueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
HRESULT HrRegSetValueEx(IN HKEY hKey, IN LPCTSTR lpValueName, IN DWORD dwReserved, IN DWORD dwType, IN CONST BYTE *lpData, IN DWORD cbData);
HRESULT HrRegEnumKey(HKEY hKey, DWORD dwIndex, LPTSTR lpName, DWORD cbName);
HRESULT HrRegEnumValue(HKEY hKey, DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcValueName, LPDWORD lpReserved,
        LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
HRESULT HrRegQueryInfoKey(HKEY hKey, LPTSTR lpClass, LPDWORD lpcClass, LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcMaxSubKeyLen, 
            LPDWORD lpcMaxClassLen, LPDWORD lpcValues, LPDWORD lpcMaxValueNameLen, LPDWORD lpcMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime);

HRESULT HrBStrRegQueryValue(IN HKEY hKey, IN LPCTSTR lpValueName, OUT BSTR * pbstr);
HRESULT HrSHGetValue(IN HKEY hKey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, OPTIONAL OUT LPDWORD pdwType, OPTIONAL OUT LPVOID pvData, OPTIONAL OUT LPDWORD pcbData);
HRESULT HrSHSetValue(IN HKEY hkey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, DWORD dwType, OPTIONAL OUT LPVOID pvData, IN DWORD cbData);
DWORD HrRegGetDWORD(HKEY hKey, LPCWSTR szKey, LPCWSTR szValue, DWORD dwDefault);
HRESULT HrRegSetDWORD(HKEY hKey, LPCWSTR szKey, LPCWSTR szValue, DWORD dwData);
HRESULT HrRegDeleteValue(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName);

HRESULT HrRegSetValueString(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValueName, LPCWSTR pszString);
HRESULT HrRegGetValueString(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValueName, LPWSTR pszString, DWORD cchSize);
HRESULT HrRegSetPath(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName, BOOL fUseExpandSZ, LPCWSTR pszPath);
HRESULT HrRegGetPath(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName, IN LPWSTR pszPath, IN DWORD cchSize);


// Palette Helpers
COLORREF GetNearestPaletteColor(HPALETTE hpal, COLORREF rgb);
BOOL IsPaletteColor(HPALETTE hpal, COLORREF rgb);


// Theme Specific Helpers
HRESULT SHGetResourcePath(BOOL fLocaleNode, IN LPWSTR pszPath, IN DWORD cchSize);
HRESULT ExpandResourceDir(IN LPWSTR pszPath, IN DWORD cchSize);
HRESULT GetCurrentUserCustomName(LPWSTR pszDisplayName, DWORD cchSize);
HRESULT InstallVisualStyle(IThemeManager * pThemeManager, LPCTSTR pszVisualStylePath, LPCTSTR pszVisualStyleColor, LPCTSTR pszVisualStyleSize);
HRESULT ApplyVisualStyle(LPCTSTR pszVisualStylePath, LPCTSTR pszVisualStyleColor, LPCTSTR pszVisualStyleSize);


// IProperty Bag Helpers
STDAPI SHPropertyBag_WritePunk(IN IPropertyBag * ppb, IN LPCWSTR pwzPropName, IN IUnknown * punk);
STDAPI SHPropertyBag_ReadByRef(IN IPropertyBag * ppb, IN LPCWSTR pwzPropName, IN void * p, IN SIZE_T cbSize);
STDAPI SHPropertyBag_WriteByRef(IN IPropertyBag * ppb, IN LPCWSTR pwzPropName, IN void * p);
HRESULT GetPageByCLSID(IUnknown * punkSite, const GUID * pClsid, IPropertyBag ** ppPropertyBag);
HRESULT IEnumUnknown_FindCLSID(IN IUnknown * punk, IN CLSID clsid, OUT IUnknown ** ppunkFound);


// UXTheme wrappers
DWORD QueryThemeServicesWrap(void);

// Other Helpers
BOOL _InitComCtl32();
HRESULT HrShellExecute(HWND hwnd, LPCTSTR lpVerb, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd);
COLORREF ConvertColor(LPTSTR pszColor);
BOOL IUnknown_CompareCLSID(IN IUnknown * punk, IN CLSID clsid);
UINT ErrorMessageBox(HWND hwndOwner, LPCTSTR pszTitle, UINT idTemplate, HRESULT hr, LPCTSTR pszParam, UINT dwFlags);
HRESULT DisplayThemeErrorDialog(HWND hwndParent, HRESULT hrError, UINT nTitle, UINT nTemplate);
BOOL EnumDisplaySettingsExWrap(LPCTSTR lpszDeviceName, DWORD iModeNum, LPDEVMODE lpDevMode, DWORD dwFlags);

void LogStatus(LPCSTR pszMessage, ...);
void LogSystemMetrics(LPCSTR pszMessage, SYSTEMMETRICSALL * pSystemMetrics);

void PostMessageBroadAsync(IN UINT Msg, IN WPARAM wParam, IN LPARAM lParam);
void SystemParametersInfoAsync(IN UINT uiAction, IN UINT uiParam, IN void * pvParam, IN DWORD cbSize, IN UINT fWinIni, IN CDimmedWindow* pDimmedWindow);
void DebugStartWatch(void);
DWORD DebugStopWatch(void);
BOOL IsTSPerfFlagEnabled(enumTSPerfFlag eTSFlag);


void SPISetThreadCounter(LONG *pcThreads);
BOOL SPICreateThread(LPTHREAD_START_ROUTINE pfnThreadProc, void *pvData);

typedef struct tagPROGRESSINFO
{
    IProgressDialog * ppd;
    ULARGE_INTEGER uliBytesCompleted;
    ULARGE_INTEGER uliBytesTotal;
} PROGRESSINFO, * LPPROGRESSINFO;


HRESULT HrByteToStream(LPSTREAM *lppstm, LPBYTE lpb, ULONG cb);
void    GetDateString(char * szSentDateString, ULONG stringLen);
void    SystemPathAppend(TCHAR *pszPath, TCHAR *pszMore);


#endif // _UTIL_H
