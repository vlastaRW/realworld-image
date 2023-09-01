

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 04:14:07 2038
 */
/* Compiler settings for RWImageCodecJPEG.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __RWImageCodecJPEG_h__
#define __RWImageCodecJPEG_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __DocumentEncoderJPEG_FWD_DEFINED__
#define __DocumentEncoderJPEG_FWD_DEFINED__

#ifdef __cplusplus
typedef class DocumentEncoderJPEG DocumentEncoderJPEG;
#else
typedef struct DocumentEncoderJPEG DocumentEncoderJPEG;
#endif /* __cplusplus */

#endif 	/* __DocumentEncoderJPEG_FWD_DEFINED__ */


#ifndef __DocumentDecoderJPEG_FWD_DEFINED__
#define __DocumentDecoderJPEG_FWD_DEFINED__

#ifdef __cplusplus
typedef class DocumentDecoderJPEG DocumentDecoderJPEG;
#else
typedef struct DocumentDecoderJPEG DocumentDecoderJPEG;
#endif /* __cplusplus */

#endif 	/* __DocumentDecoderJPEG_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "RWImaging.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __RWImageCodecJPEGLib_LIBRARY_DEFINED__
#define __RWImageCodecJPEGLib_LIBRARY_DEFINED__

/* library RWImageCodecJPEGLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_RWImageCodecJPEGLib;

EXTERN_C const CLSID CLSID_DocumentEncoderJPEG;

#ifdef __cplusplus

class DECLSPEC_UUID("35AE7667-8041-4F2F-986B-4F4945F842F0")
DocumentEncoderJPEG;
#endif

EXTERN_C const CLSID CLSID_DocumentDecoderJPEG;

#ifdef __cplusplus

class DECLSPEC_UUID("1DF031DC-4C60-45E1-962E-127FE8C672E8")
DocumentDecoderJPEG;
#endif
#endif /* __RWImageCodecJPEGLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


