#ifndef HEXPLORER
#define HEXPLORER
#define BETA 0

#include<windows.h>
#include<commctrl.h>
#include<shlobj.h>
#include<process.h>
#include<wininet.h>

extern "C"
{
#include"libdisasm/libdis.h"
}
extern int pg_offset;
extern bool strip_bit7;

INT_PTR CALLBACK SViewDlgProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK SimpleDataDlgProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK ChecksumDlgProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK OccurenceDlgProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK PixelDlgProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK AddStructDlgProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK AddMemberDlgProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK OptionsDlgProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK NavigatorDlgProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK FillDlgProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK FilePart(HWND, unsigned int, WPARAM, LPARAM);
void ScrollHexplorer(int pos = pg_offset);
void ScrollTo(int);
inline unsigned char ToHex4bits(unsigned char);
void SetStatus(char* info = 0);
void SetColumnNum(int);
bool Find(bool forward);

const int STD_OVERALLOC = 10000;
const int STD_BUF = 2000;

const int PANEL_POSITION_SIZE = 60+10
      -70;
const int PANEL_OFFSET = 20;

#endif
