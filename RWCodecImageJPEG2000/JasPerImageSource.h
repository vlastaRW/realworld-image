
#pragma once

struct SJasPerImageSource : public jas_stream_t
{
	SJasPerImageSource(DWORD a_nSize, BYTE const* a_pData) : m_nActPos(0), m_nSize(a_nSize), m_pData(a_pData)
	{
		Init();
	}

private:
	void Init()
	{
		openmode_ = JAS_STREAM_READ | JAS_STREAM_BINARY;
		flags_ = 0;
		ops_ = &jas_stream_imgsrcops;
		obj_ = this;
		rwcnt_ = 0;
		rwlimit_ = -1;
		bufbase_ = m_bBuffer;
		bufsize_ = BUFFERSIZE;
		bufstart_ = m_bBuffer+JAS_STREAM_MAXPUTBACK;
		ptr_ = bufstart_;
		cnt_ = 0;
		bufmode_ = JAS_STREAM_FULLBUF;
	}

	static int imgsrc_read(jas_stream_obj_t *obj, char *buf, int cnt)
	{
		SJasPerImageSource* pCtx = reinterpret_cast<SJasPerImageSource*>(obj);
		if (pCtx->m_nActPos >= pCtx->m_nSize)
			return 0;
		int nLen = min(ULONG(cnt), pCtx->m_nSize-pCtx->m_nActPos);
		memcpy(buf, pCtx->m_pData+pCtx->m_nActPos, nLen);
		pCtx->m_nActPos += nLen;
		return nLen;
	}
	static int imgsrc_write(jas_stream_obj_t *obj, char *buf, int cnt)
	{
		return -1;
	}
	static long imgsrc_seek(jas_stream_obj_t *obj, long offset, int origin)
	{
		SJasPerImageSource* pCtx = reinterpret_cast<SJasPerImageSource*>(obj);

		switch (origin)
		{
		case SEEK_SET:
			return pCtx->m_nActPos = offset;
		case SEEK_END:
			return (pCtx->m_nActPos = pCtx->m_nSize-offset);
		case SEEK_CUR:
			return (pCtx->m_nActPos += offset);
		default:
			ATLASSERT(0);
			return -1;
		}
	}
	static int imgsrc_close(jas_stream_obj_t *obj)
	{
		return 0;
	}

	static jas_stream_ops_t const jas_stream_imgsrcops;

	enum { BUFFERSIZE = 4096 };

	BYTE m_bBuffer[BUFFERSIZE+JAS_STREAM_MAXPUTBACK];

	BYTE const* m_pData;
	DWORD m_nSize;
	DWORD m_nActPos;
};

__declspec(selectany) jas_stream_ops_t const SJasPerImageSource::jas_stream_imgsrcops =
{
	imgsrc_read,
	imgsrc_write,
	imgsrc_seek,
	imgsrc_close
};
