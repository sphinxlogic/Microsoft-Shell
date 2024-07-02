//
//  Copyright 2001 - Microsoft Corporation
//
//  Created By:
//      Geoff Pease (GPease)    23-JAN-2001
//
//  Maintained By:
//      Geoff Pease (GPease)    23-JAN-2001
//
//  Notes:
//      If ENTRY_PREFIX is defined, this indicates that your are trying to
//      included proxy/stub code into the DLL that is generated by the
//      MIDL compiler.
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <InitGuid.h>
#include "Guids.h"

#include <shfusion.h>

#include "DocProp.h"
#include "DefProp.h"
#include "PropertyCacheItem.h"
#include "IEditVariantsInPlace.h"
#include "EditTypeItem.h"
#include "MLEditTypeItem.h"
#include "DropListTypeItem.h"
#include "CalendarTypeItem.h"

#pragma hdrstop

DEFINE_MODULE("DOCPROP3")

//
// Classes in this Component
//
// This table is used to create the objects supported in this DLL. It also is
// used to map a name with a particular CLSID. HrCoCreateInternalInstance() uses
// this table to shortcut COM.
//
//            CreateInstance                        CLSID                                 User Friendly Name                                   Apartment Model
BEGIN_CLASSTABLE 
DEFINE_CLASS( CDocPropShExt::CreateInstance       , CLSID_DocPropShellExtension         , "Microsoft DocProp Shell Ext"                      , "Apartment" )
DEFINE_CLASS( CEditTypeItem::CreateInstance       , CLSID_DocPropEditBoxControl         , "Microsoft DocProp Inplace Edit Box Control"       , "Apartment" )
DEFINE_CLASS( CMLEditTypeItem::CreateInstance     , CLSID_DocPropMLEditBoxControl       , "Microsoft DocProp Inplace ML Edit Box Control"    , "Apartment" )
DEFINE_CLASS( CDropListTypeItem::CreateInstance   , CLSID_DocPropDropListComboControl   , "Microsoft DocProp Inplace Droplist Combo Control" , "Apartment" )
DEFINE_CLASS( CCalendarTypeItem::CreateInstance   , CLSID_DocPropCalendarControl        , "Microsoft DocProp Inplace Calendar Control"       , "Apartment" )  
DEFINE_CLASS( CEditTypeItem::CreateInstance       , CLSID_DocPropTimeControl            , "Microsoft DocProp Inplace Time Control"           , "Apartment" )
END_CLASSTABLE


//
// DLL Globals
//

HINSTANCE g_hInstance = NULL;
LONG      g_cObjects  = 0;
LONG      g_cLock     = 0;
TCHAR     g_szDllFilename[ MAX_PATH ] = { 0 };

LPVOID    g_GlobalMemoryList = NULL;    // Global memory tracking list

#if defined( ENTRY_PREFIX )
extern "C"
{
    extern HINSTANCE hProxyDll;
}
#endif

//
//  Macros to generate RPC entry points
//
#define __rpc_macro_expand2(a, b) a##b
#define __rpc_macro_expand(a, b) __rpc_macro_expand2(a,b)

#if !defined(NO_DLL_MAIN) || defined(ENTRY_PREFIX) || defined(DEBUG)

//
// Description:
//      Dll entry point.
//
BOOL WINAPI
DllMain(
    HINSTANCE hInstIn,      //  DLL instance
    ULONG     ulReasonIn,   //  DLL reason code for entrance.
    LPVOID                  //  lpReservedIn
    )
{
    //
    // KB: NO_THREAD_OPTIMIZATIONS gpease 19-OCT-1999
    //
    // By not defining this you can prvent the linker
    // from calling you DllEntry for every new thread.
    // This makes creating new thread significantly
    // faster if every DLL in a process does it.
    // Unfortunately, not all DLLs do this.
    //
    // In CHKed/DEBUG, we keep this on for memory
    // tracking.
    //
#if defined( DEBUG )
    #define NO_THREAD_OPTIMIZATIONS
#endif // DEBUG

#if defined(NO_THREAD_OPTIMIZATIONS)
    switch( ulReasonIn )
    {
        case DLL_PROCESS_ATTACH:
        {
            SHFusionInitializeFromModule( hInstIn );
#if defined(USE_WMI_TRACING)
            TraceInitializeProcess( g_rgTraceControlGuidList,
                                    ARRAYSIZE( g_rgTraceControlGuidList )
                                    );
#else
            TraceInitializeProcess();
#endif
            TraceCreateMemoryList( g_GlobalMemoryList );
            TraceMemoryDelete( g_GlobalMemoryList, FALSE ); // can't track this list.

#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: DLL_PROCESS_ATTACH - ThreadID = %#x"),
                          GetCurrentThreadId( )
                          );
            FRETURN( TRUE );
#endif // DEBUG
            g_hInstance = (HINSTANCE) hInstIn;

#if defined( ENTRY_PREFIX )
             hProxyDll = g_hInstance;
#endif

            GetModuleFileName( g_hInstance, g_szDllFilename, MAX_PATH );
            break;
        }

        case DLL_PROCESS_DETACH:
        {
#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: DLL_PROCESS_DETACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
                          GetCurrentThreadId( ),
                          g_cLock,
                          g_cObjects
                          );
            FRETURN( TRUE );
#endif // DEBUG
            TraceMemoryAddAddress( g_GlobalMemoryList );
            TraceTerminateMemoryList( g_GlobalMemoryList );
#if defined(USE_WMI_TRACING)
            TraceTerminateProcess( g_rgTraceControlGuidList,
                                   ARRAYSIZE( g_rgTraceControlGuidList )
                                   );
#else
            TraceTerminateProcess();
#endif
            SHFusionUninitialize();
            break;
        }

        case DLL_THREAD_ATTACH:
        {
            TraceInitializeThread( NULL );
#if defined( DEBUG )
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("The thread %#x has started."),
                          GetCurrentThreadId( ) );
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: DLL_THREAD_ATTACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
                          GetCurrentThreadId( ),
                          g_cLock,
                          g_cObjects
                          );
            FRETURN( TRUE );
#endif // DEBUG
            break;
        }

        case DLL_THREAD_DETACH:
        {
#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: DLL_THREAD_DETACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
                          GetCurrentThreadId( ),
                          g_cLock,
                          g_cObjects
                          );
            FRETURN( TRUE );
#endif // DEBUG
            TraceTerminateThread( );;
            break;
        }

        default:
        {
#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: UNKNOWN ENTRANCE REASON - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
                          GetCurrentThreadId( ),
                          g_cLock,
                          g_cObjects
                          );
            FRETURN( TRUE );
#endif // DEBUG
            break;
        }
    }

    return TRUE;

#else // !NO_THREAD_OPTIMIZATIONS

    Assert( ulReasonIn == DLL_PROCESS_ATTACH || ulReasonIn == DLL_PROCESS_DETACH );

    if ( DLL_PROCESS_ATTACH == ulReasonIn )
    {
        SHFusionInitializeFromModule( hInstIn );
        g_hInstance = (HINSTANCE) hInstIn;
#ifdef ENTRY_PREFIX
         hProxyDll = g_hInstance;
#endif

#ifdef DEBUG
#ifdef USE_WMI_TRACING
        TraceInitializeProcess( g_rgTraceControlGuidList,
                                ARRAYSIZE( g_rgTraceControlGuidList )
                                );
#else
        TraceInitializeProcess();
#endif USE_WMI_TRACING
#endif DEBUG
        GetModuleFileName( g_hInstance, g_szDllFilename, MAX_PATH );
        BOOL fResult = DisableThreadLibraryCalls( g_hInstance );
        AssertMsg( fResult, "*ERROR* DisableThreadLibraryCalls( ) failed."  );
    }
    else
    {
#ifdef DEBUG
#ifdef USE_WMI_TRACING
        TraceTerminateProcess( g_rgTraceControlGuidList,
                               ARRAYSIZE( g_rgTraceControlGuidList )
                               );
#else
        TraceTerminateProcess();
#endif USE_WMI_TRACING
#endif DEBUG
        SHFusionUninitialize();
    }

    return TRUE;
#endif // NO_THREAD_OPTIMIZATIONS

}

#endif // !defined(NO_DLL_MAIN) && !defined(ENTRY_PREFIX) && !defined(DEBUG)

//
// Description:
//      OLE calls this to get the class factory from the DLL.
//
// Return Values:
//      S_OK
//          Success.
//
//      any other HRESULT to indicate failure.
//
STDAPI
DllGetClassObject(
    REFCLSID rclsidIn,  //  class ID of the object that the class factory should create.
    REFIID riidIn,      //  Interface of the class factory
    void** ppvOut       //  The interface pointer to the class factory.
    )
{
    TraceFunc( "rclsidIn, riidIn, ppvOut" );

    if ( ppvOut == NULL )
    {
        HRETURN(E_POINTER);
    }

    LPCFACTORY  lpClassFactory;
    HRESULT     hr;
    int         idx;

    hr = CLASS_E_CLASSNOTAVAILABLE;
    idx = 0;
    while( g_DllClasses[ idx ].rclsid )
    {
        if ( *g_DllClasses[ idx ].rclsid == rclsidIn )
        {
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"rclsidIn = %s", g_DllClasses[ idx ].pszName );
            hr = S_OK;
            break;

        }

        idx++;

    }

    if ( hr == CLASS_E_CLASSNOTAVAILABLE )
    {
        TraceMsgGUID( mtfFUNC, "rclsidIn = ", rclsidIn );
#if defined( ENTRY_PREFIX )
        //
        //  See if the MIDL generated code can create it.
        //
        hr = STHR( __rpc_macro_expand( ENTRY_PREFIX, DllGetClassObject )( rclsidIn, riidIn, ppvOut ) );
#endif // defined( ENTRY_PREFIX )
        goto Cleanup;
    }

    Assert( g_DllClasses[ idx ].pfnCreateInstance != NULL );

    lpClassFactory = new CFactory;
    if ( lpClassFactory == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;

    }

    hr = THR( lpClassFactory->Init( g_DllClasses[ idx ].pfnCreateInstance ) );
    if ( FAILED( hr ) )
    {
        TraceDo( lpClassFactory->Release( ) );
        goto Cleanup;

    }

    // Can't safe type.
    hr = lpClassFactory->QueryInterface( riidIn, ppvOut );

    //
    // Release the created instance to counter the AddRef( ) in Init( ).
    //

    ((IUnknown *) lpClassFactory )->Release( );

Cleanup:
    HRETURN(hr);

}

//
// Description:
//      OLE's register entry point.
//
// Return Values:
//      S_OK
//          Success.
//
//      any other HRESULT.
//
STDAPI
DllRegisterServer( void )
{
    HRESULT hr;

    TraceFunc( "" );

    hr = THR( HrRegisterDll( TRUE ) );

#if defined( ENTRY_PREFIX )
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( __rpc_macro_expand( ENTRY_PREFIX, DllRegisterServer )( ) );
    }
#endif // defined( ENTRY_PREFIX )

    if ( SUCCEEDED( hr ) )
    {
        hr = CDocPropShExt::RegisterShellExtensions( TRUE );
    }

    HRETURN(hr);

}

//
// Description:
//      OLE's unregister entry point.
//
// Return Values:
//      S_OK
//          Success.
//
//      any other HRESULT
//
STDAPI
DllUnregisterServer( void )
{
    TraceFunc( "" );

    HRESULT hr;

    hr = THR( HrRegisterDll( FALSE ) );

#if defined( ENTRY_PREFIX )
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( __rpc_macro_expand( ENTRY_PREFIX, DllUnregisterServer )( ) );
    }
#endif // defined( ENTRY_PREFIX )

    if ( SUCCEEDED( hr ) )
    {
        hr = CDocPropShExt::RegisterShellExtensions( TRUE );
    }

    HRETURN( hr );

}

//
// Description:
//      OLE calls this entry point to see if it can unload the DLL.
//
// Return Values:
//      S_OK 
//          Can unload the DLL.
//
//      S_FALSE 
//          Can NOT unload the DLL.
//
STDAPI
DllCanUnloadNow( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( g_cLock || g_cObjects )
    {
        TraceMsg( mtfDLL, "DLL: Can't unload - g_cLock=%u, g_cObjects=%u", g_cLock, g_cObjects );
        hr = S_FALSE;

    } 
#if defined( ENTRY_PREFIX )
    else
    {
        //
        //  Check with the MIDL generated proxy/stubs.
        //
        hr = STHR( __rpc_macro_expand( ENTRY_PREFIX, DllCanUnloadNow )( ) );
    }
#endif

    HRETURN(hr);

}

//
// Description:
//      Mimic CoCreateInstance( ) except that it looks into the DLL table
//      to see if we can shortcut the CoCreate with a simple CreateInstance
//      call.
//
// Return Values:
//      S_OK
//          Success.
//
//      E_OUTOFMEMORY
//          Out of memory.
//
//      other HRESULT values
//
HRESULT
HrCoCreateInternalInstance(
    REFCLSID rclsidIn,      //  Class identifier (CLSID) of the object
    LPUNKNOWN pUnkOuterIn,  //  Pointer to controlling IUnknown
    DWORD dwClsContextIn,   //  Context for running executable code
    REFIID riidIn,          //  Reference to the identifier of the interface
    LPVOID * ppvOut         //  Address of output variable that receives
    )
{
    TraceFunc( "" );

    Assert( ppvOut != NULL );

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    //
    // Limit simple CoCreate( ) only works to INPROC and non-aggregatable objects.
    //

    if ( ( dwClsContextIn & CLSCTX_INPROC_HANDLER )     // inproc only
      && ( NULL == pUnkOuterIn )                        // no aggregation
       )
    {
        int idx;

        //
        // Try to find the class in our DLL table.
        //
        for( idx = 0; g_DllClasses[ idx ].rclsid != NULL; idx++ )
        {
            if ( *g_DllClasses[ idx ].rclsid == rclsidIn )
            {
                LPUNKNOWN punk;
                Assert( g_DllClasses[ idx ].pfnCreateInstance != NULL );

                hr = THR( g_DllClasses[ idx ].pfnCreateInstance( &punk ) );
                if ( SUCCEEDED( hr ) )
                {
                    // Can't safe type.
                    hr = THR( punk->QueryInterface( riidIn, ppvOut ) );
                    punk->Release( );
                }

                break;  // bail loop
            }
        }
    }

    //
    // If not found or asking for something we do not support,
    // use the COM version.
    //

    if ( hr == CLASS_E_CLASSNOTAVAILABLE )
    {
        //
        // Try it the old fashion way...
        //
        hr = THR( CoCreateInstance( rclsidIn, pUnkOuterIn, dwClsContextIn, riidIn, ppvOut ) );

    }

    HRETURN( hr );
}


//
// TODO:    gpease 27-NOV-1999
//          Whilest parrusing the around the MIDL SDK, I foud that
//          RPC creates the same type of class table we do. Maybe
//          we can leverage the MIDL code to create our objects(??).
//