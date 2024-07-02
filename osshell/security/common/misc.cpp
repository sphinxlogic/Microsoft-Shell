//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, 1998 - 1999
//
//  File:       misc.cpp
//
//--------------------------------------------------------------------------

#include "pch.h"
#include <advpub.h>     // REGINSTALL


/*-----------------------------------------------------------------------------
/ DPA_DestroyCallback
/ --------------
/   Same as in newer comctl32, but not present in NT4 SP4.
/
/ In:
/   -
/ Out:
/   
/----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
/ CallRegInstall
/ --------------
/   Called by DllRegisterServer and DllUnregisterServer to register/unregister
/   this module.  Uses the ADVPACK APIs and loads our INF data from resources.
/
/ In:
/   -
/ Out:
/   HRESULT
/----------------------------------------------------------------------------*/
HRESULT
CallRegInstall(HMODULE hModule, LPCSTR pszSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack;

    TraceEnter(TRACE_COMMON_MISC, "CallRegInstall");

    hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnRegInstall = (REGINSTALL)GetProcAddress(hinstAdvPack, achREGINSTALL);

        if ( pfnRegInstall )
        {
            STRENTRY seReg[] =
            {
                // These two NT-specific entries must be at the end
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };

            hr = pfnRegInstall(hModule, pszSection, &stReg);
        }

        FreeLibrary(hinstAdvPack);
    }

    TraceLeaveResult(hr);
}

// This wrapper function required to make prefast shut up when we are 
// initializing a critical section in a constructor.

void
ExceptionPropagatingInitializeCriticalSection(LPCRITICAL_SECTION critsec)
{
   __try
   {
      ::InitializeCriticalSection(critsec);
   }

   // propagate the exception to our caller.  This will cause Log::Log
   // to abort prematurely, which will jump to the the handler in
   // Log::GetInstance
   
   __except (EXCEPTION_CONTINUE_SEARCH)
   {
   }
}


