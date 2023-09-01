
#pragma once


struct SJasPerDstStream : public jas_stream_t
{
	SJasPerDstStream(IReturnedData* a_pDst) : pDst(a_pDst), nPos(0)
	{
		openmode_ = JAS_STREAM_WRITE | JAS_STREAM_BINARY;
		flags_ = 0;
		ops_ = &jas_stream_imgsinkops;
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

private:
	static int imgsink_read(jas_stream_obj_t *obj, char *buf, int cnt)
	{
		return -1;
	}
	static int imgsink_write(jas_stream_obj_t *obj, char *buf, int cnt)
	{
		SJasPerDstStream* pCtx = reinterpret_cast<SJasPerDstStream*>(obj);

		if (FAILED(pCtx->pDst->Write(cnt, reinterpret_cast<BYTE const*>(buf))))
			return -1;
		pCtx->nPos += cnt;
		return cnt;
	}
	static long imgsink_seek(jas_stream_obj_t *obj, long offset, int origin)
	{
		ATLASSERT(0);
		return -1;
	}
	static int imgsink_close(jas_stream_obj_t *obj)
	{
		return 0;
	}

	static jas_stream_ops_t const jas_stream_imgsinkops;

	enum { BUFFERSIZE = 4096 };

	BYTE m_bBuffer[BUFFERSIZE+JAS_STREAM_MAXPUTBACK];
	int nPos;
	IReturnedData* pDst;
};

__declspec(selectany) jas_stream_ops_t const SJasPerDstStream::jas_stream_imgsinkops =
{
	imgsink_read,
	imgsink_write,
	imgsink_seek,
	imgsink_close
};
