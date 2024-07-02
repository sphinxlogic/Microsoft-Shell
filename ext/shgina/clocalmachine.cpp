//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1999.
//
//  File:       CLocalMachine.cpp
//
//  Contents:   implementation of CLocalMachine
//
//----------------------------------------------------------------------------

#include "priv.h"

#include "UserOM.h"
#include "LogonIPC.h"
#include "CInteractiveLogon.h"
#include "WinUser.h"
#include "trayp.h"      // for TM_REFRESH
#include <lmaccess.h>   // for NetUserModalsGet
#include <lmapibuf.h>   // for NetApiBufferFree
#include <lmerr.h>      // for NERR_Success
#include <ntlsa.h>
#include <cfgmgr32.h>
#include <cscapi.h>     // for CSCIsCSCEnabled

//
// IUnknown Interface
//

ULONG CLocalMachine::AddRef()
{
    _cRef++;
    return _cRef;
}


ULONG CLocalMachine::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
    {
        return _cRef;
    }

    delete this;
    return 0;
}


HRESULT CLocalMachine::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = 
    {
        QITABENT(CLocalMachine, IDispatch),
        QITABENT(CLocalMachine, ILocalMachine),
        {0},
    };

    return QISearch(this, qit, riid, ppvObj);
}


//
// IDispatch Interface
//

STDMETHODIMP CLocalMachine::GetTypeInfoCount(UINT* pctinfo)
{ 
    return CIDispatchHelper::GetTypeInfoCount(pctinfo); 
}


STDMETHODIMP CLocalMachine::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{ 
    return CIDispatchHelper::GetTypeInfo(itinfo, lcid, pptinfo); 
}


STDMETHODIMP CLocalMachine::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{ 
    return CIDispatchHelper::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); 
}


STDMETHODIMP CLocalMachine::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    return CIDispatchHelper::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}


//
// ILocalMachine Interface
//



STDMETHODIMP CLocalMachine::get_MachineName(VARIANT* pvar)
{
    HRESULT hr;
    DWORD cch;
    WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH+1];

    if (pvar)
    {
        pvar->vt = VT_BSTR;
        cch = MAX_COMPUTERNAME_LENGTH+1;
        if (GetComputerNameW(szMachineName, &cch))
        {
            pvar->bstrVal = SysAllocString(szMachineName);
        }
        else
        {
            pvar->bstrVal = SysAllocString(TEXT(""));
        }
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


DWORD BuildAccountSidFromRid(LPCWSTR pszServer, DWORD dwRid, PSID* ppSid)
{
    DWORD dwErr = ERROR_SUCCESS;
    PUSER_MODALS_INFO_2 umi2;
    NET_API_STATUS nasRet;

    *ppSid = NULL;

    // Get the account domain Sid on the target machine
    nasRet = NetUserModalsGet(pszServer, 2, (LPBYTE*)&umi2);

    if ( nasRet == NERR_Success )
    {
        UCHAR cSubAuthorities;
        PSID pSid;

        cSubAuthorities = *GetSidSubAuthorityCount(umi2->usrmod2_domain_id);

        // Allocate storage for new the Sid (domain Sid + account Rid)
        pSid = (PSID)LocalAlloc(LPTR, GetSidLengthRequired((UCHAR)(cSubAuthorities+1)));

        if (pSid != NULL)
        {
            if (InitializeSid(pSid,
                              GetSidIdentifierAuthority(umi2->usrmod2_domain_id),
                              (BYTE)(cSubAuthorities+1)))
            {
                // Copy existing subauthorities from domain Sid to new Sid
                for (UINT i = 0; i < cSubAuthorities; i++)
                {
                    *GetSidSubAuthority(pSid, i) = *GetSidSubAuthority(umi2->usrmod2_domain_id, i);
                }

                // Append Rid to new Sid
                *GetSidSubAuthority(pSid, cSubAuthorities) = dwRid;

                *ppSid = pSid;
            }
            else
            {
                dwErr = GetLastError();
                LocalFree(pSid);
            }
        }
        else
        {
            dwErr = GetLastError();
        }

        NetApiBufferFree(umi2);
    }
    else
    {
        dwErr = nasRet;
    }

    return dwErr;
}

PSID g_pGuestSid = NULL;

DWORD GetGuestSid(PSID* ppSid)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (g_pGuestSid == NULL)
    {
        PSID pSid;

        dwErr = BuildAccountSidFromRid(NULL, DOMAIN_USER_RID_GUEST, &pSid);
        if (dwErr == ERROR_SUCCESS)
        {
            if (InterlockedCompareExchangePointer(&g_pGuestSid, pSid, NULL))
            {
                // someone else beat us to initing g_pGuestSid, free ours
                LocalFree(pSid);
                pSid = NULL;
            }
        }
    }

    // There is no need to free the returned PSID
    *ppSid = g_pGuestSid;

    if (*ppSid == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    return dwErr;
}

BOOL FreeGuestSid()
{
    BOOL bRet = FALSE;
    PSID pSid = (PSID)InterlockedExchangePointer(&g_pGuestSid, NULL);
    
    if (pSid)
    {
        LocalFree(pSid);
        bRet = TRUE;
    }

    return bRet;
}

LPCWSTR g_pszGuestAccountName = NULL;

LPCWSTR GetGuestAccountName()
{
    if (g_pszGuestAccountName == NULL)
    {
        PSID pSidGuest;
        WCHAR szGuestAccountName[UNLEN + 1];
        WCHAR szDomain[DNLEN + 1];
        DWORD cchGuestAccountName;
        DWORD cchDomain;
        SID_NAME_USE eUse;

        szGuestAccountName[0] = L'\0';

        if (GetGuestSid(&pSidGuest) == ERROR_SUCCESS)
        {
            cchGuestAccountName = ARRAYSIZE(szGuestAccountName);
            cchDomain = ARRAYSIZE(szDomain);

            if (LookupAccountSidW(NULL,
                                  pSidGuest,
                                  szGuestAccountName,
                                  &cchGuestAccountName,
                                  szDomain,
                                  &cchDomain,
                                  &eUse))
            {
                ASSERT(szGuestAccountName[0] != L'\0');
            }
            else
            {
                // If LookupAccountSid failed, assume english "Guest" and see if the reverse-lookup matches
                // the pSidGuest
                BYTE rgByte[sizeof(SID) + ((SID_MAX_SUB_AUTHORITIES - 1) * sizeof(ULONG))] = {0};
                PSID pSid;
                DWORD cbSid;

                pSid = (PSID)&rgByte;
                cbSid = sizeof(rgByte);
                cchDomain = ARRAYSIZE(szDomain);

                if (LookupAccountNameW(NULL,
                                       L"Guest",
                                       pSid,
                                       &cbSid,
                                       szDomain,
                                       &cchDomain,
                                       &eUse))
                {
                    if (!EqualSid(pSidGuest, pSid))
                    {
                        // guest sid dosen't match the sid for "Guest" account
                        szGuestAccountName[0] = L'\0';
                    }
                }
            }
        }

        if (szGuestAccountName[0] != L'\0')
        {
            LPWSTR pwsz;
            size_t cch;

            cch = lstrlenW(szGuestAccountName) + 1;
            pwsz = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));
            if (pwsz)
            {
                if (FAILED(StringCchCopy(pwsz, cch, szGuestAccountName)) ||
                    InterlockedCompareExchangePointer((void**)&g_pszGuestAccountName, pwsz, NULL))
                {
                    // someone else beat us to initializing g_pszGuestAccountName, free ours
                    LocalFree(pwsz);
                    pwsz = NULL;
                }
            }
        }
    }

    return g_pszGuestAccountName;
}

BOOL FreeGuestAccountName()
{
    BOOL bRet = FALSE;
    LPWSTR psz = (LPWSTR)InterlockedExchangePointer((void **)&g_pszGuestAccountName, NULL);

    if (psz)
    {
        LocalFree(psz);
        bRet = TRUE;
    }

    return bRet;
}

STDMETHODIMP CLocalMachine::get_isGuestEnabled(ILM_GUEST_FLAGS flags, VARIANT_BOOL* pbEnabled)
{
    HRESULT         hr = S_OK;
    DWORD           dwErr;
    BOOL            bEnabled = FALSE;
    USER_INFO_1     *pusri1 = NULL;
    DWORD           dwFlags = (DWORD)(flags & (ILM_GUEST_INTERACTIVE_LOGON | ILM_GUEST_NETWORK_LOGON));
    LPCTSTR         pszGuest;

    if (NULL == pbEnabled)
    {
        return E_POINTER;
    }

    pszGuest = GetGuestAccountName();
    if (pszGuest)
    {
        //  First check to see if the guest account is truly enabled
        dwErr = NetUserGetInfo(NULL, pszGuest, 1, (LPBYTE*)&pusri1);
        if ((ERROR_SUCCESS == dwErr) && ((pusri1->usri1_flags & UF_ACCOUNTDISABLE) == 0))
        {
            // Guest is enabled
            bEnabled = TRUE;

            // Do they want to check the LSA logon rights?
            if (0 != dwFlags)
            {
                BOOL bDenyInteractiveLogon = FALSE;
                BOOL bDenyNetworkLogon = FALSE;
                LSA_HANDLE hLsa;
                LSA_OBJECT_ATTRIBUTES oa = {0};

                oa.Length = sizeof(oa);
                dwErr = LsaNtStatusToWinError(LsaOpenPolicy(NULL, &oa, POLICY_LOOKUP_NAMES, &hLsa));

                if (ERROR_SUCCESS == dwErr)
                {
                    PSID pSid;

                    dwErr = GetGuestSid(&pSid);
                    if (ERROR_SUCCESS == dwErr)
                    {
                        PLSA_UNICODE_STRING pAccountRights;
                        ULONG cRights;

                        // Get the list of LSA rights assigned to the Guest account
                        //
                        // Note that SE_INTERACTIVE_LOGON_NAME is often inherited via
                        // group membership, so its absence doesn't mean much. We could
                        // bend over backwards and check group membership and such, but
                        // Guest normally gets SE_INTERACTIVE_LOGON_NAME one way or
                        // another, so we only check for SE_DENY_INTERACTIVE_LOGON_NAME
                        // here.

                        dwErr = LsaNtStatusToWinError(LsaEnumerateAccountRights(hLsa, pSid, &pAccountRights, &cRights));
                        if (ERROR_SUCCESS == dwErr)
                        {
                            PLSA_UNICODE_STRING pRight;
                            for (pRight = pAccountRights; cRights > 0 && 0 != dwFlags; pRight++, cRights--)
                            {
                                if (0 != (dwFlags & ILM_GUEST_INTERACTIVE_LOGON) &&
                                    CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT,
                                                                NORM_IGNORECASE,
                                                                SE_DENY_INTERACTIVE_LOGON_NAME,
                                                                -1,
                                                                pRight->Buffer,
                                                                pRight->Length/2))
                                {
                                    bDenyInteractiveLogon = TRUE;
                                    dwFlags &= ~ILM_GUEST_INTERACTIVE_LOGON;
                                }
                                else if (0 != (dwFlags & ILM_GUEST_NETWORK_LOGON) &&
                                    CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT,
                                                                NORM_IGNORECASE,
                                                                SE_DENY_NETWORK_LOGON_NAME,
                                                                -1,
                                                                pRight->Buffer,
                                                                pRight->Length/2))
                                {
                                    bDenyNetworkLogon = TRUE;
                                    dwFlags &= ~ILM_GUEST_NETWORK_LOGON;
                                }
                            }
                            LsaFreeMemory(pAccountRights);
                        }
                        else if (ERROR_FILE_NOT_FOUND == dwErr)
                        {
                            // Guest isn't in LSA's database, so we know it can't
                            // have either of the deny logon rights.
                            dwErr = ERROR_SUCCESS;
                        }
                    }
                    LsaClose(hLsa);
                }

                if (bDenyInteractiveLogon || bDenyNetworkLogon)
                    bEnabled = FALSE;
            }
        }
    }
    else
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (NULL != pusri1)
    {
        (NET_API_STATUS)NetApiBufferFree(pusri1);
    }

    hr = HRESULT_FROM_WIN32(dwErr);

    *pbEnabled = bEnabled ? VARIANT_TRUE : VARIANT_FALSE;

    return hr;
}

STDMETHODIMP CLocalMachine::EnableGuest(ILM_GUEST_FLAGS flags)
{
    DWORD           dwErr;
    USER_INFO_1     *pusri1;
    DWORD dwFlags = (DWORD)(flags & (ILM_GUEST_INTERACTIVE_LOGON | ILM_GUEST_NETWORK_LOGON));
    LPCTSTR pszGuest;

    pszGuest = GetGuestAccountName();
    if (pszGuest)
    {
        //  First truly enable the guest account. Do this ALL the time.
        dwErr = NetUserGetInfo(NULL, pszGuest, 1, (LPBYTE*)&pusri1);
        if (ERROR_SUCCESS == dwErr)
        {
            pusri1->usri1_flags &= ~UF_ACCOUNTDISABLE;
            dwErr = NetUserSetInfo(NULL, pszGuest, 1, (LPBYTE)pusri1, NULL);
            if (ERROR_SUCCESS == dwErr && 0 != dwFlags)
            {
                LSA_HANDLE hLsa;
                LSA_OBJECT_ATTRIBUTES oa = {0};

                oa.Length = sizeof(oa);
                dwErr = LsaNtStatusToWinError(LsaOpenPolicy(NULL, &oa, POLICY_LOOKUP_NAMES, &hLsa));

                if (ERROR_SUCCESS == dwErr)
                {
                    PSID pSid;

                    dwErr = GetGuestSid(&pSid);
                    if (ERROR_SUCCESS == dwErr)
                    {
                        if (0 != (dwFlags & ILM_GUEST_INTERACTIVE_LOGON))
                        {
                            DECLARE_CONST_UNICODE_STRING(usDenyLogon, SE_DENY_INTERACTIVE_LOGON_NAME);
                            NTSTATUS status = LsaRemoveAccountRights(hLsa, pSid, FALSE, (PLSA_UNICODE_STRING)&usDenyLogon, 1);
                            dwErr = LsaNtStatusToWinError(status);
                        }
                        if (0 != (dwFlags & ILM_GUEST_NETWORK_LOGON))
                        {
                            DECLARE_CONST_UNICODE_STRING(usDenyLogon, SE_DENY_NETWORK_LOGON_NAME);
                            NTSTATUS status = LsaRemoveAccountRights(hLsa, pSid, FALSE, (PLSA_UNICODE_STRING)&usDenyLogon, 1);
                            if (ERROR_SUCCESS == dwErr)
                                dwErr = LsaNtStatusToWinError(status);
                        }

                        if (ERROR_FILE_NOT_FOUND == dwErr)
                        {
                            //
                            // NTRAID#NTBUG9-396428-2001/05/16-jeffreys
                            //
                            // This means Guest isn't in LSA's database, so we know
                            // it can't have either of the deny logon rights. Since
                            // we were trying to remove one or both rights, count
                            // this as success.
                            //
                            dwErr = ERROR_SUCCESS;
                        }
                    }
                    LsaClose(hLsa);
                }
            }
            (NET_API_STATUS)NetApiBufferFree(pusri1);
        }
    }
    else
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    return HRESULT_FROM_WIN32(dwErr);
}

STDMETHODIMP CLocalMachine::DisableGuest(ILM_GUEST_FLAGS flags)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwFlags = (DWORD)(flags & (ILM_GUEST_INTERACTIVE_LOGON | ILM_GUEST_NETWORK_LOGON));

    if (0 != dwFlags)
    {
        LSA_HANDLE hLsa;
        LSA_OBJECT_ATTRIBUTES oa = {0};

        // Turn on DenyInteractiveLogon and/or DenyNetworkLogon, but don't
        // necessarily change the enabled state of the guest account.

        oa.Length = sizeof(oa);
        dwErr = LsaNtStatusToWinError(LsaOpenPolicy(NULL, &oa, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES, &hLsa));

        if (ERROR_SUCCESS == dwErr)
        {
            PSID pSid;

            dwErr = GetGuestSid(&pSid);
            if (ERROR_SUCCESS == dwErr)
            {
                if (0 != (dwFlags & ILM_GUEST_INTERACTIVE_LOGON))
                {
                    DECLARE_CONST_UNICODE_STRING(usDenyLogon, SE_DENY_INTERACTIVE_LOGON_NAME);
                    NTSTATUS status = LsaAddAccountRights(hLsa, pSid, (PLSA_UNICODE_STRING)&usDenyLogon, 1);
                    dwErr = LsaNtStatusToWinError(status);
                }
                if (0 != (dwFlags & ILM_GUEST_NETWORK_LOGON))
                {
                    DECLARE_CONST_UNICODE_STRING(usDenyLogon, SE_DENY_NETWORK_LOGON_NAME);
                    NTSTATUS status = LsaAddAccountRights(hLsa, pSid, (PLSA_UNICODE_STRING)&usDenyLogon, 1);
                    if (ERROR_SUCCESS == dwErr)
                        dwErr = LsaNtStatusToWinError(status);
                }
            }
            LsaClose(hLsa);
        }

        if (ERROR_SUCCESS == dwErr)
        {
            // If both  SE_DENY_INTERACTIVE_LOGON_NAME and SE_DENY_NETWORK_LOGON_NAME
            // are turned on, then we might as well disable the account altogether.
            if ((ILM_GUEST_INTERACTIVE_LOGON | ILM_GUEST_NETWORK_LOGON) == dwFlags)
            {
                // We just turned both on, so disable guest below
                dwFlags = 0;
            }
            else
            {
                VARIANT_BOOL bEnabled;

                if (ILM_GUEST_INTERACTIVE_LOGON == dwFlags)
                {
                    // We just turned on SE_DENY_INTERACTIVE_LOGON_NAME, check
                    // for SE_DENY_NETWORK_LOGON_NAME.
                    flags = ILM_GUEST_NETWORK_LOGON;
                }
                else if (ILM_GUEST_NETWORK_LOGON == dwFlags)
                {
                    // We just turned on SE_DENY_NETWORK_LOGON_NAME, check
                    // for SE_DENY_INTERACTIVE_LOGON_NAME.
                    flags = ILM_GUEST_INTERACTIVE_LOGON;
                }
                else
                {
                    // Getting here implies that someone defined a new flag.
                    // Setting flags to ILM_GUEST_ACCOUNT causes a benign
                    // result in all cases (we only disable guest if guest
                    // is already disabled).
                    flags = ILM_GUEST_ACCOUNT;
                }

                if (SUCCEEDED(get_isGuestEnabled(flags, &bEnabled)) && (VARIANT_FALSE == bEnabled))
                {
                    // Both are on, so disable guest below
                    dwFlags = 0;
                }
            }
        }
    }

    if (0 == dwFlags)
    {
        USER_INFO_1 *pusri1;
        LPCTSTR pszGuest = GetGuestAccountName();

        if (pszGuest)
        {
            //  Truly disable the guest account.
            dwErr = NetUserGetInfo(NULL, pszGuest, 1, (LPBYTE*)&pusri1);
            if (ERROR_SUCCESS == dwErr)
            {
                pusri1->usri1_flags |= UF_ACCOUNTDISABLE;
                dwErr = NetUserSetInfo(NULL, GetGuestAccountName(), 1, (LPBYTE)pusri1, NULL);
                (NET_API_STATUS)NetApiBufferFree(pusri1);
            }
        }
        else
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return HRESULT_FROM_WIN32(dwErr);
}

STDMETHODIMP CLocalMachine::get_isFriendlyUIEnabled(VARIANT_BOOL* pbEnabled)

{
    *pbEnabled = ShellIsFriendlyUIActive() ? VARIANT_TRUE : VARIANT_FALSE;
    return(S_OK);
}

STDMETHODIMP CLocalMachine::put_isFriendlyUIEnabled(VARIANT_BOOL bEnabled)

{
    HRESULT hr;

    if (ShellEnableFriendlyUI(bEnabled != VARIANT_FALSE ? TRUE : FALSE) != FALSE)
    {
        RefreshStartMenu();
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return(hr);
}

STDMETHODIMP CLocalMachine::get_isMultipleUsersEnabled(VARIANT_BOOL* pbEnabled)

{
    *pbEnabled = ShellIsMultipleUsersEnabled() ? VARIANT_TRUE : VARIANT_FALSE;
    return(S_OK);
}

STDMETHODIMP CLocalMachine::put_isMultipleUsersEnabled(VARIANT_BOOL bEnabled)

{
    HRESULT hr;

    if (ShellEnableMultipleUsers(bEnabled != VARIANT_FALSE ? TRUE : FALSE) != FALSE)
    {
        RefreshStartMenu();
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return(hr);
}

STDMETHODIMP CLocalMachine::get_isRemoteConnectionsEnabled(VARIANT_BOOL* pbEnabled)

{
    *pbEnabled = ShellIsRemoteConnectionsEnabled() ? VARIANT_TRUE : VARIANT_FALSE;
    return(S_OK);
}

STDMETHODIMP CLocalMachine::put_isRemoteConnectionsEnabled(VARIANT_BOOL bEnabled)

{
    HRESULT hr;

    if (ShellEnableRemoteConnections(bEnabled != VARIANT_FALSE ? TRUE : FALSE) != FALSE)
    {
        RefreshStartMenu();
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return(hr);
}

BOOL _CanEject()
{
    BOOL fEjectAllowed = FALSE;

    if(SHRestricted(REST_NOSMEJECTPC))  //Is there a policy restriction?
        return FALSE;

    CM_Is_Dock_Station_Present(&fEjectAllowed);

    return SHTestTokenPrivilege(NULL, SE_UNDOCK_NAME) &&
           fEjectAllowed  &&
           !GetSystemMetrics(SM_REMOTESESSION);
}

STDMETHODIMP CLocalMachine::get_isUndockEnabled(VARIANT_BOOL* pbEnabled)

{
    CLogonIPC   objLogon;

    if (objLogon.IsLogonServiceAvailable())
    {
        *pbEnabled = objLogon.TestEjectAllowed() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
        *pbEnabled = _CanEject() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    return(S_OK);
}

STDMETHODIMP CLocalMachine::get_isShutdownAllowed(VARIANT_BOOL* pbShutdownAllowed)

{
    CLogonIPC   objLogon;

    if (objLogon.IsLogonServiceAvailable())
    {
        *pbShutdownAllowed = objLogon.TestShutdownAllowed() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
        *pbShutdownAllowed = VARIANT_FALSE;
    }
    return(S_OK);
}

STDMETHODIMP CLocalMachine::get_isGuestAccessMode(VARIANT_BOOL* pbForceGuest)
{
    *pbForceGuest = SUCCEEDED(_IsGuestAccessMode()) ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}


STDMETHODIMP CLocalMachine::get_isOfflineFilesEnabled(VARIANT_BOOL *pbEnabled)
{
    if (CSCIsCSCEnabled())
    {
        *pbEnabled = VARIANT_TRUE;
    }
    else
    {
        *pbEnabled = VARIANT_FALSE;
    }

    return S_OK;
}


LPCWSTR g_pszAdminAccountName = NULL;

LPCWSTR GetAdminAccountName(void)
{
    if (g_pszAdminAccountName == NULL)
    {
        PSID pSidAdmin;

        if (BuildAccountSidFromRid(NULL,
                                   DOMAIN_USER_RID_ADMIN,
                                   &pSidAdmin) == ERROR_SUCCESS)
        {
            WCHAR szAdminAccountName[UNLEN + 1];
            WCHAR szDomain[DNLEN + 1];
            DWORD cchAdminAccountName;
            DWORD cchDomain;
            SID_NAME_USE eUse;

            cchAdminAccountName = ARRAYSIZE(szAdminAccountName);
            cchDomain = ARRAYSIZE(szDomain);

            if (LookupAccountSidW(NULL,
                                  pSidAdmin,
                                  szAdminAccountName,
                                  &cchAdminAccountName,
                                  szDomain,
                                  &cchDomain,
                                  &eUse))
            {
                LPWSTR psz;
                DWORD cch;

                ASSERT(szAdminAccountName[0] != L'\0');

                cch = lstrlenW(szAdminAccountName) + 1;
                psz = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));
                if (psz)
                {
                    if (InterlockedCompareExchangePointer((void**)&g_pszAdminAccountName, psz, NULL))
                    {
                        // someone else beat us to initing g_pszAdminAccountName, free ours
                        LocalFree(psz);
                        psz = NULL;
                    }
                }
            }
                
            LocalFree(pSidAdmin);
        }
    }

    return g_pszAdminAccountName;
}

BOOL FreeAdminAccountName()
{
    BOOL bRet = FALSE;
    LPWSTR psz = (LPWSTR)InterlockedExchangePointer((void **)&g_pszAdminAccountName, NULL);

    if (psz)
    {
        LocalFree(psz);
        bRet = TRUE;
    }

    return bRet;
}

STDMETHODIMP CLocalMachine::get_AccountName(VARIANT varAccount, VARIANT* pvar)
{
    DWORD dwRID = 0;
    LPCWSTR pszName = NULL;

    if (NULL == pvar)
        return E_POINTER;

    switch (varAccount.vt)
    {
    case VT_I4:
    case VT_UI4:
        dwRID = varAccount.ulVal;
        break;

    case VT_BSTR:
        if (0 == StrCmpIW(varAccount.bstrVal, L"Guest"))
            dwRID = DOMAIN_USER_RID_GUEST;
        else if (0 == StrCmpIW(varAccount.bstrVal, L"Administrator"))
            dwRID = DOMAIN_USER_RID_ADMIN;
        else
            return E_INVALIDARG;
        break;

    default:
        return E_INVALIDARG;
    }

    switch (dwRID)
    {
    case DOMAIN_USER_RID_GUEST:
        pszName = GetGuestAccountName();
        break;

    case DOMAIN_USER_RID_ADMIN:
        pszName = GetAdminAccountName();
        break;

    default:
        return E_INVALIDARG;
    }

    pvar->vt = VT_BSTR;
    pvar->bstrVal = SysAllocString(pszName);

    return(S_OK);
}

STDMETHODIMP CLocalMachine::TurnOffComputer()
{
    HRESULT hr;
    CLogonIPC   objLogon;

    if (!objLogon.IsLogonServiceAvailable())
    {
        return E_FAIL;
    }

    if (objLogon.TurnOffComputer ())
        hr = S_OK;
    else
        hr = E_FAIL;

    return hr;
}

STDMETHODIMP CLocalMachine::UndockComputer()
{
    HRESULT hr;
    CLogonIPC   objLogon;

    if (!objLogon.IsLogonServiceAvailable())
    {
        return E_FAIL;
    }

    if (objLogon.EjectComputer())
        hr = S_OK;
    else
        hr = E_FAIL;
    return hr;
}

STDMETHODIMP CLocalMachine::SignalUIHostFailure()
{
    CLogonIPC   objLogon;

    if (!objLogon.IsLogonServiceAvailable())
    {
        return E_FAIL;
    }

    objLogon.SignalUIHostFailure ();
    return S_OK;
}

STDMETHODIMP CLocalMachine::AllowExternalCredentials()

{
    CLogonIPC   objLogon;

    if (!objLogon.IsLogonServiceAvailable())
    {
        return E_FAIL;
    }

    if (!objLogon.AllowExternalCredentials ())
    {
        return E_NOTIMPL;
    }
    else
    {
        return S_OK;
    }
}

STDMETHODIMP CLocalMachine::RequestExternalCredentials()
{
    CLogonIPC   objLogon;

    if (!objLogon.IsLogonServiceAvailable())
    {
        return E_FAIL;
    }

    objLogon.RequestExternalCredentials ();
    return S_OK;
}

STDMETHODIMP CLocalMachine::LogonWithExternalCredentials(BSTR pstrUsername, BSTR pstrDomain, BSTR pstrPassword, VARIANT_BOOL* pbRet)
{
    HRESULT hr;
    CLogonIPC objLogon;
    TCHAR szUsername[UNLEN + 1];
    TCHAR szDomain[DNLEN + 1];
    TCHAR szPassword[PWLEN + 1];

    if (pstrUsername)
    {
        StringCchCopy(szUsername, ARRAYSIZE(szUsername), pstrUsername);
    }
    else
    {
        szUsername[0] = TEXT('\0');
    }

    if (pstrDomain)
    {
        StringCchCopy(szDomain, ARRAYSIZE(szDomain),  pstrDomain);
    }
    else
    {
        szDomain[0] = TEXT('\0');
    }

    if (pstrPassword)
    {
        StringCchCopy(szPassword, ARRAYSIZE(szPassword), pstrPassword);
    }
    else
    {
        szPassword[0] = TEXT('\0');
    }
        
    if (!objLogon.IsLogonServiceAvailable())
    {
        *pbRet = VARIANT_FALSE;
        hr = S_OK;
    }
    else
    {
        if (objLogon.LogUserOn(szUsername, szDomain, szPassword))
        {
            *pbRet = VARIANT_TRUE;
            hr = S_OK;
        }
        else
        {
            *pbRet = VARIANT_FALSE;
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}

//  --------------------------------------------------------------------------
//  CLocalMachine::InitiateInteractiveLogon
//
//  Arguments:  pstrUsername    =   User name.
//              pstrDomain      =   Domain.
//              pstrPassword    =   Password (in clear text).
//              pbRet           =   Result (returned).
//
//  Returns:    HRESULT
//
//  Purpose:    Send a request for interactive logon using CInteractiveLogon.
//              It's magic. I don't care how it works.
//
//  History:    2000-12-06  vtan        created
//  --------------------------------------------------------------------------

STDMETHODIMP CLocalMachine::InitiateInteractiveLogon(BSTR pstrUsername, BSTR pstrDomain, BSTR pstrPassword, DWORD dwTimeout, VARIANT_BOOL* pbRet)

{
    DWORD   dwErrorCode;

    dwErrorCode = CInteractiveLogon::Initiate(pstrUsername, pstrDomain, pstrPassword, dwTimeout);
    *pbRet = (ERROR_SUCCESS == dwErrorCode) ? VARIANT_TRUE : VARIANT_FALSE;
    return(HRESULT_FROM_WIN32(dwErrorCode));
}

//  --------------------------------------------------------------------------
//  CLocalMachine::RefreshStartMenu
//
//  Arguments:  <none>
//
//  Returns:    <none>
//
//  Purpose:    Finds the shell tray window and sends it a message to refresh
//              its contents.
//
//  History:    2000-08-01  vtan        created
//  --------------------------------------------------------------------------

void    CLocalMachine::RefreshStartMenu (void)

{
    HWND    hwndTray;

    hwndTray = FindWindow(TEXT("Shell_TrayWnd"), NULL);
    if (hwndTray != NULL)
    {
        TBOOL(PostMessage(hwndTray, TM_REFRESH, 0, 0));
    }
}

CLocalMachine::CLocalMachine() : _cRef(1), CIDispatchHelper(&IID_ILocalMachine, &LIBID_SHGINALib)
{
    DllAddRef();
}


CLocalMachine::~CLocalMachine()
{
    ASSERT(_cRef == 0);
    DllRelease();
}


STDAPI CLocalMachine_Create(REFIID riid, LPVOID* ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CLocalMachine* pLocalMachine = new CLocalMachine();

    if (pLocalMachine)
    {
        hr = pLocalMachine->QueryInterface(riid, ppv);
        pLocalMachine->Release();
    }

    return hr;
}

