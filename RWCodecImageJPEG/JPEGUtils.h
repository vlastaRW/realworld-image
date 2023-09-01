#pragma once

class CJPEGErrorMgr : public jpeg_error_mgr
{
public:
	CJPEGErrorMgr()
	{
		error_exit = ErrorExit;
		emit_message = EmitMessage;
		output_message = OutputMessage;
		format_message = FormatMessage;
		reset_error_mgr = ResetErrorMgr;

		trace_level = 0;	// default = no tracing
		num_warnings = 0;	// no warnings emitted yet
		msg_code = 0;		// may be useful as a flag for "no error"

		// Initialize message table pointers - ???
		jpeg_message_table = NULL;//jpeg_std_message_table;
		last_jpeg_message = -1;//(int) JMSG_LASTMSGCODE - 1;

		addon_message_table = NULL;
		first_addon_message = 0;	// for safety
		last_addon_message = 0;
	}

	~CJPEGErrorMgr()
	{
	}

	static void ErrorExit(j_common_ptr cinfo)
	{
		throw -1;
	}

	static void EmitMessage(j_common_ptr cinfo, int msg_level)
	{
	}

	static void OutputMessage(j_common_ptr cinfo)
	{
	}

	static void FormatMessage(j_common_ptr cinfo, char * buffer)
	{
	}

	static void ResetErrorMgr(j_common_ptr cinfo)
	{
	}
};


class CJPEGCompressStruct : public jpeg_compress_struct
{
public:
	CJPEGCompressStruct()
	{
		ZeroMemory(static_cast<jpeg_compress_struct*>(this), sizeof(jpeg_compress_struct));
		err = &error_mgr;
		jpeg_create_compress(this);
	}
	~CJPEGCompressStruct()
	{
		jpeg_destroy_compress(this);
	}

private:
	CJPEGErrorMgr error_mgr;
};

class CJPEGDecompressStruct : public jpeg_decompress_struct
{
public:
	CJPEGDecompressStruct()
	{
		ZeroMemory(static_cast<jpeg_decompress_struct*>(this), sizeof(jpeg_compress_struct));
		err = &error_mgr;
		jpeg_create_decompress(this);
	}
	~CJPEGDecompressStruct()
	{
		jpeg_destroy_decompress(this);
	}

private:
	CJPEGErrorMgr error_mgr;
};


class CJPEGDataSource : public jpeg_source_mgr
{
public:

	CJPEGDataSource(BYTE const* a_pData, ULONG a_nSize) :
		m_pOrigBuffer(a_pData), m_nOrigSize(a_nSize)
	{
		bDummy[0] = 0xFF;
		bDummy[1] = JPEG_EOI;
		bytes_in_buffer = 0;
		next_input_byte = NULL;
		init_source = InitSource;
		term_source = TermSource;
		fill_input_buffer = FillInputBuffer;
		skip_input_data = SkipInputData;
		resync_to_restart = jpeg_resync_to_restart; /* use default method */
	}

	static void InitSource(j_decompress_ptr cinfo)
	{
		CJPEGDataSource* pThis = static_cast<CJPEGDataSource*>(cinfo->src);
		ATLASSERT(pThis->m_pOrigBuffer != NULL);
		pThis->bytes_in_buffer = pThis->m_nOrigSize;
		pThis->next_input_byte = pThis->m_pOrigBuffer;
	}

	static boolean FillInputBuffer(j_decompress_ptr cinfo)
	{
		CJPEGDataSource* pThis = static_cast<CJPEGDataSource*>(cinfo->src);
		pThis->bytes_in_buffer = 2;
		pThis->next_input_byte = pThis->bDummy;
		return TRUE;
	}
	static void SkipInputData(j_decompress_ptr cinfo, long num_bytes)
	{
		CJPEGDataSource* pThis = static_cast<CJPEGDataSource*>(cinfo->src);
		if (num_bytes < long(pThis->bytes_in_buffer))
		{
			pThis->bytes_in_buffer -= num_bytes;
			pThis->next_input_byte += num_bytes;
		}
		else
		{
			pThis->bytes_in_buffer = 2;
			pThis->next_input_byte = pThis->bDummy;
		}
	}

	static void TermSource(j_decompress_ptr cinfo)
	{
	}

	ULONG DataSizeGet() const
	{
		return m_nOrigSize;
	}
	BYTE const* DataGet() const
	{
		return m_pOrigBuffer;
	}
	void Restart()
	{
		bDummy[0] = 0xFF;
		bDummy[1] = JPEG_EOI;
		bytes_in_buffer = 0;
		next_input_byte = NULL;
	}
private:
	BYTE const* m_pOrigBuffer;
	ULONG m_nOrigSize;
	BYTE bDummy[2];
};


class CJPEGDstStream : public jpeg_destination_mgr
{
public:

	CJPEGDstStream(IReturnedData* a_pDst) :
		m_pDst(a_pDst), m_nWritten(0), m_bFailed(false)
	{
		init_destination = InitDestination;
		empty_output_buffer = EmptyOutputBuffer;
		term_destination = TermDestination;
	}

	static void InitDestination(j_compress_ptr cinfo)
	{
		CJPEGDstStream* pThis = static_cast<CJPEGDstStream*>(cinfo->dest);
		pThis->WriteData(0);
	}
	static boolean EmptyOutputBuffer(j_compress_ptr cinfo)
	{
		CJPEGDstStream* pThis = static_cast<CJPEGDstStream*>(cinfo->dest);
		pThis->WriteData(sizeof(pThis->m_aBuffer));
		return TRUE;
	}
	static void TermDestination(j_compress_ptr cinfo)
	{
		CJPEGDstStream* pThis = static_cast<CJPEGDstStream*>(cinfo->dest);
		pThis->WriteData(sizeof(pThis->m_aBuffer)-pThis->free_in_buffer);
	}

	void WriteData(ULONG a_nLen)
	{
		if (a_nLen != 0)
		{
			m_bFailed = m_bFailed || FAILED(m_pDst->Write(a_nLen, m_aBuffer));
			m_nWritten += a_nLen;
		}
		next_output_byte = m_aBuffer;
		free_in_buffer = sizeof(m_aBuffer);
	}

private:
	CComPtr<IReturnedData> m_pDst;
	BYTE m_aBuffer[4096];
	ULONG m_nWritten;
	bool m_bFailed;
};


class CJPEGMemDataSink : public jpeg_destination_mgr
{
public:

	CJPEGMemDataSink() :
		m_pBuffer(new BYTE[4096]), m_nAlloc(4096)
	{
		init_destination = InitDestination;
		empty_output_buffer = EmptyOutputBuffer;
		term_destination = TermDestination;
	}
	~CJPEGMemDataSink()
	{
		delete[] m_pBuffer;
	}
	ULONG Length() const
	{
		return m_nAlloc-free_in_buffer;//next_output_byte-m_pBuffer;
	}
	BYTE const* Data() const
	{
		return m_pBuffer;
	}
	void Reset()
	{
	}

	static void InitDestination(j_compress_ptr cinfo)
	{
		CJPEGMemDataSink* pThis = static_cast<CJPEGMemDataSink*>(cinfo->dest);
		pThis->next_output_byte = pThis->m_pBuffer;
		pThis->free_in_buffer = pThis->m_nAlloc;
	}
	static boolean EmptyOutputBuffer(j_compress_ptr cinfo)
	{
		CJPEGMemDataSink* pThis = static_cast<CJPEGMemDataSink*>(cinfo->dest);
		ULONG nWritten = pThis->m_nAlloc;//pThis->next_output_byte-pThis->m_pBuffer;
		pThis->m_nAlloc *= 2;
		BYTE* p = new BYTE[pThis->m_nAlloc];
		CopyMemory(p, pThis->m_pBuffer, nWritten);
		delete[] pThis->m_pBuffer;
		pThis->m_pBuffer = p;
		pThis->next_output_byte = pThis->m_pBuffer+nWritten;
		pThis->free_in_buffer = pThis->m_nAlloc-nWritten;
		return TRUE;
	}
	static void TermDestination(j_compress_ptr cinfo)
	{
	}

private:
	BYTE* m_pBuffer;
	ULONG m_nAlloc;
};

