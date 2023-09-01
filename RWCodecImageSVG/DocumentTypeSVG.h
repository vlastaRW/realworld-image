// DocumentEncoderSVG.h : Declaration of the CDocumentEncoderSVG

#pragma once
#include <RWImaging.h>
#include <RWInput.h>


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsSVG[] = L"svg";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathWebP[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameSVG[] = L"[0409]SVG image files[0405]Soubory obrázků SVG";
extern __declspec(selectany) OLECHAR const g_pszTypeNameSVG[] = L"[0409]SVG Image[0405]Obrázek SVG";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameSVG, g_pszTypeNameSVG, g_pszSupportedExtensionsSVG, 0, NULL/*IDI_WEBP_FILETYPE, g_pszShellIconPathWebP*/> CDocumentTypeCreatorSVG;

