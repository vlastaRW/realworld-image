
#pragma once

extern __declspec(selectany) CLSID const CLSID_DocumentEncoderPNG = { 0x1ce9642e, 0xba8c, 0x4110, { 0x87, 0xac, 0xdf, 0x39, 0xd5, 0x7c, 0x96, 0x40 } };
class DECLSPEC_UUID("1CE9642E-BA8C-4110-87AC-DF39D57C9640") DocumentEncoderPNG;

extern __declspec(selectany) CLSID const CLSID_DocumentDecoderPNG = { 0x1953b6dc, 0xc029, 0x4c0f, { 0xbc, 0x93, 0x1b, 0x23, 0x1c, 0xfe, 0x97, 0x50 } };
class DECLSPEC_UUID("1953B6DC-C029-4C0F-BC93-1B231CFE9750") DocumentDecoderPNG;

extern __declspec(selectany) CLSID const CLSID_DocumentEncoderAPNG = { 0xc2cf2ab6, 0x95dd, 0x4226, { 0xab, 0x0d, 0x66, 0x55, 0x92, 0x73, 0x0a, 0x94 } };
class DECLSPEC_UUID("C2CF2AB6-95DD-4226-AB0D-665592730A94") DocumentEncoderAPNG;

// encoder options
extern __declspec(selectany) OLECHAR const CFGID_INTERLACING[] = L"Interlacing"; // bool
extern __declspec(selectany) OLECHAR const CFGID_OPTIMIZE[] = L"Optimize"; // bool
extern __declspec(selectany) OLECHAR const CFGID_EXTRAATTRS[] = L"ExtraAttrs"; // bool

