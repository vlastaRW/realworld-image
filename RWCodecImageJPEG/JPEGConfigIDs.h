
#pragma once

extern __declspec(selectany) OLECHAR const CFGID_QUALITY[] = L"Quality";
extern __declspec(selectany) OLECHAR const CFGNAME_QUALITY[] = L"[0409]Quality[0405]Kvalita";
extern __declspec(selectany) OLECHAR const CFGDESC_QUALITY[] = L"[0409]Quality of the compressed image. Usefull range is from 5 to 95.[0405]Kvalita zkomprimovaného obrázku. Užitečné rozmezí je od 5 do 95.";

extern __declspec(selectany) OLECHAR const CFGID_LOSSLESS[] = L"Lossless";
extern __declspec(selectany) OLECHAR const CFGNAME_LOSSLESS[] = L"[0409]Lossless mode[0405]Bezeztrátový mód";
extern __declspec(selectany) OLECHAR const CFGDESC_LOSSLESS[] = L"[0409]If enabled, the application will prevent quality loss in unchanged regions of a JPEG image.[0405]Je-li povoleno, aplikace se pokusí minimalizovat ztráty způsobené opakovanou JPEG kompresí.";
static LONG const CFGVAL_LL_NEVER = 0;
static LONG const CFGVAL_LL_SIMPLE = 1;
static LONG const CFGVAL_LL_ALL = 2;

extern __declspec(selectany) OLECHAR const CFGID_METADATA[] = L"Metadata";
extern __declspec(selectany) OLECHAR const CFGNAME_METADATA[] = L"[0409]Metadata[0405]Metadata";
extern __declspec(selectany) OLECHAR const CFGDESC_METADATA[] = L"[0409]Defines how to handle EXIF metadata.[0405]Určuje, jak bude naloženo s EXIF metadaty.";
static LONG const CFGVAL_MD_REMOVE = 0;
static LONG const CFGVAL_MD_KEEP = 1;
static LONG const CFGVAL_MD_UPDATE = 2;
extern __declspec(selectany) OLECHAR const OPTNAME_MD_REMOVE[] = L"[0409]Do not save metadata[0405]Neukládat metadata";
extern __declspec(selectany) OLECHAR const OPTNAME_MD_KEEP[] = L"[0409]Save original metadata[0405]Uložit původní metadata";
extern __declspec(selectany) OLECHAR const OPTNAME_MD_UPDATE[] = L"[0409]Update and save metadata[0405]Aktualizovat a uložit metadata";

extern __declspec(selectany) OLECHAR const CFGID_CHROMASUBSAMPLING[] = L"Chroma";
extern __declspec(selectany) OLECHAR const CFGNAME_CHROMASUBSAMPLING[] = L"[0409]Chroma subsampling[0405]Podvzorkování barvy";
extern __declspec(selectany) OLECHAR const CFGDESC_CHROMASUBSAMPLING[] = L"[0409]Single chroma value will be saved for the selected block size.[0405]Pro zvolenou velikost bloku bude uložena jedna informace o odstínu.";
static LONG const CFGVAL_CH_1x1 = 0+1;
static LONG const CFGVAL_CH_1x2 = 2+1;
static LONG const CFGVAL_CH_2x1 = 8+1;
static LONG const CFGVAL_CH_2x2 = 10+1;
extern __declspec(selectany) OLECHAR const OPTNAME_CH_1x1[] = L"[0409]1 pixel[0405]1 pixel";
extern __declspec(selectany) OLECHAR const OPTNAME_CH_1x2[] = L"[0409]1x2 pixels[0405]1x2 pixely";
extern __declspec(selectany) OLECHAR const OPTNAME_CH_2x1[] = L"[0409]2x1 pixels[0405]2x1 pixel";
extern __declspec(selectany) OLECHAR const OPTNAME_CH_2x2[] = L"[0409]2x2 pixels[0405]2x2 pixely";

extern __declspec(selectany) OLECHAR const CFGID_ARITHMETIC[] = L"Arithmetic";
extern __declspec(selectany) OLECHAR const CFGNAME_ARITHMETIC[] = L"[0409]Arithmetic coding[0405]Aritmetické kódování";
extern __declspec(selectany) OLECHAR const CFGDESC_ARITHMETIC[] = L"[0409]Results in about 10% smaller files, but only a few applications are currently able to display the images.[0405]Vyrobí asi o 10% menší sobory, ale jen málo současných aplikací bude schopno soubory zobrazit.";
