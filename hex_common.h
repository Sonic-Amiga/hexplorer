#ifndef HEX_COMMON
#define HEX_COMMON
#include"hexplorer.h"

#include"hash/MessageDigest.h"
#include"hash/sha.h"
#include"hash/ripemd.h"
#include"hash/md5.h"

#include"srecord-1.16/include/quit.h"
#include"srecord-1.16/include/srec/record.h"
#include"srecord-1.16/include/srec/input.h"
#include"srecord-1.16/include/srec/output.h"
#include"srecord-1.16/include/srec/input/file.h"

#include"srecord-1.16/include/srec/input/file/ascii_hex.h"
#include"srecord-1.16/include/srec/input/file/atmel_generic.h"
#include"srecord-1.16/include/srec/input/file/cosmac.h"
#include"srecord-1.16/include/srec/input/file/dec_binary.h"
#include"srecord-1.16/include/srec/input/file/emon52.h"
#include"srecord-1.16/include/srec/input/file/fairchild.h"
#include"srecord-1.16/include/srec/input/file/fastload.h"
#include"srecord-1.16/include/srec/input/file/formatted_binary.h"
#include"srecord-1.16/include/srec/input/file/four_packed_code.h"
#include"srecord-1.16/include/srec/input/file/intel.h"
#include"srecord-1.16/include/srec/input/file/mos_tech.h"
#include"srecord-1.16/include/srec/input/file/needham.h"
#include"srecord-1.16/include/srec/input/file/os65v.h"
#include"srecord-1.16/include/srec/input/file/signetics.h"
#include"srecord-1.16/include/srec/input/file/spasm.h"
#include"srecord-1.16/include/srec/input/file/spectrum.h"
#include"srecord-1.16/include/srec/input/file/srecord.h"
#include"srecord-1.16/include/srec/input/file/tektronix.h"
#include"srecord-1.16/include/srec/input/file/tektronix_extended.h"
#include"srecord-1.16/include/srec/input/file/ti_tagged.h"
#include"srecord-1.16/include/srec/input/file/wilson.h"

#include"srecord-1.16/include/srec/output/file.h"

#include"srecord-1.16/include/srec/output/file/ascii_hex.h"
#include"srecord-1.16/include/srec/output/file/atmel_generic.h"
#include"srecord-1.16/include/srec/output/file/cosmac.h"
#include"srecord-1.16/include/srec/output/file/dec_binary.h"
#include"srecord-1.16/include/srec/output/file/emon52.h"
#include"srecord-1.16/include/srec/output/file/fairchild.h"
#include"srecord-1.16/include/srec/output/file/fastload.h"
#include"srecord-1.16/include/srec/output/file/formatted_binary.h"
#include"srecord-1.16/include/srec/output/file/four_packed_code.h"
#include"srecord-1.16/include/srec/output/file/intel.h"
#include"srecord-1.16/include/srec/output/file/mos_tech.h"
#include"srecord-1.16/include/srec/output/file/needham.h"
#include"srecord-1.16/include/srec/output/file/os65v.h"
#include"srecord-1.16/include/srec/output/file/signetics.h"
#include"srecord-1.16/include/srec/output/file/spasm.h"
#include"srecord-1.16/include/srec/output/file/spectrum.h"
#include"srecord-1.16/include/srec/output/file/srecord.h"
#include"srecord-1.16/include/srec/output/file/tektronix.h"
#include"srecord-1.16/include/srec/output/file/tektronix_extended.h"
#include"srecord-1.16/include/srec/output/file/ti_tagged.h"
#include"srecord-1.16/include/srec/output/file/wilson.h"


//const double M_PI=3.14159265359;

#if BETA
void Info(char*);
void InfoDig(unsigned int);
void WyswietlBlad();
void Type(int);
void CutPaste(int);
void RunTest1(int);
void RunTest();
#endif
const char SEPARATOR=',';
//void HexDisassemble(char*,unsigned char*,int);
void GetModulePath(char*);
void CenterWindow(HWND);
void Dots(char*, int);
unsigned char DecryptHex(unsigned char);
void TextToHex(char* dest, unsigned char* src);
void BytesToHex(char* dest, unsigned char* src, int len);
void HexToText(unsigned char* dest, char* src, int size);
void ToUnicode(char*);
inline unsigned char ToHex4bits(unsigned char znak)
{
    znak &= 0x0F;
    switch(znak)
    {
        case 10: return 'A';
        case 11: return 'B';
        case 12: return 'C';
        case 13: return 'D';
        case 14: return 'E';
        case 15: return 'F';
        default: return znak + 48;
    }
}
BOOL CALLBACK NewValueDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK TimeDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, unsigned int, WPARAM, LPARAM);
inline void IntelMotorola(unsigned char*adr,int len)
{
  for(int i=0; i < len/2; i++)
  {
      int tmp;
      tmp = adr[i];
      adr[i] = adr[len-i-1];
      adr[len-i-1]=tmp;
  }

}
#endif
