

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Fri Jan 06 20:42:58 2023
 */
/* Compiler settings for .\RWCreatorWIA.idl:
    Oicf, W1, Zp8, env=Win64 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __RWCreatorWIA_h__
#define __RWCreatorWIA_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __DocumentCreatorWIA_FWD_DEFINED__
#define __DocumentCreatorWIA_FWD_DEFINED__

#ifdef __cplusplus
typedef class DocumentCreatorWIA DocumentCreatorWIA;
#else
typedef struct DocumentCreatorWIA DocumentCreatorWIA;
#endif /* __cplusplus */

#endif 	/* __DocumentCreatorWIA_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "RWConceptDesignerExtension.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __RWCreatorWIALib_LIBRARY_DEFINED__
#define __RWCreatorWIALib_LIBRARY_DEFINED__

/* library RWCreatorWIALib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_RWCreatorWIALib;

EXTERN_C const CLSID CLSID_DocumentCreatorWIA;

#ifdef __cplusplus

class DECLSPEC_UUID("54FF5EE0-185B-4C58-8553-F72EBFE4B987")
DocumentCreatorWIA;
#endif
#endif /* __RWCreatorWIALib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


