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

BOOL CALLBACK SViewDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK SimpleDataDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK ChecksumDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK OccurenceDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK PixelDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK AddStructDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK AddMemberDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK OptionsDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK NavigatorDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK FillDlgProc(HWND, unsigned int, WPARAM, LPARAM);
BOOL CALLBACK FilePart(HWND, unsigned int, WPARAM, LPARAM);
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
