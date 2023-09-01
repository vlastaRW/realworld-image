
#pragma once

struct CWebPData : public WebPData
{
	CWebPData()
	{
		WebPDataInit(this);
	}
	~CWebPData()
	{
		WebPDataClear(this);
	}

private:
	CWebPData(CWebPData const&);
};

struct CWebPMuxPtr
{
	WebPMux* p;

	CWebPMuxPtr() : p(NULL) {}
	CWebPMuxPtr(WebPMux* a_p) : p(a_p) {}
	~CWebPMuxPtr() { WebPMuxDelete(p); }
	operator WebPMux*() { return p; }

private:
	CWebPMuxPtr(CWebPMuxPtr const&);
};

struct CWebPMemoryWriter : public WebPMemoryWriter
{
	CWebPMemoryWriter() { WebPMemoryWriterInit(this); }
	~CWebPMemoryWriter() { free(mem); }
};

struct CWebPPicture : public WebPPicture
{
	CWebPPicture() { WebPPictureInit(this); }
	~CWebPPicture() { WebPPictureFree(this); }
};

