/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef _GIF_H_
#define _GIF_H_

typedef BYTE PRUint8;
typedef BYTE PRPackedBool;
typedef int int32;
typedef int PRInt32;
typedef int PRUintn;
typedef short PRUint16;
typedef unsigned PRUint32;
typedef bool PRBool;
typedef enum { PR_FAILURE = -1, PR_SUCCESS = 0 } PRStatus;


#define MAX_LZW_BITS          12
#define MAX_BITS            4097 /* 2^MAX_LZW_BITS+1 */
#define MINIMUM_DELAY_TIME   100
#define MAX_COLORS           256

/* gif2.h  
   The interface for the GIF87/89a decoder. 
*/
// List of possible parsing states
typedef enum {
    gif_gather,
    gif_init,                   //1
    gif_type,
    gif_version,
    gif_global_header,
    gif_global_colormap,
    gif_image_start,            //6
    gif_image_header,
    gif_image_colormap,
    gif_image_body,
    gif_lzw_start,
    gif_lzw,                    //11
    gif_sub_block,
    gif_extension,
    gif_control_extension,
    gif_consume_block,
    gif_skip_block,
    gif_done,                   //17
    gif_oom,
    gif_error,
    gif_comment_extension,
    gif_application_extension,
    gif_netscape_extension_block,
    gif_consume_netscape_extension,
    gif_consume_comment,
    gif_delay,
    gif_stop_animating   //added for animation stop 
} gstate;

/* "Disposal" method indicates how the image should be handled in the
   framebuffer before the subsequent image is displayed. */
typedef enum 
{
    DISPOSE_NOT_SPECIFIED      = 0,
    DISPOSE_KEEP               = 1, /* Leave it in the framebuffer */
    DISPOSE_OVERWRITE_BGCOLOR  = 2, /* Overwrite with background color */
    DISPOSE_OVERWRITE_PREVIOUS = 3  /* Save-under */
} gdispose;

/* A GIF decoder's state */
typedef struct gif_struct {
    void* clientptr;
    /* Parsing state machine */
    gstate state;               /* Curent decoder master state */
    PRUint8 *hold;              /* Accumulation buffer */
    PRUint8 *gather_head;       /* Next byte to read in accumulation buffer */
    int32 gather_request_size;  /* Number of bytes to accumulate */
    int32 gathered;             /* bytes accumulated so far*/
    gstate post_gather_state;   /* State after requested bytes accumulated */

    /* LZW decoder state machine */
    PRUint8 *stackp;              /* Current stack pointer */
    int datasize;
    int codesize;
    int codemask;
    int clear_code;             /* Codeword used to trigger dictionary reset */
    int avail;                  /* Index of next available slot in dictionary */
    int oldcode;
    PRUint8 firstchar;
    int count;                  /* Remaining # bytes in sub-block */
    int bits;                   /* Number of unread bits in "datum" */
    int32 datum;                /* 32-bit input buffer */

    /* Output state machine */
    int ipass;                  /* Interlace pass; Ranges 1-4 if interlaced. */
    PRUintn rows_remaining;        /* Rows remaining to be output */
    PRUintn irow;                  /* Current output row, starting at zero */
    PRUint8 *rowbuf;              /* Single scanline temporary buffer */
    PRUint8 *rowend;              /* Pointer to end of rowbuf */
    PRUint8 *rowp;                /* Current output pointer */

    /* Parameters for image frame currently being decoded*/
    PRUintn x_offset, y_offset;    /* With respect to "screen" origin */
    PRUintn height, width;
    int tpixel;                 /* Index of transparent pixel */
    gdispose disposal_method;   /* Restore to background, leave in place, etc.*/
    PRUint8 *local_colormap;    /* Per-image colormap */
    int local_colormap_size;    /* Size of local colormap array. */
    PRUint32 delay_time;        /* Display time, in milliseconds,
                                   for this image in a multi-image GIF */

    /* Global (multi-image) state */
    int screen_bgcolor;         /* Logical screen background color */
    int version;                /* Either 89 for GIF89 or 87 for GIF87 */
    PRUintn screen_width;       /* Logical screen width & height */
    PRUintn screen_height;
    int global_colormap_size;   /* Size of global colormap array. */
    int images_decoded;         /* Counts images for multi-part GIFs */
    int loop_count;             /* Netscape specific extension block to control
                                   the number of animation loops a GIF renders. */

    PRPackedBool progressive_display;    /* If TRUE, do Haeberli interlace hack */
    PRPackedBool interlaced;             /* TRUE, if scanlines arrive interlaced order */
    PRPackedBool is_transparent;         /* TRUE, if tpixel is valid */
    PRPackedBool is_local_colormap_defined;

    PRUint16  prefix[MAX_BITS];          /* LZW decoding tables */
    PRUint8   global_colormap[3*MAX_COLORS];   /* Default colormap if local not supplied, 3 bytes for each color  */
    PRUint8   suffix[MAX_BITS];          /* LZW decoding tables */
    PRUint8   stack[MAX_BITS];           /* Base of LZW decoder stack */

} gif_struct;


/* These are the APIs that the client calls to intialize,
push data to, and shut down the GIF decoder. */
PRBool GIFInit(gif_struct* gs, void* aClientData);

void gif_destroy(gif_struct* aGIFStruct);

PRStatus gif_write(gif_struct* aGIFStruct, const PRUint8 * buf, PRUint32 numbytes);

PRBool gif_write_ready(const gif_struct* aGIFStruct);


class nsGIFDecoder2
{
public:
	nsGIFDecoder2(IDocumentFactoryAnimation* a_pAni, IDocumentFactoryRasterImage* a_pImg, BSTR a_bstrPrefix, IDocumentBase* a_pBase, gif_struct& aGIFStruct) :
		m_pAni(a_pAni), m_pImg(a_pImg), m_bstrPrefix(a_bstrPrefix), m_pBase(a_pBase), m_hRes(E_FAIL), m_nFix0Delay(0),
		m_bDelayedFrame(false), mGIFStruct(aGIFStruct), m_pBuffer(NULL), m_pBackup(NULL), m_bFrameAdded(false)
	{
	}
	~nsGIFDecoder2()
	{
		delete[] m_pBuffer;
		delete[] m_pBackup;
	}

	static int BeginGIF(void* aClientData, PRUint32 aLogicalScreenWidth, PRUint32 aLogicalScreenHeight, PRUint8 aBackgroundRGBIndex);
	static int EndGIF(void* aClientData, int aAnimationLoopCount);
	static int BeginImageFrame(void* aClientData, PRUint32 aFrameNumber, PRUint32 aFrameXOffset, PRUint32 aFrameYOffset, PRUint32 aFrameWidth, PRUint32 aFrameHeight);
	static int EndImageFrame(void* aClientData, PRUint32 aFrameNumber, PRUint32 aDelayTimeout);

	static int HaveDecodedRow(void* aClientData, PRUint8* aRowBufPtr, int aRow, int aDuplicateCount, int aInterlacePass);

	HRESULT Finalize() const;

private:
	void DisposeFrame();

private:
	IDocumentFactoryAnimation* m_pAni;
	IDocumentFactoryRasterImage* m_pImg;
	bool m_bFrameAdded;
	BSTR m_bstrPrefix;
	IDocumentBase* m_pBase;
	HRESULT m_hRes;
	bool m_bDelayedFrame;
	PRUint32 m_aDelayTimeout;
	PRUint8 mBackgroundRGBIndex;
	PRUint32 mLogicalScreenWidth;
	PRUint32 mLogicalScreenHeight;
	BYTE* m_pBuffer;
	ULONG m_nFix0Delay;
	gdispose m_eBackupMode;
	BYTE* m_pBackup;
	ULONG m_nBackupOffX;
	ULONG m_nBackupOffY;
	ULONG m_nBackupSizeX;
	ULONG m_nBackupSizeY;
	gif_struct& mGIFStruct;
	int mCurrentRow;

  //PRUint8 *mAlphaLine;
  //PRUint8 *mRGBLine;
  //PRUint32 mRGBLineMaxSize;
  //PRUint32 mAlphaLineMaxSize;
  //PRUint8 mBackgroundRGBIndex;
  //PRUint8 mCurrentPass;
  //PRUint8 mLastFlushedPass;
};

#endif

