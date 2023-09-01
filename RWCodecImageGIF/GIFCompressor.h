
#pragma once


template<class CWriter>
class bitpacker

  //Packs a sequence of variable length codes into a buffer. Every time
  //255 bytes have been completed, they are written to a binary file as a
  //data block of 256 bytes (where the first byte is the 'bytecount' of the
  //rest and therefore equals 255). Any remaining bits are moved to the
  //buffer start to become part of the following block. After submitting
  //the last code via submit(), the user must call writeflush() to write
  //a terminal, possibly shorter, data block.

{
private:
CWriter& binfile;
BYTE buffer[260];  // holds the total buffer
BYTE *pos;     // points into buffer
WORD need;    // used by addcodetobuffer(), see there

public:
bool bad;            // to be checked after construction
unsigned byteswritten;  // Initialized with 0 upon construction, updated
                     // in submit() and writeflush(), counts the total
                     // number of bytes written during the object's lifetime

BYTE *addcodetobuffer(unsigned code,short n)

 //Copied from Michael A. Mayer's AddCodeToBuffer(), with the following
 //changes:
 //- addcodetobuffer() is now a class member of class bitpacker
 //- former local static variable 'need' now is a class member and
 //  initialized with 8 in the constructor
 //- former formal parameter 'buf' is now the class member 'BYTE *pos' and
 //  initialized with 'pos=buffer' in the constructor
 //- type of 'code' has been changed to 'unsigned long'.

 //'n' tells how many least significant bits of 'code' are to be packed
 //into 'buffer' in this call, its possible values are 1..32 and -1
 //(see the n<0 branch).
 //When the function is called, 'pos' points to a partially empty byte and
 //'need' (whose possible values are 1..8) tells how many bits will still fit
 //in there. Since the bytes are filled bottom up (least significant bits first),
 //the 'need' vacant bits are the most significant ones.

{
unsigned mask;

   //If called with n==-1, then if the current byte is partially filled,
   //leave it alone and target the next byte (which is empty).  

if(n<0)           
  {               
  if(need<8)
    {
    pos++;
    *pos=0x00;              
    }
  need=8;
  return pos;
  }
while (n >= need)
  {
  mask = (1<<need)-1;            // 'mask'= all zeroes followed by 'need' ones
  *pos += (BYTE)((mask&code) << (8-need)); // the 'need' lowest bits of 'code'                                                                                // fill the current byte at its upper end
  pos++;                     // byte is now full, next byte
  *pos=0x00;                 // initialize it
  code = code>>need;         // remove the written bits from code
  n -= need;                 // update the length of 'code'
  need=8;                    // current byte can take 8 bits
  }
//                                                        Now we have n < need
if(n>0)
  {
  mask= (1<<n)-1;
  *pos += (BYTE)((mask&code)<<(8-need));
                             // (remainder of) code is written to the n
                             // rightmost free bits of the current byte.
  need-=n;                   // The current byte can still take 'need' bits,
                             // and we have 'need'>0. The bits will be filled
                             // upon future calls.
  }                         
return pos;
} // bitpacker::addcodetobuffer





BYTE *submit(unsigned code,WORD n)

  //Packs an incoming code of n bits to the buffer. As soon as 255 bytes
  //are full, they are written to 'binfile' as a data block and cleared
  //from 'buffer'.

{
addcodetobuffer(code,n);
if(pos-buffer >= 255)             // pos pointing to buffer[255] or beyond
  {
	  BYTE tmp = 255;
	  binfile.Write(&tmp, 1);   // write the "bytecount-byte"
	  binfile.Write(buffer,255);// write buffer[0..254] to file
  buffer[0]=buffer[255];          // rotate the following bytes,
  buffer[1]=buffer[256];          // which may still contain data, to the
  buffer[2]=buffer[257];          // beginning of buffer, and point
  buffer[3]=buffer[258];          // (pos,need) to the position for new
  pos-=255;                       // input ('need' can stay unchanged)
  byteswritten+=256;
  }
return pos;
} // bitpacker::submit





void writeflush(void)
  //Writes any data contained in 'buffer' to the file as one data block of
  //1<=length<=255. Clears 'buffer' and reinitializes for new data.

{
addcodetobuffer(0,-1);        // close any partially filled terminal byte
if(pos<=buffer)               // buffer is empty
  return;
BYTE tmp = pos-buffer;
binfile.Write(&tmp, 1);   // write the "bytecount-byte"
binfile.Write(buffer,pos-buffer);
byteswritten+=(pos-buffer+1);
pos=buffer;
*pos=0x00;
need=8;
} // bitpacker::writeflush

 


bitpacker(CWriter& bf) : binfile(bf)
{
need=8;
byteswritten=0;
pos=buffer;
*pos=0x00;
} // bitpacker-Constructor

}; // class bitpacker


template<class CWriter>
class gifcompressor

  //Contains the stringtable, generates compression codes and writes them to a
  //binary file, formatted in data blocks of maximum length 255 with
  //additional bytecount header.
  //   Users will open the binary file, write the first part themselves,
  //create a gifcompressor, call its method writedatablocks(), delete it and
  //write the last byte 0x3b themselves before closing the file.

{
private:
bitpacker<CWriter> bp;        // object that does the packing and writing of the
                     // compression codes
unsigned nofdata;       // number of pixels in the data stream
unsigned width;         // width of bitmap in pixels
unsigned height;        // height of bitmap in pixels
unsigned div8,div4,div2; // number of bitmap rows whose index is divisible by
                     // 8,4,2 if the row counting starts with row 0
unsigned GDIwidth;      // width rounded up to next multiple of 4
                     // = number of bytes per line as provided by the Windows GDI
unsigned curordinal;    // ordinal number of next pixel to be encoded
BYTE pixel;         // next pixel to be encoded
BYTE const *data;         // points to the first byte of the input data stream
WORD datadepth;    // length of the input data in bits. Possible values
                     // are 1..8. Still, each input item comes in a
                     // separate byte.
WORD nbits;        // current length of compression codes in bits
                     // (changes during encoding process)
bool interlaced;
WORD *axon, *next; // arrays making up the stringtable
BYTE *pix;          // dito
unsigned cc;            // "clear code" which signals the clearing of the string table
unsigned eoi;           // "end-of-information code" which must be the last item of
                     // the code stream
WORD freecode;     // next code to be added to the string table

void iniroots(void)
//  Initialize a root node for each root code.
{
WORD nofrootcodes=1<<max(2,datadepth);
for(WORD i=0; i<nofrootcodes; i++)
  {
  axon[i]=0;
  pix[i]=(BYTE)i;
  // next[] is unused for root codes       
  }
} // gifcompressor::iniroots


void flushstringtable(void)
  //The stringtable is flushed by removing the outlets of all root nodes.
  //It is thereby reduced to its initialized state.

{
WORD nofrootcodes=1<<max(2,datadepth);
for(WORD i=0; i<nofrootcodes; i++)
  axon[i]=0;
} // gifcompressor::flushstringtable





WORD findpixeloutlet(WORD headnode,BYTE pixel)
  //Checks if the chain emanating from headnode's axon contains a node
  //for 'pixel'. Returns that node's address (=code), or 0 if there
  //is no such node. (0 cannot be the root node 0, since root nodes
  //occur in no chain).

{
WORD outlet=axon[headnode];
while(outlet)
  {
  if (pix[outlet]==pixel)
    return outlet;
  outlet=next[outlet];
  }
return 0;
} // gifcompressor::findpixeloutlet





unsigned weiter(void)
  //Writes the next code to the codestream and adds one entry to
  //the stringtable. Does not change 'freecode'. Moves 'curordinal'
  //forward and returns it pointing to the first pixel that hasn't
  //been encoded yet. Reckognizes the end of the data stream.
{
WORD up=pixel,down;          // start with the root node for 'pixel'
if(++curordinal > nofdata)           // end of data stream ? Terminate
  {
  bp.submit(up,nbits);
  return curordinal;
  }                 
         //Follow the string table and the data stream to the end of the
         //                               longest string that has a code

pixel=map(curordinal);  
while((down=findpixeloutlet(up,pixel))!=0)
  {
  up=down;
  if(++curordinal > nofdata)         // end of data stream ? Terminate
    {
    bp.submit(up,nbits);
    return curordinal;
    }
  pixel=map(curordinal);
  }
//               Submit 'up' which is the code of the longest string ...

bp.submit(up,nbits);
       //                ... and extend the string by appending 'pixel':
       //Create a successor node for 'pixel' whose code is 'freecode'...

pix[freecode]=pixel;
axon[freecode]=next[freecode]=0;
    //    ...and link it to the end of the chain emanating from axon[up].
    //Don't link it to the start instead: it would slow down performance.

down=axon[up];
if(!down)
  axon[up]=freecode;
else
  {
  while(next[down])
    down=next[down];
  next[down]=freecode;
  }
return curordinal;
} // gifcompressor::weiter
 


BYTE map(unsigned ordinal)
  //Maps the ordinal number in which a pixel should go into the GIF
  //to its actual location in the input array, and returns the pixel.
  //Use this version of map() if the pixel array is sorted in the
  //format expected by -.BMP files: left to right, bottom to top,
  //and every row padded with as many (0 to 3) trailing bytes as
  //needed to have its total length divisible by 4.
  //The other task of map() is the support of interlacing: See the
  //comment in map()#1.
{
unsigned line=((ordinal-1)/width); // Row number of bitmap 
                                // (with numeration starting with 0 from top)
unsigned col=ordinal-line*width;   // 1..width (numeration starting with 1)
if(interlaced)                  // line numbers (starting with 0) are mapped
                                // to different line numbers:
  {
  if(line+1<=div8)              // if line is among the first div8 rows...
    line*=8;                    // ...it is dealt with during pass 1
  else if (line+1<=div4)        // if line is among the first div4 rows...
    line=4+8*(line-div8);       // etc.
  else if (line+1<=div2)
    line=2+4*(line-div4);
  else
    line=1+2*(line-div2);
  }
//line=height-line-1;             // row numeration is converted to bottom up,
                                // starting with 0
return data[line * GDIwidth+col-1];
} // gifcompressor::map#2

CWriter& m_cWriter;
public:
gifcompressor(CWriter& a_cWriter) : bp(a_cWriter), m_cWriter(a_cWriter) {}

unsigned writedatablocks(BYTE const *pixeldata, unsigned nof, unsigned wi,WORD dd, bool il)
  //Encodes the pixel data and writes the "raster data"-section of the GIF
  //file, consisting of the "code size" byte followed by the counter-headed
  //data blocks, including the terminating zero block.
  //pixeldata  is an array of bytes containing one pixel each and sorted
  //           in the way explained in the map() functions. The first pixel
  //           is in pixeldata[0]
  //nof        total number of pixels in pixeldata (not counting any padding
  //           bytes)
  //wi         Number of pixels per row (not counting the padding)
  //dd         number of bits per pixel, where 2^dd is the size of the GIF's
  //           color tables. Allowed are 1..8. Used to determine 'nbits' and
  //           the number of root codes.
  //il         tells if interlacing is requested
  //returns:   The total number of bytes that have been written.

{
nofdata=nof;                // number of pixels in data stream
width=wi;                   // bitmap width and height in pixels
height=nofdata/width;
GDIwidth=wi;//(wi+3)/4;          // round 'width' upwards to next multiple of 4
//GDIwidth*=4;
interlaced=il;
div8=1+(height-1)/8;        // needed to support interlacing
div4=1+(height-1)/4;
div2=1+(height-1)/2;
data=pixeldata;             // the data array
curordinal=1;               // pixel #1 is next to be processed           
pixel=map(curordinal);      // get pixel #1
datadepth=dd;               // number of bits per data item (=pixel)
nbits=max(3,dd+1);          // initial size of compression codes
cc       = (1<<(nbits-1));  // 'cc' is the lowest code requiring 'nbits' bits
eoi      = cc+1;            // 'end-of-information'-code
freecode = (WORD)cc+2;    // code of the next entry to be added to the
                            // stringtable
CAutoVectorPtr<WORD> axonX(new WORD[4096]);
CAutoVectorPtr<WORD> nextX(new WORD[4096]);
CAutoVectorPtr<BYTE> pixX(new BYTE[4096]);
axon = axonX;
ZeroMemory(axon, sizeof(WORD)*4096);
next = nextX;
ZeroMemory(next, sizeof(WORD)*4096);
pix = pixX;
ZeroMemory(pix, sizeof(BYTE)*4096);
iniroots();                 // initialize the string table's root nodes
BYTE tmp = max(datadepth,2);
m_cWriter.Write(&tmp, 1); // Write what the GIF specification calls the
                            // "code size". Confusingly, this is the
                            // number of bits required to represent the pixel
                            // values. Allowed are 2,3,4,5,6,7,8.
bp.submit(cc,nbits);       // Submit one 'cc' as the first code
                 
while(1)
  {
  weiter();                 // generates the next code, submits it to 'bp' and
                            // updates 'curordinal'
  if(curordinal > nofdata)  // if reached the end of data stream:
    {
    bp.submit(eoi,nbits);  // submit 'eoi' as the last item of the code stream
    bp.writeflush();       // write remaining codes including this 'eoi' to
                            // the binary file
	tmp = 0; // write an empty data block to signal the end of "raster data" section in the file
	m_cWriter.Write(&tmp, 1);
    unsigned byteswritten = 2 + bp.byteswritten;
    return byteswritten;
    }
  if(freecode==(1U<<nbits)) // if the latest code added to the stringtable
                            // exceeds 'nbits' bits:
    nbits++;                // increase size of compression codes by 1 bit     
  freecode++;
  if(freecode==0xfff)      
    {                    
    flushstringtable();     // avoid stringtable overflow
    bp.submit(cc,nbits);   // tell the decoding software to flush its
                            // stringtable                 
    nbits=max(3,1+datadepth);     
    freecode=(WORD)cc+2;
    }
  } // while(1)
} // gifcompressor::writedatablocks

}; // class gifcompressor







//BYTE gifcompressor::map(unsigned ordinal)
////
////  Maps the ordinal number in which a pixel should go into the GIF
////  to its actual location in the input array, and returns the pixel.
////  Use this version of map if the pixel array is sorted left to right,
////  top to bottom. Watch out: it has never been tested. Its
////  implementation is trivial except when interlacing is switched on:
////  Then there are four passes through the image.
////  The first pass outputs rows 0,8,16,24,... , the second pass
////  rows 4,12,20,28,... , then comes 2,6,10,14,... and finally
////  rows 1,3,5,7,... . Note that the row numeration begins with 0.
////
//{
//if(!interlaced)
//  return data[ordinal-1];
//unsigned line,col;
//line=((ordinal-1)/width);     // Row number of bitmap 
//                              // (with numeration starting with 0 from top)
//col=ordinal-line*width;       // 1..width (numeration starting with 1)
//                              // line numbers (starting with 0) are mapped
//                              // to different line numbers:
//if(line+1<=div8)              // if line is among the first div8 rows...
//  line*=8;                    // ...it is dealt with during pass 1
//else if (line+1<=div4)        // if line is among the first div4 rows...
//  line=4+8*(line-div8);       // etc.
//else if (line+1<=div2)
//  line=2+4*(line-div4);
//else
//  line=1+2*(line-div2);
//return data[line * width+col-1];
//} // gifcompressor::map#1


