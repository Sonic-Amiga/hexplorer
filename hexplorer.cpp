/**********************************************************
Hexplorer source code.
Core written (C)2002-2003 by ICY.
**********************************************************/

#include"hexplorer.h"
#include"container.h"
#include"tool.h"
#include"position.h"
#include"undo.h"
#include"fonts.h"
#include"macro.h"
#include"filetype.h"
#include"mclip.h"
#include"hex_common.h"
#include"srec_win_quit.h"

#include"mediaaccess.h"

cPosition position[cPosition::MAX_POSITION];

ucContainer*pucc;
const char* szAppName = "Hexplorer v.2.17b";
char temp[3000];
HINSTANCE hInstance;
HWND h_main_wnd, h_client_wnd, status_bar, h_tool_bar, h_tool_tip;
int status_bar_size;
HANDLE plik;
HINTERNET hiVer, hiFile;
volatile int thread_num = 0;
int cxChar, cyChar, cxClient, cyClient;
int sys_COLOR_3DFACE, sys_COLOR_3DHILIGHT, sys_COLOR_3DSHADOW, sys_COLOR_BTNTEXT, sys_COLOR_WINDOW;
int selected_begin = 0;
int selected_end = 0;
int highlight_struct, highlight_struct_size;
HMENU hPopupRadixMenu, hPopupBPPMenu,
    hPopupCut;

srec_win_quit win_quitter(h_main_wnd, szAppName);

int column_num = 16;
bool auto_column = 0;
int column_group = 4;
int column_separators;

char nazwa_pliku[MAX_PATH] = "\0";
char nazwa_pliku_temp[MAX_PATH];
char title[MAX_PATH];
int dlugosc_pliku = 0;
int offset_pliku=0;
int pg_offset = 0;
unsigned char* pamiec = 0;
/*
class cPamiec
{
private:
  unsigned char*p;
public:
  cPamiec() : p(0) {}
  unsigned char operator[](__int64 addr) {return p+addr;}
};
cPamiec pamiec;
*/

int bytes_allocated = 0;
HGLOBAL hGlobal;
unsigned char* pGlobal;
unsigned char* schowek = 0;
int schoweklen = 0;

unsigned char hexFind [STD_BUF];
int hexFind_size = 0;
int hexFind_from;
bool hexFind_from_cursor = 0;
bool hexFind_forward = 1;

unsigned char hexReplace [STD_BUF];
int hexReplace_size = 0;
bool hexReplace_mode = 0;

unsigned char highlight[STD_BUF];
int highlight_size = 0;
unsigned char highlightbeta[STD_BUF];
int highlightbeta_size = 0;
unsigned char highlightgamma[STD_BUF];
int highlightgamma_size = 0;
unsigned char highlightdelta[STD_BUF];  // reserved
int highlightdelta_size = 0;

bool goto_dec = 0;
int goto_mode = 0;

bool shift = 0;
bool control = 0;
bool redraw = 1;
bool LMBpressed = 0;

//bool hex_addresses = 1;
bool split = 0;
bool insert = 1;
bool upper4 = 1;
char saved = 2;
bool always_ontop = 0;
bool hide_toolbar = 0;
bool align_structures = 1;
bool using_unicode=0;
// files allow inserting & deleting etc. , disks and RAM not; this is "semaphore" to many instructions
bool files=1;
cMediaAccess*dysk;
//unsigned __int64 disk_offset=0;  // unused
int BytesPerSector;
__int64 CurrentSector=0;
int SectorsLoaded=1;
__int64 TotalDiskSize;
char drive_name;

HMODULE dll_logo = 0;

HDC hdc;  // compromise...

Font font[] = {
    Font(CreateFont(0, 0, 0, 0, FW_BOLD&0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, FIXED_PITCH | FF_ROMAN, NULL)),
    Font((HFONT)GetStockObject(SYSTEM_FONT)),
    Font((HFONT)GetStockObject(SYSTEM_FIXED_FONT)),
    Font((HFONT)GetStockObject(DEFAULT_GUI_FONT)),
    Font((HFONT)GetStockObject(OEM_FIXED_FONT))
};

hexColors hColors[] =
{
    hexColors(4),
    hexColors(1),
    hexColors(2),
    hexColors(),
    hexColors(3),
    hexColors(5),
};

template<class typ>void _atox(HWND hwnd, typ t, bool fl)
{
  bool separators=true;
  ucContainer*pucc;
  pucc=new ucContainer;
  char txt[STD_BUF];
  *txt=0;
  char cyfra[2];
  typ liczba;
  OpenClipboard(hwnd);
  HGLOBAL hGlobal = GetClipboardData(CF_TEXT);
  unsigned char*pGlobal = (unsigned char*)GlobalLock(hGlobal);
  for(int i = 0; i < strlen((char*)pGlobal); i++)
      switch(pGlobal[i])
      {
          case '+': case '-': case 'e': case 'E': case '0':
          case '1': case '2': case '3': case '4': case '5':
          case '6': case '7': case '8': case '9': case '.':
            cyfra[0]=pGlobal[i];
            cyfra[1]=0;
            strcat(txt, cyfra);
            separators=false;
            break;
          default:
            if(!separators)
            {
              if(fl)
                liczba=atof(txt);
              else
                liczba=(typ)_atoi64(txt);
              pucc->Push((unsigned char*)&liczba, sizeof(typ));
              strcpy(txt, "");
              separators=true;
            }
      }
              if(fl)
                liczba=atof(txt);
              else
                liczba=(typ)_atoi64(txt);
              pucc->Push((unsigned char*)&liczba, sizeof(typ));
  pucc->CopyToClipboard(hwnd);
  delete pucc;
  GlobalUnlock(hGlobal);
  CloseClipboard();
}

class cRecent
{
private:
    const static int MAX_RECENT = 15;
    char nazwa[MAX_PATH];
    int pg_offset;
    int selected_begin, selected_end;
    bool split, insert;
    cPosition position[cPosition::MAX_POSITION];
    void RememberEditingPosition();
    void RestoreEditingPosition();
    char* Read(char* b);
    void Write(HANDLE plik);
    void SetHexplorerTitle();
public:
    static int recent_num;
    static int actual;
    static cRecent pliki[MAX_RECENT];
    static void SaveFileQuery();
    void SetMenu(HMENU hMenu, int pos);
    void Open();
    static void Add(char* n);
    static void ReadAll(char* bufor, int rozmiar);
    static void WriteAll(HANDLE plik);
};
int cRecent::recent_num = 0;
int cRecent::actual = -1;
cRecent cRecent::pliki[MAX_RECENT];

void cRecent::RememberEditingPosition()
{
    if(actual != -1)   // != Unnamed file
    {
        pg_offset = ::pg_offset;
        selected_begin = ::selected_begin;
        selected_end = ::selected_end;
        split = ::split;
        insert = ::insert;
        for(int i = 0; i < cPosition::MAX_POSITION; i++)
            position[i].Set(::position[i].pos, ::position[i].description);
    }
}

void cRecent::RestoreEditingPosition()
{
    ::pg_offset = pg_offset;
    if(selected_begin < dlugosc_pliku)
        ::selected_begin = selected_begin;
    else
        ::selected_begin = dlugosc_pliku?dlugosc_pliku-1:0;
    if(selected_end < dlugosc_pliku)
        ::selected_end = selected_end;
    else
        ::selected_end = dlugosc_pliku?dlugosc_pliku-1:0;
    ::split = split;
    ::insert = insert;
    for(int i = 0; i < cPosition::MAX_POSITION; i++)
        ::position[i].Set(position[i].pos, position[i].description);
}

char* cRecent::Read(char* b)
{
    pg_offset = *(int*)b;    b += sizeof (int);
    selected_begin = *(int*)b;    b += sizeof(int);
    selected_end = *(int*)b;    b += sizeof(int);
    split = *(bool*)b;    b += sizeof(bool);
    insert = *(bool*)b;    b += sizeof(bool);
    for(int i = 0; i < cPosition::MAX_POSITION; i++)
    {
        position[i].Set(*(int*)b, b + sizeof(int)); b += sizeof(int); b += strlen(b) + 1;
    }
    strcpy(nazwa, b);
    b += strlen(b) + 1;
    return b;
}

void cRecent::Write(HANDLE plik)
{
    int br;
    WriteFile(plik, &pg_offset, sizeof(int), &(DWORD)br, NULL);
    WriteFile(plik, &selected_begin, sizeof(int), &(DWORD)br, NULL);
    WriteFile(plik, &selected_end, sizeof(int), &(DWORD)br, NULL);
    WriteFile(plik, &split, sizeof(bool), &(DWORD)br, NULL);
    WriteFile(plik, &insert, sizeof(bool), &(DWORD)br, NULL);
    for(int i = 0; i < cPosition::MAX_POSITION; i++)
    {
        WriteFile(plik, &position[i].pos, sizeof(int), &(DWORD)br, NULL);
        WriteFile(plik, position[i].description, strlen(position[i].description) + 1, &(DWORD)br, NULL);
    }
    WriteFile(plik, nazwa, strlen(nazwa) + 1, &(DWORD)br, NULL);
}

void cRecent::SetHexplorerTitle()
{
    char title_text [STD_BUF] = "ICY Hexplorer - [";
    strcat(title_text, nazwa);
    strcat(title_text, "]");
    SetWindowText(h_main_wnd, title_text);
}

void cRecent::SaveFileQuery()
{
    if(!saved && MessageBox(h_main_wnd, "Save current file?", szAppName, MB_YESNO | MB_ICONQUESTION) == IDYES)
        SendMessage(h_client_wnd, WM_COMMAND, 104, 0);
}

void cRecent::SetMenu(HMENU hMenu, int pos)
{
    MENUITEMINFO mio;
    mio.cbSize = sizeof(MENUITEMINFO);
    mio.fMask = MIIM_TYPE;
    mio.fType = MFT_STRING;
    mio.dwTypeData = nazwa;
    mio.cch = strlen(nazwa);
    if(mio.cch > 40)
    {
        mio.dwTypeData += mio.cch - 40;
        mio.cch = 40;
    }
    EnableMenuItem(hMenu, 200 + pos, MF_ENABLED);
    SetMenuItemInfo(hMenu, 200 + pos, FALSE, &mio);
}

void cRecent::Open()
{
    strcpy(nazwa_pliku, nazwa);
    SendMessage(h_client_wnd, WM_COMMAND, 108, 0);
    ScrollHexplorer();
}

void cRecent::Add(char* n)
{
    pliki[actual].RememberEditingPosition();
    for(int i = 0; i < recent_num; i++)
        if(!_stricmp(pliki[i].nazwa, n))
        {
            actual = recent_num - 1;
            cRecent temp = pliki[i];
            for(int k = i + 1; k < recent_num; k++)
                pliki[k - 1] = pliki[k];
            pliki[actual] = temp;
            pliki[actual].RestoreEditingPosition();
            pliki[actual].SetHexplorerTitle();
            return;
        }
    if(recent_num == MAX_RECENT)
    {
        recent_num = MAX_RECENT - 1;
        for(int i = 1; i < MAX_RECENT; i++)
            pliki[i - 1] = pliki[i];
    }
    actual = recent_num;
    strcpy(pliki[recent_num].nazwa, n);
    pliki[recent_num].selected_begin = pliki[recent_num].selected_end = pliki[recent_num].pg_offset = 0;
    for(int i = 0; i < cPosition::MAX_POSITION; i++)
        pliki[recent_num].position[i].Reset();

    pliki[recent_num].RestoreEditingPosition();
    pliki[recent_num++].SetHexplorerTitle();
}

void cRecent::ReadAll(char* bufor, int rozmiar)
{
    recent_num = 0;
    char* b = bufor;
    while(b - bufor < rozmiar)
        b = pliki[recent_num++].Read(b);
}

void cRecent::WriteAll(HANDLE plik)
{
    pliki[actual].RememberEditingPosition();
    for(int i = 0; i < cRecent::recent_num; i++)
        cRecent::pliki[i].Write(plik);
}

enum fill_mode
{
    fFILL,
    fXOR,
    fAND,
    fOR
} fmode;

bool Find(bool forward)
{
    int shift_forward[256];
    for(int i = 0; i < 256; i++)
        shift_forward[i] = hexFind_size;
    for(int i = 0; i < hexFind_size; i++)
        shift_forward[hexFind[i]] = hexFind_size - i - 1;
    int shift_backward[256];
    for(int i = 0; i < 256; i++)
        shift_backward[i] = hexFind_size;
    for(int i = hexFind_size - 1; i >= 0; i--)
        shift_backward[hexFind[i]] = i;

    unsigned char *p = pamiec + (forward ? hexFind_from + hexFind_size - 1: hexFind_from - hexFind_size - 1);
    unsigned char *end = pamiec + dlugosc_pliku;
    if(dlugosc_pliku && hexFind_size && p >= pamiec)
    {
        if(forward)
            for(int j =  hexFind_size - 1; j >= 0; j--, p--)
                while(*p != hexFind[j])
                {
                    int x = shift_forward[*p];
                    if(hexFind_size - j > x)
                        p += hexFind_size - j;
                    else
                        p += x;
                    if(p>=end)
                        return 0;
                    j = hexFind_size - 1;
                }
        else
            for(int j = 0; j < hexFind_size; j++, p++)
                while(*p != hexFind[j])
                {
                    int x = shift_backward[*p];
                    if(j + 1 > x)
                        p -= j + 1;
                    else
                        p -= x;
                    if(p<pamiec)
                        return 0;
                    j = 0;
                }
        if(forward)
            p++;
        else
            p-=hexFind_size;
        selected_begin = p - pamiec;
        selected_end = selected_begin + hexFind_size - 1;
        if(redraw)
        {
            ScrollTo(selected_begin);
            InvalidateRect(h_client_wnd, NULL, 0);
            SetStatus();
        }
        hexFind_from = selected_begin + (hexReplace_mode ? hexReplace_size : hexFind_size);
    }
    else
        return 0;
    return 1;
}

BOOL CALLBACK FindDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    char hex[STD_BUF];
    cPosition p[200];
    switch (message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            SetWindowText(GetDlgItem(hDlg, 1000), (char*)hexFind);
            CheckRadioButton(hDlg, 1010, 1011, 1010 + hexFind_forward);
            CheckRadioButton(hDlg, 1012, 1013, 1012 + hexFind_from_cursor);
            SetFocus(GetDlgItem(hDlg, 1000));
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    GetWindowText(GetDlgItem(hDlg, 1001), hex, 200);
                    hexFind_size = strlen(hex) / 2;
                    HexToText(hexFind, hex, hexFind_size);
                    hexFind_forward = (IsDlgButtonChecked(hDlg, 1010) != BST_CHECKED);
                    hexFind_from_cursor = (IsDlgButtonChecked(hDlg, 1012) != BST_CHECKED);
                    if(hexFind_from_cursor)
                        hexFind_from = selected_begin;
                    else
                        hexFind_from = 0;
                    MultiClipboard::Copy(hexFind, hexFind_size);
                    EndDialog(hDlg, 1);
                    return TRUE;
                case 1005:
                    GetWindowText(GetDlgItem(hDlg, 1001), hex, 200);
                    hexFind_size = strlen(hex) / 2;
                    HexToText(hexFind, hex, hexFind_size);
                    hexFind_forward = (IsDlgButtonChecked(hDlg, 1010) != BST_CHECKED);
                    hexFind_from_cursor = (IsDlgButtonChecked(hDlg, 1012) != BST_CHECKED);

                    if(hexFind_from_cursor)
                        hexFind_from = selected_begin;
                    else
                        hexFind_from = 0;

                    hexReplace_mode = 0;

                    int n;
                    n = 0;
                    redraw = 0;
                    while(Find(hexFind_forward) && n < 200)
                    {
                        p[n].Set(selected_begin, "\0");
                        n++;
                    }
                    redraw = 1;
                    ChildDlg::svd[ChildDlg::num] = new NavigatorFindDlg(p, n, hex);
                    if(align_structures)
                        ChildDlg::AlignAll();

                    MultiClipboard::Copy(hexFind, hexFind_size);

                    EndDialog(hDlg, 0);
                    return FALSE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
                case 1000:
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        char hex[STD_BUF];
                        GetWindowText(GetDlgItem(hDlg, 1000), (char*)hexFind, 200);
                        TextToHex(hex, hexFind);
                        //ToUnicode(hex);
                        SetWindowText(GetDlgItem(hDlg, 1001), hex);
                    }
                    return FALSE;
            }
    }
    return FALSE;
}

BOOL CALLBACK ReplaceDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    static bool found = 1;
    char hex[STD_BUF];
    char hex2[STD_BUF];
    switch (message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            SetWindowText(GetDlgItem(hDlg, 1000), (char*)hexFind);
            SetWindowText(GetDlgItem(hDlg, 1008), (char*)hexReplace);
            CheckRadioButton(hDlg, 1010, 1011, 1010 + hexFind_forward);
            CheckRadioButton(hDlg, 1012, 1013, 1012 + hexFind_from_cursor);
            SetFocus(GetDlgItem(hDlg, 1000));
            if(hexFind_from_cursor)
                hexFind_from = selected_begin;
            else
                hexFind_from = 0;
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    GetWindowText(GetDlgItem(hDlg, 1001), hex, 200);
                    hexFind_size = GetWindowTextLength(GetDlgItem(hDlg, 1001)) >> 1;
                    HexToText(hexFind, hex, hexFind_size);

                    hexFind_forward = (IsDlgButtonChecked(hDlg, 1010) != BST_CHECKED);
                    hexFind_from_cursor = (IsDlgButtonChecked(hDlg, 1012) != BST_CHECKED);

                    found = 1;
                    hexReplace_mode = 1;
                    if(!Find(hexFind_forward))
                    {
                       MessageBox(hDlg, "End search.", szAppName, 0);
                       found = 0;
                    }
                    return TRUE;
                case 1006:
                    GetWindowText(GetDlgItem(hDlg, 1009), hex2, 200);
                    hexReplace_size = GetWindowTextLength(GetDlgItem(hDlg, 1009)) >> 1;
                    HexToText(hexReplace, hex2, hexReplace_size);
                    delete [] schowek;
                    schowek = new unsigned char[hexReplace_size];
                    CopyMemory(schowek, hexReplace, hexReplace_size);
                    schoweklen = hexReplace_size;

                    SendMessage(hDlg, WM_COMMAND, IDOK, 0);
                    if(!found)
                        return TRUE;
                    SendMessage(h_client_wnd, WM_USER + 1, 0, 0);
                    return TRUE;
                case 1007:
                    char pszcount[STD_BUF/10];
                    int icount;
                    SetCursor(LoadCursor(NULL, IDC_WAIT));

                    icount = 0;
                    redraw = 0;
                    do
                    {
                        SendMessage(hDlg, WM_COMMAND, 1006, 0);
                        icount++;
                        _ui64toa(icount, pszcount, 10);
                        strcat(pszcount, " replaced...");
                        SetStatus(pszcount);
                    }
                    while(found);
                    redraw = 1;
                    ScrollHexplorer();
                    SetStatus();
                    InvalidateRect(h_client_wnd, NULL, 0);
                    return TRUE;
                case IDCANCEL:
                    //MultiClipboard::Copy(hexFind, hexFind_size);
                    //MultiClipboard::Copy(hexReplace, hexReplace_size);
                    EndDialog(hDlg, 0);
                    return FALSE;
                case 1000:
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        char hex[STD_BUF];
                        GetWindowText(GetDlgItem(hDlg, 1000), (char*)hexFind, 200);
                        TextToHex(hex, hexFind);
                        SetWindowText(GetDlgItem(hDlg, 1001), hex);
                    }
                    return TRUE;
                case 1008:
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        char hex[STD_BUF];
                        GetWindowText(GetDlgItem(hDlg, 1008), (char*)hexReplace, 200);
                        TextToHex(hex, hexReplace);
                        SetWindowText(GetDlgItem(hDlg, 1009), hex);
                    }
                    return TRUE;
                case 1013:
                    if(HIWORD(wParam) == BN_CLICKED)
                        hexFind_from = selected_begin;
                    return TRUE;
                case 1012:
                    if(HIWORD(wParam) == BN_CLICKED)
                        hexFind_from = 0;
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

BOOL CALLBACK FillDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            SetFocus(GetDlgItem(hDlg, 1000));
            SetWindowText(GetDlgItem(hDlg, 1000), (char*)schowek);
            switch(fmode)
            {
                case fFILL:  SetWindowText(hDlg, "Fill Selection with"); break;
                case fXOR:  SetWindowText(hDlg, "XOR Selection with"); break;
                case fOR:  SetWindowText(hDlg, "OR Selection with"); break;
                case fAND:  SetWindowText(hDlg, "AND Selection with"); break;
            }
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    SetCursor(LoadCursor(NULL, IDC_WAIT));
                    EndDialog(hDlg, 0);
                    {
                        bool temp_ins = insert;
                        insert = 0;
                        cUndo::undo[cUndo::step] = new cUndo(WM_COMMAND, 125, "Fill Selection");
                        insert = temp_ins;
                    }

                    unsigned char tekst[STD_BUF];
                    char hex[STD_BUF];
                    int crep, rep_len;
                    GetWindowText(GetDlgItem(hDlg, 1001), hex, 200);
                    rep_len = GetWindowTextLength(GetDlgItem(hDlg, 1001)) >> 1;
                    HexToText(tekst, hex, rep_len);

                    hGlobal = GlobalAlloc(GHND | GMEM_SHARE, rep_len + 1);
                    pGlobal = (unsigned char*)GlobalLock(hGlobal);
                    CopyMemory(pGlobal, tekst, rep_len);
                    GlobalUnlock(hGlobal);
                    OpenClipboard(h_main_wnd);
                    EmptyClipboard();
                    SetClipboardData(CF_TEXT, hGlobal);
                    CloseClipboard();
                    delete [] schowek;
                    schowek = new unsigned char[rep_len];
                    CopyMemory(schowek, tekst, rep_len);
                    schoweklen = rep_len;

                    switch(fmode)
                    {
                        case fFILL:
                            for(int i = selected_begin; i <= selected_end; i++)
                                pamiec[i] = schowek[(i - selected_begin)%schoweklen];
                            break;
                        case fXOR:
                            for(int i = selected_begin; i <= selected_end; i++)
                                pamiec[i] ^= schowek[(i - selected_begin)%schoweklen];
                            break;
                        case fOR:
                            for(int i = selected_begin; i <= selected_end; i++)
                                pamiec[i] |= schowek[(i - selected_begin)%schoweklen];
                            break;
                        case fAND:
                            for(int i = selected_begin; i <= selected_end; i++)
                                pamiec[i] &= schowek[(i - selected_begin)%schoweklen];
                            break;
                    }
                    saved = 0;
                    MultiClipboard::Copy(tekst, rep_len);
                    ScrollHexplorer();
                    SetStatus();
                    InvalidateRect(h_client_wnd, NULL, 0);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
                case 1000:
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        char hex[STD_BUF];
                        GetWindowText(GetDlgItem(hDlg, 1000), (char*)hexFind, 200);
                        TextToHex(hex, hexFind);
                        SetWindowText(GetDlgItem(hDlg, 1001), hex);
                    }
            }
            break;
    }
    return FALSE;
}

BOOL CALLBACK DiskDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
  char txt[STD_BUF];
    switch (message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            SetFocus(GetDlgItem(hDlg, 1000));
            _ui64toa(CurrentSector, txt, 10);
            SetWindowText(GetDlgItem(hDlg, 1000), txt);
            _itoa(SectorsLoaded, txt, 10);
            SetWindowText(GetDlgItem(hDlg, 1001), txt);
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    GetWindowText(GetDlgItem(hDlg, 1000), txt, STD_BUF);
                    CurrentSector=_atoi64(txt);
                    GetWindowText(GetDlgItem(hDlg, 1001), txt, STD_BUF);
                    SectorsLoaded=atoi(txt);
                    EndDialog(hDlg, 1);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
                case 1000:
                    if(HIWORD(wParam) == EN_CHANGE)
                    {

                    }
            }
            break;
    }
    return FALSE;
}

BOOL CALLBACK ChainDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            SetFocus(GetDlgItem(hDlg, 1000));
            SetWindowText(GetDlgItem(hDlg, 1000), (char*)schowek);
            SetWindowText(GetDlgItem(hDlg, 1002), "1");
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    SetCursor(LoadCursor(NULL, IDC_WAIT));
                    EndDialog(hDlg, 0);
                    unsigned char tekst[STD_BUF];
                    char hex[STD_BUF];
                    char srep[STD_BUF];
                    int crep, rep_len;
                    GetWindowText(GetDlgItem(hDlg, 1001), hex, 200);
                    GetWindowText(GetDlgItem(hDlg, 1002), srep, 200);
                    rep_len = GetWindowTextLength(GetDlgItem(hDlg, 1001)) >> 1;
                    HexToText(tekst, hex, rep_len);
                    crep = _atoi64(srep);

                    hGlobal = GlobalAlloc(GHND | GMEM_SHARE, rep_len + 1);
                    pGlobal = (unsigned char*)GlobalLock(hGlobal);
                    CopyMemory(pGlobal, tekst, rep_len);
                    GlobalUnlock(hGlobal);
                    OpenClipboard(h_main_wnd);
                    EmptyClipboard();
                    SetClipboardData(CF_TEXT, hGlobal);
                    CloseClipboard();
                    delete [] schowek;
                    schowek = new unsigned char[rep_len];
                    CopyMemory(schowek, tekst, rep_len);
                    schoweklen = rep_len;
                    SetStatus("Pasting chain...");
                    cUndo::redirected_by_undo = 1;
                    redraw = 0;
                    bool temp_ins; temp_ins = insert;  insert = 1;
                    int temp_sb; temp_sb = selected_begin;
                    for(int i = 0; i < crep; i++)
                    {
                        SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                        selected_begin = selected_end + 1;
                    }
                    redraw = 1;
                    cUndo::redirected_by_undo = 0;
                    selected_begin = temp_sb;
                    selected_end = selected_begin + crep * rep_len - 1;
                    cUndo::undo[cUndo::step] = new cUndo(WM_COMMAND, 125, "Paste Chain");
                    insert = temp_ins;
                    MultiClipboard::Copy(tekst, rep_len);
                    ScrollHexplorer();
                    SetStatus();
                    InvalidateRect(h_client_wnd, NULL, 0);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
                case 1000:
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        char hex[STD_BUF];
                        GetWindowText(GetDlgItem(hDlg, 1000), (char*)hexFind, 200);
                        TextToHex(hex, hexFind);
                        SetWindowText(GetDlgItem(hDlg, 1001), hex);
                    }
                    return FALSE;
                case 1001: case 1002:
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        char srep[STD_BUF];
                        int len;
                        GetWindowText(GetDlgItem(hDlg, 1002), srep, 200);
                        len = _atoi64(srep);
                        len *= GetWindowTextLength(GetDlgItem(hDlg, 1001))>>1;
                        _itoa(len, srep, 10);
                        strcat(srep, " byte(s)");
                        SetWindowText(GetDlgItem(hDlg, 1005), srep);
                    }
                    return FALSE;
            }
            break;
    }
    return FALSE;
}

BOOL CALLBACK GoToDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    char bufor[10];
    switch (message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            SetFocus(GetDlgItem(hDlg, 1000));
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Remember as postition #0");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Remember as postition #1");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Remember as postition #2");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Remember as postition #3");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Remember as postition #4");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Remember as postition #5");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Remember as postition #6");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Remember as postition #7");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Remember as postition #8");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Remember as postition #9");
            CheckRadioButton(hDlg, 1005, 1006, 1005 + goto_dec);
            CheckRadioButton(hDlg, 1007, 1010, 1007 + goto_mode);
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    char hex[STD_BUF];
                    int savegoto;
                    int bin, mul, len, base;
                    GetWindowText(GetDlgItem(hDlg, 1000), hex, 200);
                    if(IsDlgButtonChecked(hDlg, 1005) == BST_CHECKED)
                        base = 16;
                    else
                        base = 10;
                    bin = 0;
                    mul = 1;
                    len = strlen(hex);
                    for(int i = 0; i < len; i++)
                    {
                        bin += (DecryptHex(hex[len - i - 1])) * mul;
                        mul *= base;
                    }
                    if(IsDlgButtonChecked(hDlg, 1008) == BST_CHECKED)
                        bin = dlugosc_pliku - bin - 1;
                    else if(IsDlgButtonChecked(hDlg, 1009) == BST_CHECKED)
                        bin = selected_begin + bin;
                    else if(IsDlgButtonChecked(hDlg, 1010) == BST_CHECKED)
                        bin = selected_begin - bin;

                    int diff;
                    diff = selected_end - selected_begin;

                    if(bin + diff + 1 > dlugosc_pliku)
                        bin = dlugosc_pliku - diff - 1;
                    if(bin < 0)
                        bin = 0;

                    ScrollTo(bin);

                    selected_begin = bin;
                    selected_end = selected_begin + diff;
                    hexFind_from = selected_begin;
                    savegoto = SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);
                    GetWindowText(GetDlgItem(hDlg, 1002), hex, 200);
                    if(savegoto != CB_ERR)
                        position[savegoto].Set(bin, hex);

                    InvalidateRect(h_client_wnd, NULL, 0);
                    SetStatus();
                    EndDialog(hDlg, 0);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
                case 1665: // From cursor
                    BytesToHex(bufor, pamiec+selected_end+3, 1);
                    BytesToHex(bufor+2, pamiec+selected_end+2, 1);
                    BytesToHex(bufor+4, pamiec+selected_end+1, 1);
                    BytesToHex(bufor+6, pamiec+selected_end, 1);
                    SetWindowText(GetDlgItem(hDlg, 1000), bufor);
                    CheckRadioButton(hDlg, 1005, 1006, 1005);
                    goto_dec=0;
                    return TRUE;
                case 1005:
                    if(HIWORD(wParam) == BN_CLICKED)
                        goto_dec = 0;
                    return TRUE;
                case 1006:
                    if(HIWORD(wParam) == BN_CLICKED)
                        goto_dec = 1;
                    return TRUE;
                case 1007:
                    if(HIWORD(wParam) == BN_CLICKED)
                        goto_mode = 0;
                    return TRUE;
                case 1008:
                    if(HIWORD(wParam) == BN_CLICKED)
                        goto_mode = 1;
                    return TRUE;
                case 1009:
                    if(HIWORD(wParam) == BN_CLICKED)
                        goto_mode = 2;
                    return TRUE;
                case 1010:
                    if(HIWORD(wParam) == BN_CLICKED)
                        goto_mode = 3;
                    return TRUE;
            }
            return TRUE;
    }
    return FALSE;
}

BOOL CALLBACK HighlightDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            char str[STD_BUF];
            SetFocus(GetDlgItem(hDlg, 1000));
            SetWindowText(GetDlgItem(hDlg, 1000), (char*)schowek);
            strcpy(str, "String Alpha (so far: ");
            if(highlight_size)
            {
                str[strlen(str)+ highlight_size] = 0;
                memcpy(str + strlen(str), highlight, highlight_size);
            }
            else
                strcat(str, "empty");
            strcat(str, ")");
            SendMessage(GetDlgItem(hDlg, 1005), CB_ADDSTRING, 0, (LPARAM)str);
            strcpy(str, "String Beta (so far: ");
            if(highlightbeta_size)
            {
                str[strlen(str)+ highlightbeta_size] = 0;
                memcpy(str + strlen(str), highlightbeta, highlightbeta_size);
            }
            else
                strcat(str, "empty");
            strcat(str, ")");
            SendMessage(GetDlgItem(hDlg, 1005), CB_ADDSTRING, 0, (LPARAM)str);
            strcpy(str, "String Gamma (so far: ");
            if(highlightgamma_size)
            {
                str[strlen(str)+ highlightgamma_size] = 0;
                memcpy(str + strlen(str), highlightgamma, highlightgamma_size);
            }
            else
                strcat(str, "empty");
            strcat(str, ")");
            SendMessage(GetDlgItem(hDlg, 1005), CB_ADDSTRING, 0, (LPARAM)str);
            SendMessage(GetDlgItem(hDlg, 1005), CB_SETCURSEL, 0, 0);
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    char hex[STD_BUF];
                    unsigned char bytes[STD_BUF];
                    int size;
                    size = GetWindowTextLength(GetDlgItem(hDlg, 1001)) >> 1;
                    GetWindowText(GetDlgItem(hDlg, 1001), hex, 200);
                    HexToText(bytes, hex, size);
                    MultiClipboard::Copy(bytes, size);
                    switch(SendMessage(GetDlgItem(hDlg, 1005), CB_GETCURSEL, 0, 0))
                    {
                        case 0:
                            highlight_size = size;
                            CopyMemory(highlight, bytes, size);
                            //HexToText(highlight, hex, highlight_size);
                            break;
                        case 1:
                            highlightbeta_size = size;
                            CopyMemory(highlightbeta, bytes, size);
                            //HexToText(highlightbeta, hex, highlightbeta_size);
                            break;
                        case 2:
                            highlightgamma_size = size;
                            CopyMemory(highlightgamma, bytes, size);
                            //HexToText(highlightgamma, hex, highlightgamma_size);
                            break;
                    }
                    EndDialog(hDlg, 0);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
                case 1000:
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        char hex[STD_BUF];
                        GetWindowText(GetDlgItem(hDlg, 1000), (char*)hexFind, 200);
                        TextToHex(hex, hexFind);
                        SetWindowText(GetDlgItem(hDlg, 1001), hex);
                    }
                    return FALSE;
            }
    }
    return FALSE;
}

LRESULT CALLBACK OknoGlowne(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    int br, toolbar_size;
    static char module_name[MAX_PATH];
    static WINDOWPLACEMENT wndpl;
    switch(message)
    {
        case WM_CREATE:
        {
            char b[10000];
            char* bufor;    bufor = b;
            int file_size;
            //GetModuleFileName(NULL, module_name, MAX_PATH - 1);
            //strcpy(module_name + strlen(module_name) - 13, "hexplorer.dat");
    GetModulePath(module_name);
    strcat(module_name, "hexplorer.dat");

            plik = CreateFile(module_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if(plik != INVALID_HANDLE_VALUE)
            {
                file_size = GetFileSize(plik, NULL);
                ReadFile(plik, bufor, file_size, &(DWORD)br, NULL);
                CloseHandle(plik);
                Font::actual = *(int*)bufor;    bufor += sizeof(int);
                hexColors::scheme = *(char*)bufor;    bufor += sizeof(char);

                column_num = *(int*)bufor;    bufor += sizeof(int);
                auto_column = *(bool*)bufor;    bufor += sizeof(bool);
                column_group = *(int*)bufor;    bufor += sizeof(int);
                using_unicode = *(bool*)bufor;    bufor += sizeof(bool);

                always_ontop = *(bool*)bufor;    bufor += sizeof(bool);
                hide_toolbar = *(bool*)bufor;    bufor += sizeof(bool);
                hexFind_forward = *(bool*)bufor;    bufor += sizeof(bool);
                hexFind_from_cursor = *(bool*)bufor;    bufor += sizeof(bool);
                align_structures = *(bool*)bufor;    bufor += sizeof(bool);
                highlight_size = *(int*)bufor;    bufor += sizeof(int);
                CopyMemory(highlight, bufor, highlight_size);    bufor += highlight_size;
                highlightbeta_size = *(int*)bufor;    bufor += sizeof(int);
                CopyMemory(highlightbeta, bufor, highlightbeta_size);    bufor += highlightbeta_size;
                highlightgamma_size = *(int*)bufor;    bufor += sizeof(int);
                CopyMemory(highlightgamma, bufor, highlightgamma_size);    bufor += highlightgamma_size;
                goto_dec = *(bool*)bufor;    bufor += sizeof(bool);
                goto_mode = *(int*)bufor;    bufor += sizeof(int);

                CopyMemory(&wndpl, bufor, sizeof(WINDOWPLACEMENT));    bufor += sizeof(WINDOWPLACEMENT);
                cRecent::ReadAll(bufor, file_size - (bufor - b));
            }
            else
            {
                wndpl.length = sizeof(WINDOWPLACEMENT);
                wndpl.flags = 0;
                wndpl.showCmd = SW_SHOWNORMAL;
                wndpl.ptMinPosition.x = wndpl.ptMinPosition.y = wndpl.ptMaxPosition.x = wndpl.ptMaxPosition.y = 0;
                wndpl.rcNormalPosition.left = wndpl.rcNormalPosition.top = 0;
                wndpl.rcNormalPosition.right = 640;
                wndpl.rcNormalPosition.bottom = 344;
            }
            hColors[hexColors::scheme].Set();
            cFileType::ReadAll();
            MultiClipboard::ReadAll();
            for(int i = 0; i < cPosition::MAX_POSITION; i++)
                position[i].Reset();
            ChecksumDlg::InitCRC32();
        }
        case WM_SYSCOLORCHANGE:
            sys_COLOR_3DFACE = GetSysColor(COLOR_3DFACE);
            sys_COLOR_3DHILIGHT = GetSysColor(COLOR_3DHILIGHT);
            sys_COLOR_3DSHADOW = GetSysColor(COLOR_3DSHADOW);
            sys_COLOR_BTNTEXT = GetSysColor(COLOR_BTNTEXT);
            sys_COLOR_WINDOW = GetSysColor(COLOR_WINDOW);
            return 0;
        case WM_USER:   // Sent after main window has been shown
            structure::ReadAll();
            SetWindowPlacement(hwnd, &wndpl);
            macro::ReadAll();
            DrawMenuBar(h_main_wnd);
            SetColumnNum(column_num);
            if(always_ontop)
                SetWindowPos(h_main_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            return 0;
        case WM_SIZE:
            int cxClient, cyClient;
            cxClient = LOWORD(lParam);
            if(cxClient < 52 + 20 + (cxChar << 5))
                cxClient = 52 + 20 + (cxChar << 5);
            cyClient = HIWORD(lParam);
            cyClient -= status_bar_size;
            if(!hide_toolbar)
            {
                toolbar_size = 45;
                cyClient -= toolbar_size;
                MoveWindow(h_tool_bar, 0, 0, cxClient, 45, 1);
            }
            else
            {
                toolbar_size = 0;
                MoveWindow(h_tool_bar, 0, -50, cxClient, 45, 1);
            }
            MoveWindow(h_client_wnd, 0, toolbar_size, cxClient, cyClient, 1);
            SendMessage(status_bar, WM_SIZE, 0, 0);
        case WM_MOVING:
            if(align_structures)
                SViewerDlg::AlignAll();
            return 0;
        case WM_INITMENUPOPUP:
            if(lParam == 0)
            {
                if(thread_num)
                    MessageBox(hwnd, "File menu not available, while some background threads are still working...", szAppName, MB_OK);
                EnableMenuItem((HMENU)wParam, 113, nazwa_pliku[0]==0?MF_GRAYED:MF_ENABLED);
                for(int i = 0; i < cRecent::recent_num; i++)
                    cRecent::pliki[i].SetMenu((HMENU)wParam, i);
                return 0;
            }
            else if(lParam == 1)
            {
              for(int i=2302; i<=2304; i++)
                EnableMenuItem((HMENU)wParam, i, files?MF_GRAYED:MF_ENABLED);
            }
            else if(lParam == 1+1)
            {
                if(cUndo::step)
                {
                    EnableMenuItem((HMENU)wParam, 121, MF_ENABLED);
                    cUndo::undo[cUndo::step - 1]->SetMenu((HMENU)wParam, 121);
                }
                else
                {
                    EnableMenuItem((HMENU)wParam, 121, MF_GRAYED);
                    cUndo temp(0, 0, "");
                    temp.SetMenu((HMENU)wParam, 121);
                }
                EnableMenuItem((HMENU)wParam, 125, schoweklen?MF_ENABLED:MF_GRAYED);
                if(IsClipboardFormatAvailable(CF_TEXT))
                {
                  for(int i=174; i<=180; i++)
                    EnableMenuItem((HMENU)wParam, i, MF_ENABLED);
                }
                else
                {
                  for(int i=174; i<=180; i++)
                    EnableMenuItem((HMENU)wParam, i, MF_GRAYED);
                }
                for(int i = 0; i < MultiClipboard::total; i++)
                    MultiClipboard::vc[i]->SetMenu((HMENU)wParam);
            }
            else if(lParam == 2+1)
            {
                for(int i = 0; i < cPosition::MAX_POSITION; i++)
                    position[i].SetMenu((HMENU)wParam, i);
            }
            else if(lParam == 5+1)
            {
                EnableMenuItem((HMENU)wParam, 398, macro::recording?MF_GRAYED:MF_ENABLED);
                EnableMenuItem((HMENU)wParam, 399, macro::recording?MF_ENABLED:MF_GRAYED);
            }
            break;
        case WM_KEYUP: case WM_KEYDOWN: case WM_CHAR: case WM_COMMAND: case WM_MOUSEWHEEL:
            SendMessage(h_client_wnd, message, wParam, lParam);
            return 0;
        case WM_ACTIVATE:
            SendMessage(h_tool_bar, message, wParam, lParam);
            return 0;
        case WM_CLOSE:
            if(thread_num)
                MessageBox(hwnd, "Some background threads are still running, terminate them, then exit", szAppName, MB_OK);
            else
            {
                cRecent::SaveFileQuery();
                DestroyWindow(h_main_wnd);
            }
            return 0;
        case WM_DESTROY:
            plik = CreateFile(module_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
            WriteFile(plik, &Font::actual, sizeof(int), &(DWORD)br, NULL);
            WriteFile(plik, &hexColors::scheme, sizeof(char), &(DWORD)br, NULL);

            WriteFile(plik, &column_num, sizeof(int), &(DWORD)br, NULL);
            WriteFile(plik, &auto_column, sizeof(bool), &(DWORD)br, NULL);
            WriteFile(plik, &column_group, sizeof(int), &(DWORD)br, NULL);
            WriteFile(plik, &using_unicode, sizeof(bool), &(DWORD)br, NULL);

            WriteFile(plik, &always_ontop, sizeof(bool), &(DWORD)br, NULL);
            WriteFile(plik, &hide_toolbar, sizeof(bool), &(DWORD)br, NULL);
            WriteFile(plik, &hexFind_forward, sizeof(bool), &(DWORD)br, NULL);
            WriteFile(plik, &hexFind_from_cursor, sizeof(bool), &(DWORD)br, NULL);
            WriteFile(plik, &align_structures, sizeof(bool), &(DWORD)br, NULL);
            WriteFile(plik, &highlight_size, sizeof(int), &(DWORD)br, NULL);
            WriteFile(plik, highlight, highlight_size, &(DWORD)br, NULL);
            WriteFile(plik, &highlightbeta_size, sizeof(int), &(DWORD)br, NULL);
            WriteFile(plik, highlightbeta, highlightbeta_size, &(DWORD)br, NULL);
            WriteFile(plik, &highlightgamma_size, sizeof(int), &(DWORD)br, NULL);
            WriteFile(plik, highlightgamma, highlightgamma_size, &(DWORD)br, NULL);
            WriteFile(plik, &goto_dec, sizeof(bool), &(DWORD)br, NULL);
            WriteFile(plik, &goto_mode, sizeof(int), &(DWORD)br, NULL);

            wndpl.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(h_main_wnd, &wndpl);
            WriteFile(plik, &wndpl, wndpl.length, &(DWORD)br, NULL);
            cRecent::WriteAll(plik);
            CloseHandle(plik);
            structure::WriteAll();
            macro::Clear();
            MultiClipboard::WriteAll();
            if(dll_logo)
                FreeLibrary(dll_logo);
            return 0;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

LRESULT CALLBACK ProceduraTool(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    MSG msg;
    static int cxClient;
    static int selected;
    static const int ICON_NUM = 11;
    static HICON hIcon[13][2];
    static bool pressed = 0;
    switch(message)
    {
        case WM_CREATE:
            HINSTANCE hInstance;
            hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
            for(int i = 0; i < ICON_NUM; i++)
            {
                hIcon[i][0] = LoadIcon(hInstance, MAKEINTRESOURCE (501 + i * 2));
                hIcon[i][1] = LoadIcon(hInstance, MAKEINTRESOURCE (502 + i * 2));
            }
            selected = -2;
            return 0;
        case WM_SIZE:
            cxClient = LOWORD(lParam);
            return 0;
        case WM_ACTIVATE:
            if(LOWORD(wParam) == WA_INACTIVE)
            {
                selected = -2;
                pressed = 0;
                InvalidateRect(hwnd, NULL, 0);
            }
            return 0;
        case WM_MOUSEMOVE:
            int cxMouse, cyMouse;
            SetCapture(hwnd);
            cxMouse = LOWORD(lParam);
            cyMouse = HIWORD(lParam);
            if(cyMouse > 0 && cyMouse < 40 && cxMouse > 0 && cxMouse < 48 * ICON_NUM)
            {
                if((cxMouse - 4) / 48 != selected)
                {
                    rect.top = 0;
                    rect.bottom = 43;
                    rect.left = selected * 48;
                    rect.right = selected * 48 + 51;
                    selected = (cxMouse - 4) / 48;
                    InvalidateRect(hwnd, &rect, 0);
                    rect.left = selected * 48;
                    rect.right = selected * 48 + 51;
                    pressed = 0;
                    InvalidateRect(hwnd, &rect, 0);
                }
            }
            else
            {
                ReleaseCapture();
                rect.top = 0;
                rect.bottom = 43;
                rect.left = selected * 48;
                rect.right = selected * 48 + 51;
                selected = -2;
                pressed = 0;
                InvalidateRect(hwnd, &rect, 0);
            }
            return 0;
        case WM_LBUTTONDOWN:
            rect.top = 0;
            rect.bottom = 43;
            rect.left = selected * 48;
            rect.right = selected * 48 + 51;
            pressed = 1;
            InvalidateRect(hwnd, &rect, 0);
            return 0;
        case WM_LBUTTONUP:
            rect.top = 0;
            rect.bottom = 43;
            rect.left = selected * 48;
            rect.right = selected * 48 + 51;
            if(pressed)
            {
                switch(selected)
                {
                    case 0: SendMessage(h_main_wnd, WM_COMMAND, 103, 0); break;
                    case 1: SendMessage(h_main_wnd, WM_COMMAND, 123, 0); break;
                    case 2: SendMessage(h_main_wnd, WM_COMMAND, 124, 0); break;
                    case 3: SendMessage(h_main_wnd, WM_COMMAND, 125, 0); break;
                    case 4: SendMessage(h_main_wnd, WM_COMMAND, 127, 0); break;
                    case 5: SendMessage(h_main_wnd, WM_COMMAND, 128, 0); break;
                    case 6: SendMessage(h_main_wnd, WM_COMMAND, 129, 0); break;
                    case 7: SendMessage(h_main_wnd, WM_COMMAND, 131, 0); break;
                    case 8: SendMessage(h_main_wnd, WM_COMMAND, 137, 0); break;
                    //case 8: case 9: case 10: SendMessage(h_main_wnd, WM_COMMAND, 183 + selected, 0); break;
                    case 9: SendMessage(h_main_wnd, WM_COMMAND, 252, 0); break;
                    case 10: SendMessage(h_main_wnd, WM_COMMAND, 251, 0); break;
                }
            }
            pressed = 0;
            InvalidateRect(hwnd, &rect, 0);
            return 0;
        case WM_RBUTTONUP:
            HMENU hPopupMenu;
            MENUITEMINFO mio;
            POINT point;
            point.x = LOWORD(lParam);
            point.y = HIWORD(lParam);
            ClientToScreen(hwnd, &point);
            switch(selected)
            {
                case 0:
                    hPopupMenu = GetMenu(h_main_wnd);
                    hPopupMenu = GetSubMenu(hPopupMenu, 0);
                    hPopupMenu = GetSubMenu(hPopupMenu, 7);
                    for(int i = 0; i < cRecent::recent_num; i++)
                        cRecent::pliki[i].SetMenu(hPopupMenu, i);
                    TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, h_client_wnd, NULL);
                    selected = -2;
                    pressed = 0;
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 2:
                    hPopupMenu = GetMenu(h_main_wnd);
                    hPopupMenu = GetSubMenu(hPopupMenu, 1+1);
                    hPopupMenu = GetSubMenu(hPopupMenu, 4);
                    TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, h_client_wnd, NULL);
                    selected = -2;
                    pressed = 0;
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 3:
                    hPopupMenu = GetMenu(h_main_wnd);
                    hPopupMenu = GetSubMenu(hPopupMenu, 1+1);
                    hPopupMenu = GetSubMenu(hPopupMenu, 6);

                    for(int i = 0; i < MultiClipboard::total; i++)
                        MultiClipboard::vc[i]->SetMenu(hPopupMenu);

                    TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, h_client_wnd, NULL);
                    selected = -2;
                    pressed = 0;
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 4:
                    hPopupMenu = GetMenu(h_main_wnd);
                    hPopupMenu = GetSubMenu(hPopupMenu, 1+1);
                    hPopupMenu = GetSubMenu(hPopupMenu, 6);

                    for(int i = 0; i < MultiClipboard::total; i++)
                        MultiClipboard::vc[i]->SetMenu(hPopupMenu);

                    TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, h_tool_bar, NULL);
                    selected = -2;
                    pressed = 0;
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 7:
                    hPopupMenu = GetMenu(h_main_wnd);
                    hPopupMenu = GetSubMenu(hPopupMenu, 2+1);
                    hPopupMenu = GetSubMenu(hPopupMenu, 2);
                    for(int i = 0; i < cPosition::MAX_POSITION; i++)
                        position[i].SetMenu(hPopupMenu, i);
                    TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, h_client_wnd, NULL);
                    selected = -2;
                    pressed = 0;
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 8:
                    hPopupMenu = GetMenu(h_main_wnd);
                    hPopupMenu = GetSubMenu(hPopupMenu, 1+1);
                    hPopupMenu = GetSubMenu(hPopupMenu, 12);
                    SetMenuDefaultItem(hPopupMenu, 137, FALSE);
                    TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, h_client_wnd, NULL);
                    SetMenuDefaultItem(hPopupMenu, (unsigned)-1, FALSE);
                    selected = -2;
                    pressed = 0;
                    InvalidateRect(hwnd, NULL, 0);
                    break;
            }
            return 0;
        case WM_COMMAND:
            case 700: case 701: case 702: case 703: case 704: case 705: case 706: case 707: case 708: case 709:
            case 710: case 711: case 712: case 713: case 714:
                if(!MultiClipboard::vc[LOWORD(wParam) - 700]->Find(hwnd))
                    MessageBox(hwnd, "Search bytes not found.", szAppName, MB_OK);
            return 0;
        case WM_PAINT:
            int press_offset;
            press_offset = pressed;
            hdc = BeginPaint(hwnd, &ps);
            SelectObject(hdc, CreateSolidBrush(sys_COLOR_3DFACE));
            PatBlt(hdc, 0, 0, cxClient, 45, PATCOPY);

            int lbStyle;
            lbStyle = BS_NULL;
            DeleteObject(SelectObject(hdc, CreateBrushIndirect((LOGBRUSH*)&lbStyle)));

            DeleteObject(SelectObject(hdc, CreatePen(PS_SOLID, 1, sys_COLOR_3DHILIGHT)));
            Rectangle(hdc, 1, 1, cxClient, 42);

            DeleteObject(SelectObject(hdc, CreatePen(PS_SOLID, 1, sys_COLOR_3DSHADOW)));
            Rectangle(hdc, 0, 0, cxClient - 1, 41);

            #if BETA
            TextOut(hdc, cxClient - 50, 0, "v.BETA", 6);
            #endif

            for(int i = 0; i < ICON_NUM; i++)
                DrawIcon(hdc, i * 48 + 10 + press_offset, 5 + press_offset, hIcon[i][i==selected?0:1]);

            if(selected != -2)
            {                
                DeleteObject(SelectObject(hdc, CreatePen(PS_SOLID, 1, pressed?sys_COLOR_3DSHADOW:sys_COLOR_3DHILIGHT)));
                MoveToEx(hdc, selected * 48 + 46 + 4, 4, NULL);
                LineTo(hdc, selected * 48 + 4, 4);
                LineTo(hdc, selected * 48 + 4, 38);

                DeleteObject(SelectObject(hdc, CreatePen(PS_SOLID, 1, pressed?sys_COLOR_3DHILIGHT:sys_COLOR_3DSHADOW)));
                MoveToEx(hdc, selected * 48 + 46 + 4, 4, NULL);
                LineTo(hdc, selected * 48 + 46 + 4, 38);
                LineTo(hdc, selected * 48 + 4, 38);


            }
            DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
            DeleteObject(SelectObject(hdc, GetStockObject(BLACK_BRUSH)));
            EndPaint(hwnd, &ps);
            return 0;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

LRESULT CALLBACK ProceduraOkna(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    static HINSTANCE hInstance;
    static OPENFILENAME ofn;
    unsigned int br;
    static HDC ekran;
    static HBITMAP bmp;
    static HMENU hPopupMenu;
    HMENU hmenu;
    PAINTSTRUCT ps;
    TEXTMETRIC tm;
    SCROLLINFO si;
    RECT rect;
    static int iAccumDelta = 0;
    static bool drag_up = 0;
    MSG macro_atom;
    char*val;
    switch(message)
    {
        case WM_CREATE:
            hInstance = ((LPCREATESTRUCT) lParam)->hInstance;

            delete [] pamiec;
            pamiec = new unsigned char[STD_OVERALLOC];
            bytes_allocated = STD_OVERALLOC;

            ekran = CreateDC("DISPLAY", 0, 0, 0);
            hdc = CreateCompatibleDC(ekran);
            bmp = CreateCompatibleBitmap(ekran, 2000, 2000);
            SelectObject(hdc, bmp);
            SelectObject(hdc, CreatePen(PS_SOLID, 1, 0x888888));
            SetBkMode(hdc, TRANSPARENT);
            font[Font::actual].Set(hdc);

            hPopupMenu = LoadMenu(hInstance, "CONTEXT");
            hPopupMenu = GetSubMenu(hPopupMenu, 0);

            DragAcceptFiles(hwnd, 1);
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hwnd;
            ofn.hInstance = NULL;
            ofn.lpstrFilter = NULL;
            ofn.lpstrCustomFilter = NULL;
            ofn.nMaxCustFilter = 0;
            ofn.nFilterIndex = 0;
            ofn.lpstrFile = NULL;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = MAX_PATH;
            ofn.lpstrInitialDir = NULL;
            ofn.lpstrTitle = NULL;
            ofn.Flags = 0;
            ofn.nFileOffset = 0;
            ofn.nFileExtension = 0;
            ofn.lpstrDefExt = NULL;
            ofn.lCustData = 0L;
            ofn.lpfnHook = NULL;
            ofn.lpTemplateName = NULL;

            if(nazwa_pliku[0])
                SendMessage(hwnd, WM_COMMAND, 108, 0);


            char nazwa_dysku[STD_BUF];
            MENUITEMINFO mio;
            mio.cbSize = sizeof(MENUITEMINFO);
            mio.fMask = MIIM_TYPE | MIIM_ID;
            mio.fType = MFT_STRING;
            mio.dwTypeData = nazwa_dysku;

            for(char drive='a'; drive<='n'; drive++)
            {
              dysk=new cDiskAccess(drive);
              if(dysk->GetLastError()!=HEX_MEDIA_ERR_ACCESSING_MEDIA)
              {
                nazwa_dysku[0]=drive;
                nazwa_dysku[1]=0;
                strcat(nazwa_dysku, ": ");
                /*
                int p=strlen(nazwa_dysku);
                _i64toa(dysk->GetSize(), nazwa_dysku+strlen(nazwa_dysku), 10);
                Dots(nazwa_dysku+p, 10);
                strcat(nazwa_dysku, "]");*/

                mio.wID = 2000 + drive;
                mio.cch = strlen(nazwa_dysku);
                InsertMenuItem(GetMenu(h_main_wnd), 2666, FALSE, &mio);
              }
              delete dysk;
            }

            SetStatus();
            #if BETA
            {
                HMENU hMenu = GetMenu(hwnd);
                MENUITEMINFO mio;
                mio.cbSize = sizeof(MENUITEMINFO);
                mio.fMask = MIIM_TYPE | MIIM_ID;

                mio.wID = 997;
                mio.fType = MFT_STRING;
                mio.dwTypeData = "Run Tests";
                mio.cch = strlen("Run Tests");
                InsertMenuItem(GetMenu(h_main_wnd), 150, FALSE, &mio);

                mio.fType = MFT_SEPARATOR;
                InsertMenuItem(GetMenu(h_main_wnd), 150, FALSE, &mio);
            }
            #endif
            return 0;
        case WM_SIZE:
            cxClient = LOWORD(lParam);
            cyClient = HIWORD(lParam);
            if(auto_column)
                SetColumnNum((cxClient - PANEL_OFFSET - 200) / (cxChar*3));
            ScrollHexplorer();
            return 0;
        case WM_VSCROLL:
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            GetScrollInfo(h_client_wnd, SB_VERT, &si);
            pg_offset = si.nPos;
            switch(LOWORD(wParam))
            {
                case SB_TOP: si.nPos = si.nMin; break;
                case SB_BOTTOM: si.nPos = si.nMax; break;
                case SB_LINEUP: si.nPos--; break;
                case SB_LINEDOWN: si.nPos++; break;
                case SB_PAGEUP: si.nPos -= si.nPage; break;
                case SB_PAGEDOWN: si.nPos += si.nPage; break;
                case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
            }
            si.fMask = SIF_POS;
            SetScrollInfo(h_client_wnd, SB_VERT, &si, TRUE);
            GetScrollInfo(h_client_wnd, SB_VERT, &si);
            if(si.nPos != pg_offset)
            {
                ScrollWindow(h_client_wnd, 0, cyChar * (pg_offset - si.nPos), NULL, NULL);
                UpdateWindow(h_client_wnd);
            }
            return 0;
        case WM_LBUTTONDOWN:
            LMBpressed = 1;
            static int cxMouse, cyMouse;
            if(!(wParam & MK_SHIFT))
            {
                cxMouse = LOWORD(lParam);
                cyMouse = HIWORD(lParam);
                if(cxMouse < PANEL_POSITION_SIZE)
                {

                }
                else if(cxMouse < (cxChar * (column_num << 1)) + PANEL_POSITION_SIZE + column_separators)
                {
                    int interprete_row = ((cyMouse / cyChar + pg_offset) * column_num);
                    int interprete_column = (cxMouse - PANEL_POSITION_SIZE);
                    int step = (cxChar<<1)*column_group + cxChar;
                    for(int t = step; t < (cxMouse - PANEL_POSITION_SIZE); t+=step)
                        interprete_column -= cxChar;
                    interprete_column/=(cxChar<<1);
                    selected_begin = interprete_row + interprete_column;
                    split = 0;
                }
                else if(cxMouse > (cxChar * (column_num << 1)) + PANEL_POSITION_SIZE + PANEL_OFFSET + column_separators && cxMouse < cxChar * column_num * 3 + PANEL_POSITION_SIZE + PANEL_OFFSET + column_separators)
                {
                    selected_begin = ((cyMouse / cyChar + pg_offset) * column_num) + (cxMouse - PANEL_POSITION_SIZE - PANEL_OFFSET - column_separators - (cxChar*(column_num<<1))) / cxChar;
                    split = 1;
                }
                if(selected_begin > dlugosc_pliku)
                    selected_begin = dlugosc_pliku;
                selected_end = hexFind_from = selected_begin;
                InvalidateRect(hwnd, NULL, 0);
                SetStatus();
            }
            upper4 = 1;
            return 0;
        case WM_LBUTTONUP:
            LMBpressed = 0;
            wParam = MK_LBUTTON;
        case WM_MOUSEMOVE:
            if(wParam == MK_LBUTTON)
            {
                if(!LMBpressed && !shift) // this avoids respondig to message sent after double clicking
                    return 0;


                int old_begin = selected_begin;
                int old_end = selected_end;
                cxMouse = LOWORD(lParam);
                cyMouse = HIWORD(lParam);
                int* p;
                p = drag_up?&selected_begin:&selected_end;
                if(cxMouse >= PANEL_POSITION_SIZE && cxMouse < (cxChar * (column_num<<1)) + PANEL_POSITION_SIZE + column_separators)
                {
                    int interprete_row = ((cyMouse / cyChar + pg_offset) * column_num);
                    int interprete_column = (cxMouse - PANEL_POSITION_SIZE);
                    int step = (cxChar<<1)*column_group + cxChar;
                    for(int t = step; t < (cxMouse - PANEL_POSITION_SIZE); t+=step)
                        interprete_column -= cxChar;
                    interprete_column/=(cxChar<<1);
                    *p = interprete_row + interprete_column;
                    split = 0;
                }
                else if(cxMouse > (cxChar * (column_num<<1)) + PANEL_POSITION_SIZE + PANEL_OFFSET  + column_separators && cxMouse < cxChar*column_num*3 + PANEL_POSITION_SIZE + PANEL_OFFSET + column_separators)
                {
                    *p = ((cyMouse / cyChar + pg_offset)*column_num) + (cxMouse - PANEL_POSITION_SIZE - PANEL_OFFSET - column_separators - (cxChar*(column_num<<1))) / cxChar;
                    split = 1;
                }

                if(selected_end < selected_begin)
                {
                    drag_up = !drag_up;
                    int temp;
                    temp = selected_begin;
                    selected_begin = selected_end;
                    selected_end = temp;
                }

                if(selected_begin > dlugosc_pliku)
                    selected_begin = dlugosc_pliku;
                if(selected_end > dlugosc_pliku)
                    selected_end = dlugosc_pliku;
                hexFind_from = selected_begin;

                if(selected_begin != old_begin || selected_end != old_end)
                {
                    InvalidateRect(hwnd, NULL, 0);
                    SetStatus();
                }
            }
            return 0;
        case WM_RBUTTONUP:
            POINT point;
            cxMouse=point.x = LOWORD(lParam);
            cyMouse=point.y = HIWORD(lParam);
            ClientToScreen(hwnd, &point);
            int pos;

            if(cxMouse >= PANEL_POSITION_SIZE && cxMouse < (cxChar * (column_num<<1)) + PANEL_POSITION_SIZE + column_separators)
            {
                int interprete_row = ((cyMouse / cyChar + pg_offset) * column_num);
                int interprete_column = (cxMouse - PANEL_POSITION_SIZE);
                int step = (cxChar<<1)*column_group + cxChar;
                for(int t = step; t < (cxMouse - PANEL_POSITION_SIZE); t+=step)
                    interprete_column -= cxChar;
                interprete_column/=(cxChar<<1);
                pos = interprete_row + interprete_column;
                //split = 0;
            }
            else if(cxMouse > (cxChar * (column_num<<1)) + PANEL_POSITION_SIZE + PANEL_OFFSET  + column_separators && cxMouse < cxChar*column_num*3 + PANEL_POSITION_SIZE + PANEL_OFFSET + column_separators)
            {
                pos = ((cyMouse / cyChar + pg_offset)*column_num) + (cxMouse - PANEL_POSITION_SIZE - PANEL_OFFSET - column_separators - (cxChar*(column_num<<1))) / cxChar;
                //split = 1;
            }

            //cPosition::PrepareForDraw(position, pos, pos + line_num * column_num);
            for(int i = 0; i < cPosition::MAX_POSITION; i++)
               if(position[i].DrawContextMenu(hInstance,hwnd,point.x,point.y,pos))
                 return 0;


            if(point.x > PANEL_POSITION_SIZE)
            {
                EnableMenuItem(hPopupMenu, 125, schoweklen?MF_ENABLED:MF_GRAYED);
                if(IsClipboardFormatAvailable(CF_TEXT))
                {
                  for(int i=174; i<=180; i++)
                    EnableMenuItem(hPopupMenu, i, MF_ENABLED);
                }
                else
                {
                  for(int i=174; i<=180; i++)
                    EnableMenuItem(hPopupMenu, i, MF_GRAYED);
                }
                TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
            }
            else
            {
                HMENU hPopupGoTo;
                hPopupGoTo = GetMenu(h_main_wnd);
                hPopupGoTo = GetSubMenu(hPopupGoTo, 2);
                hPopupGoTo = GetSubMenu(hPopupGoTo, 2);
                InsertMenu(hPopupGoTo, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
                InsertMenu(hPopupGoTo, 0, MF_BYPOSITION | MF_STRING, 131, "Go To Address...");
                TrackPopupMenu(hPopupGoTo, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
                DeleteMenu(hPopupGoTo, 0, MF_BYPOSITION);
                DeleteMenu(hPopupGoTo, 0, MF_BYPOSITION);
            }
            return 0;
        case WM_LBUTTONDBLCLK:

            if(LOWORD(lParam) <= PANEL_POSITION_SIZE)
            {
                //hex_addresses=!hex_addresses;
            }
            else
            {
                while(
                        selected_begin > 0 &&
                        pamiec[selected_begin - 1] != 0 &&
                        pamiec[selected_begin - 1] != ' '
                    )
                    selected_begin--;
                while(
                        selected_end < dlugosc_pliku - 1 &&
                        pamiec[selected_end + 1] != 0 &&
                        pamiec[selected_end + 1] != ' '
                    )
                    selected_end++;
            }
            InvalidateRect(hwnd, NULL, 0);
            SetStatus();
            return 0;
        case WM_MOUSEWHEEL:
            if((short) HIWORD(wParam) > 0)
            {
                SendMessage(h_client_wnd, WM_VSCROLL, SB_LINEUP, 0);
                SendMessage(h_client_wnd, WM_VSCROLL, SB_LINEUP, 0);
                SendMessage(h_client_wnd, WM_VSCROLL, SB_LINEUP, 0);
            }
            else
            {
                SendMessage(h_client_wnd, WM_VSCROLL, SB_LINEDOWN, 0);
                SendMessage(h_client_wnd, WM_VSCROLL, SB_LINEDOWN, 0);
                SendMessage(h_client_wnd, WM_VSCROLL, SB_LINEDOWN, 0);
            }
            return 0;
        case WM_DROPFILES:
            HDROP hDrop;
            hDrop = (HDROP) wParam;
            DragQueryFile(hDrop, 0, nazwa_pliku, MAX_PATH);
            DragFinish(hDrop);
            SendMessage(hwnd, WM_COMMAND, 108, 0);
            return 0;
        case WM_KEYUP:
            if(macro::recording)
            {
                macro_atom.message = message;
                macro_atom.wParam = wParam;
                macro_atom.lParam = lParam;
                macro::recording->AddAction(&macro_atom);
            }
            if(wParam==16)
                shift = 0;
            else if(wParam==17)
                control = 0;
            return 0;
        case WM_KEYDOWN:
            if(macro::recording)
            {
                macro_atom.message = message;
                macro_atom.wParam = wParam;
                macro_atom.lParam = lParam;
                macro::recording->AddAction(&macro_atom);
            }
            switch(wParam)
            {
                case 16: shift = 1; break;
                case 17: control = 1; break;
                // Obs³uga klawiaturowa zak³adek
                case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 58:
                    if(control)
                        if(shift)
                        {
                            position[LOWORD(wParam) - 48].Set(selected_begin, "\0");
                            InvalidateRect(h_client_wnd, NULL, 0);
                            SetStatus();
                        }
                        else
                            position[wParam - 48].GoTo();
                    break;
                case 37:    // strzalka lewa
                    if(shift)
                    {   if(selected_end - selected_begin) selected_end--;  }
                    else
                    {
                        if(selected_begin)
                        {
                            selected_end--;
                            hexFind_from = selected_begin = selected_end;
                        }
                    }
                    upper4 = 1;
                    ScrollTo(selected_end);
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 39:    // strzalka prawa
                    if(shift)
                    {   if(selected_end<dlugosc_pliku) selected_end++; }
                    else
                    {
                        if(selected_end<dlugosc_pliku)
                        {
                            selected_end++;
                            hexFind_from = selected_begin = selected_end;
                        }
                    }
                    upper4 = 1;
                    ScrollTo(selected_end);
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 38:    // strzalka gora
                    if(shift)
                    {   if(selected_end - selected_begin - column_num > 0) selected_end -= column_num;      }
                    else
                    {
                        if(selected_begin - (column_num-1) > 0)
                        {
                            selected_end -= column_num;
                            hexFind_from = selected_begin = selected_end;
                        }
                    }
                    upper4 = 1;
                    ScrollTo(selected_end);
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 40:    // strzalka dol
                    if(shift)
                    {   if(selected_end + column_num <= dlugosc_pliku) selected_end += column_num; }
                    else
                    {
                        if(selected_end + column_num <= dlugosc_pliku)
                        {
                            selected_end += column_num;
                            hexFind_from = selected_begin = selected_end;
                        }
                    }
                    upper4 = 1;
                    ScrollTo(selected_end);
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 36:    // Home
                    if(shift)
                        selected_begin = 0;
                    else
                        hexFind_from = selected_begin = selected_end = 0;
                    ScrollTo(selected_end);
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 35:    // End
                    if(shift)
                        selected_end = dlugosc_pliku?dlugosc_pliku-1:0;
                    else
                        hexFind_from = selected_begin = selected_end = dlugosc_pliku?dlugosc_pliku-1:0;
                    ScrollTo(selected_end);
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 33:    // PgUp
                    selected_begin = selected_end -= (cyClient / cyChar + 1)*column_num;
                    if(selected_end < 0)
                        selected_begin = selected_end = 0;
                    hexFind_from = selected_end;
                    ScrollTo(selected_end);
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 34:    // PgDn
                    selected_begin = selected_end += (cyClient / cyChar + 1)*column_num;
                    if(selected_end >= dlugosc_pliku)
                        selected_begin = selected_end = dlugosc_pliku - 1;
                    hexFind_from = selected_end;
                    ScrollTo(selected_end);
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 45:
                    insert = !insert;
                    SetStatus();
                    break;
                case 46:    // Delete
                    if(selected_end < dlugosc_pliku)
                    {
                        if(!cUndo::redirected_by_undo)
                            cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Delete");
                        MoveMemory(pamiec + selected_begin, pamiec + selected_end + 1, dlugosc_pliku - (selected_end + 1));
                        dlugosc_pliku -= (selected_end - selected_begin) + 1;
                        selected_end = selected_begin;
                        saved = 0;
                        if(redraw)
                        {
                            SetStatus();
                            ScrollHexplorer();
                            InvalidateRect(hwnd, NULL, 0);
                        }
                    }
                    break;
                case 114: SendMessage(hwnd, WM_COMMAND, 129, 0); break;
                case 113: SendMessage(hwnd, WM_COMMAND, 130, 0); break;
                case 118:  // Increment Byte
                    for(int i = selected_begin; i <= selected_end; i++)
                        pamiec[i] += 1;
                    if(!cUndo::redirected_by_undo)
                        cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Increment Byte");
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    saved = 0;
                    break;
                case 119:  //Decrement Byte
                    for(int i = selected_begin; i <= selected_end; i++)
                        pamiec[i] -= 1;
                    if(!cUndo::redirected_by_undo)
                        cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Decrement Byte");
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    saved = 0;
                    break;
                case 116:   // Go To
                    SendMessage(h_client_wnd, WM_COMMAND, 131, 0);
                    break;
                case 122:   // Previous sector
                    if(files) break;
                    SendMessage(h_client_wnd, WM_COMMAND, 2303, 0);
                    break;
                case 123:   // Next sector
                    if(files) break;
                    SendMessage(h_client_wnd, WM_COMMAND, 2302, 0);
                    break;
                case 112:
                    SendMessage(h_client_wnd, WM_COMMAND, 150, 0);
            }
            return 0;
        case WM_CHAR:
            if(macro::recording)
            {
                macro_atom.message = message;
                macro_atom.wParam = wParam;
                macro_atom.lParam = lParam;
                macro::recording->AddAction(&macro_atom);
            }
            if(wParam == 0x1A)
            {
                if(cUndo::step)
                    SendMessage(h_client_wnd, WM_COMMAND, 121, 0);
                return 0;
            }
            else if(wParam == 1)    // Ctrl+A
            {
                SendMessage(h_client_wnd, WM_COMMAND, 136, 0);
                return 0;
            }
            else if(wParam == 3)    // Ctrl+C
            {
                SendMessage(h_client_wnd, WM_COMMAND, 124, 0);
                return 0;
            }
            else if(wParam == 0x16) // Ctrl+V
            {
                SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                return 0;
            }
            else if(wParam == 0x6) // Ctrl+F
            {
                SendMessage(h_client_wnd, WM_COMMAND, 127, 0);
                return 0;
            }
            else if(wParam == 0xf)
            {
                SendMessage(h_client_wnd, WM_COMMAND, 103, 0);
                return 0;
            }
            else if(wParam == 9)
            {
                split = !split;
                InvalidateRect(h_client_wnd, NULL, 0);
                return 0;
            }
            unsigned char wstaw;
            wstaw = 0;
            saved = 0;
            selected_begin = selected_end;
            if(insert && upper4) //if(insert and hex byte is fully written)
            {
                MoveMemory(pamiec + selected_end + 1, pamiec + selected_end, dlugosc_pliku - selected_end);
                dlugosc_pliku++;
                if(dlugosc_pliku + 100 > bytes_allocated)
                {
                    unsigned char* temp;
                    temp = pamiec;
                    pamiec = new unsigned char [dlugosc_pliku + STD_OVERALLOC];
                    bytes_allocated = dlugosc_pliku + STD_OVERALLOC;
                    CopyMemory(pamiec, temp, dlugosc_pliku);
                    delete [] temp;
                }
            }
            else if(selected_end == dlugosc_pliku) // don't overwrite last byte
                return 0;
            if(split)
            {
                cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Type");
                pamiec[selected_end] = (unsigned char)wParam;
                if(selected_end < dlugosc_pliku)
                    selected_begin = ++selected_end;
            }
            else
            {
                wstaw = DecryptHex(wParam);
                if(upper4)
                {
                    cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Type");
                    pamiec[selected_end] &= 0x0F;
                    pamiec[selected_end] |= (wstaw << 4);
                    upper4 = 0;
                    InvalidateRect(hwnd, NULL, 0);
                }
                else
                {
                    pamiec[selected_end] &= 0xF0;
                    pamiec[selected_end] |= wstaw;
                    upper4 = 1;
                    if(selected_end < dlugosc_pliku)
                        selected_begin = ++selected_end;
                }
            }
            SetStatus();
            ScrollHexplorer();
            ScrollTo(selected_end);
            InvalidateRect(hwnd, NULL, 0);
            return 0;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 101:   // Exit
                    SendMessage(h_main_wnd, WM_CLOSE, 0, 0);
                    //DestroyWindow(h_main_wnd);
                    break;
                case 102:   // New
                    cRecent::SaveFileQuery();
                    if(strlen(nazwa_pliku))
                        cRecent::Add(nazwa_pliku);  // niejawnie: cRecent::pliki[actual].RememberEditingPosition();
                    delete [] pamiec;
                    pamiec = new unsigned char[STD_OVERALLOC];
                    bytes_allocated = STD_OVERALLOC;
                    dlugosc_pliku = 0;
                    selected_begin = selected_end = 0;
                    saved = 2;
                    files=1;
                    cRecent::actual = -1;
                    nazwa_pliku[0] = 0;
                    for(int i = 0; i < cPosition::MAX_POSITION; i++)
                        position[i].Reset();
                    SetStatus();
                    ScrollHexplorer();
                    SetWindowText(h_main_wnd, "ICY Hexplorer");
                    cUndo::Forget();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                //case 120:   // Open partially
                case 103:   // Open
                    cRecent::SaveFileQuery();
                    strcpy(nazwa_pliku_temp, nazwa_pliku);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = nazwa_pliku;
                    ofn.lpstrFileTitle = title;
                    ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
                    if(!GetOpenFileName(&ofn))
                        break;
                    /*
                    dlugosc_pliku=GetFileSize(plik, NULL);
                    if(LOWORD(wParam)!=103)
                      DialogBox(hInstance, "FILE_PART", hwnd, FilePart);*/
                case 108:   // Reopen
                    SetCursor(LoadCursor(NULL, IDC_WAIT));
                    plik = CreateFile(nazwa_pliku, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
                    if(plik == INVALID_HANDLE_VALUE)
                    {
                        //WyswietlBlad();
                        MessageBox(hwnd, "Cannot open", szAppName, 0);

                    }
                    else
                    {
                        try
                        {
                            unsigned char* temp = pamiec;
                            pamiec = new unsigned char[GetFileSize(plik, NULL) + STD_OVERALLOC];
                            delete [] temp;
                            dlugosc_pliku = GetFileSize(plik, NULL);
                            bytes_allocated = dlugosc_pliku + STD_OVERALLOC;

                            ReadFile(plik, pamiec, dlugosc_pliku, &(DWORD)br, NULL);

                            cUndo::Forget();
                            cRecent::Add(nazwa_pliku);
                            InvalidateRect(hwnd, NULL, 0);
                            saved = 2;
                            files=1;
                            SetStatus();
                            ScrollHexplorer();
                        }
                        catch(...)
                        {
                            MessageBox(hwnd, "Not enough memory to load file", szAppName, 0);
                            strcpy(nazwa_pliku, nazwa_pliku_temp);
                        }
                        CloseHandle(plik);
                    }
                    break;
                case 104:   // Save
                    SetCursor(LoadCursor(NULL, IDC_WAIT));
                    if(strlen(nazwa_pliku))
                    {
                        plik = CreateFile(nazwa_pliku, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
                        if(plik == INVALID_HANDLE_VALUE)
                            MessageBox(hwnd, "Could not write to file", szAppName, 0);
                        else
                        {
                            WriteFile(plik, pamiec, dlugosc_pliku, &(DWORD)br, NULL);
                            CloseHandle(plik);
                            saved = 1;
                            SetStatus();
                        }
                        break;
                    }
                case 105:   // Save As
                    SetCursor(LoadCursor(NULL, IDC_WAIT));
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = nazwa_pliku;
                    ofn.lpstrFileTitle = title;
                    ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
                    if(GetSaveFileName(&ofn))
                    {
                        plik = CreateFile(nazwa_pliku, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
                        WriteFile(plik, pamiec, dlugosc_pliku, &(DWORD)br, NULL);
                        CloseHandle(plik);
                        saved = 1;
                        files=1;
                        SetStatus();
                        cRecent::Add(nazwa_pliku);
                    }
                    break;
                case 109:   // Save and Execute
                    if(*nazwa_pliku!=0)
                    {
                        char temp_file[MAX_PATH];
                        int i;
                        GetModulePath(temp_file);
                        for(
                            i = strlen(nazwa_pliku);
                            nazwa_pliku[i - 1]!='\\' && nazwa_pliku[i - 1]!='/';
                            i--
                           );
                        strcat(temp_file, "~");
                        strcat(temp_file, nazwa_pliku + i);
                        plik = CreateFile(temp_file, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
                        if(plik == INVALID_HANDLE_VALUE)
                            MessageBox(hwnd, "Could not write to file", szAppName, 0);
                        else
                        {
                            WriteFile(plik, pamiec, dlugosc_pliku, &(DWORD)br, NULL);
                            CloseHandle(plik);
                        }
                        ShellExecute(hwnd, "open", temp_file, NULL, NULL, SW_SHOWNORMAL);
                        MessageBox(hwnd, "Temporary file has been created and executed.\nPress OK to delete it now.", szAppName, MB_OK);
                        DeleteFile(temp_file);
                    }
                    else
                        if(MessageBox(hwnd, "File must have it's name before executed.\nDo you want to save it?", szAppName, MB_YESNO | MB_ICONQUESTION) == IDYES)
                        {
                            SendMessage(hwnd, WM_COMMAND, 104, 0);
                            SendMessage(hwnd, WM_COMMAND, 109, 0);
                        }
                    break;
                case 106:
                    unsigned char c[6];
                    static DOCINFO di;
                    static PRINTDLG pd;
                    int cxPrinter, cyPrinter, cxCharP, cyCharP;
                    int from_page, to_page, clines;
                    di.cbSize = sizeof (DOCINFO);
                    di.lpszDocName = szAppName;
                    pd.lStructSize = sizeof(PRINTDLG);
                    pd.hwndOwner = h_main_wnd;
                    pd.hDevMode = pd.hDevNames = pd.hDC = NULL;
                    pd.Flags = PD_ALLPAGES | PD_COLLATE | PD_RETURNDC;
                    pd.nMinPage = pd.nFromPage = 0;
                    pd.nMaxPage = pd.nToPage = 0;
                    pd.nCopies = 1;
                    pd.hInstance = NULL;
                    pd.lCustData = 0;
                    pd.lpfnPrintHook = pd.lpfnSetupHook = NULL;
                    pd.lpPrintTemplateName = pd.lpSetupTemplateName = NULL;
                    pd.hPrintTemplate = pd.hSetupTemplate = NULL;
                    if(!PrintDlg(&pd))
                        break;
                    cxPrinter = GetDeviceCaps (pd.hDC, HORZRES);
                    cyPrinter = GetDeviceCaps (pd.hDC, VERTRES) - 40;
                    GetTextMetrics(pd.hDC, &tm);
                    cxCharP = tm.tmAveCharWidth * (tm.tmPitchAndFamily & 1 ? 3 : 2) / 2 + 2;
                    cyCharP = tm.tmHeight + tm.tmExternalLeading + 2;
                    clines = cyPrinter / cyCharP + 1;
                    if(pd.Flags & PD_SELECTION)
                    {
                        from_page = (selected_begin >> 4) / clines;
                        to_page = (selected_end >> 4) / clines + 1;
                    }
                    else
                    {
                        from_page = 0;
                        to_page = (dlugosc_pliku >> 4) / clines + 1;
                    }
                    if(pd.hDC)
                    {
                        if(StartDoc (pd.hDC, &di) > 0)
                        {
                            for(int copies = 0; copies < pd.nCopies; copies++)
                            {
                                for(int page = from_page; page < to_page; page++)
                                {
                                    StartPage (pd.hDC);
                                    for(int y = 0; y < clines; y++)
                                    {
                                        for(int x = 0; x < 16; x++)
                                            if((y + page * clines << 4) + x < dlugosc_pliku)
                                            {
                                                c[0] = ToHex4bits(pamiec[(y + page * clines << 4) + x] >> 4);
                                                c[1] = ToHex4bits(pamiec[(y + page * clines << 4) + x]);
                                                TextOut(pd.hDC, x * (cxCharP + cxCharP) + (9 * cxCharP), y * cyCharP, (char*)c, 2);
                                                TextOut(pd.hDC, 16 * (cxCharP + cxCharP) + (10 * cxCharP) + x * cxCharP, y * cyCharP, (char*)(pamiec + ((y  + page * clines << 4) + x)), 1);
                                                c[0] = ToHex4bits((y + (page * clines)) >> 16);
                                                c[1] = ToHex4bits((y + (page * clines)) >> 12);
                                                c[2] = ToHex4bits((y + (page * clines)) >> 8);
                                                c[3] = ToHex4bits((y + (page * clines)) >> 4);
                                                c[4] = ToHex4bits(y + (page * clines));
                                                c[5] = '0';
                                                c[6] = ':';
                                                TextOut(pd.hDC, 0, y * cyCharP, (char*)c, 7);
                                            }

                                    }
                                    EndPage(pd.hDC);
                                }
                            }
                            EndDoc(pd.hDC);
                        }
                        DeleteDC(pd.hDC);
                    }
                    break;
                case 113:   // Date, Time and Attributes
                    DialogBox(hInstance, "TIME", hwnd, TimeDlgProc);
                    break;
                case 2501:case 2502:case 2503:case 2504:case 2505:case 2506:case 2507:case 2508:case 2509:case 2510:
                case 2511:case 2512:case 2513:case 2514:case 2515:case 2516:case 2517:case 2518:case 2519:case 2520:case 2521:
                    char plik_import[MAX_PATH];
                    plik_import[0] = 0;
                    OPENFILENAME import_ofn;
                    import_ofn.lStructSize = sizeof(OPENFILENAME);
                    import_ofn.hwndOwner = h_main_wnd;
                    import_ofn.hInstance = NULL;
                    import_ofn.lpstrFilter = NULL;
                    import_ofn.lpstrCustomFilter = NULL;
                    import_ofn.nMaxCustFilter = 0;
                    import_ofn.nFilterIndex = 1;
                    import_ofn.lpstrFile = plik_import;
                    import_ofn.nMaxFile = MAX_PATH;
                    import_ofn.lpstrFileTitle = NULL;
                    import_ofn.nMaxFileTitle = MAX_PATH;
                    import_ofn.lpstrInitialDir = NULL;
                    import_ofn.lpstrTitle = NULL;
                    import_ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
                    import_ofn.lpstrDefExt = NULL;
                    import_ofn.lCustData = 0L;
                    import_ofn.lpfnHook = NULL;
                    import_ofn.lpTemplateName = NULL;
                    if(GetOpenFileName(&import_ofn))
                    {
                      unsigned char*schowek_tmp=new unsigned char[schoweklen];
                      CopyMemory(schowek_tmp, schowek, schoweklen);
                      delete[]schowek;
                      schowek=new unsigned char[STD_OVERALLOC];
                      int schowek_len_tmp=schoweklen;
                      schoweklen=0;
                      bool insert_tmp=insert;
                      insert=true;

                      SendMessage(h_client_wnd, WM_COMMAND, 102, 0);
                      srec_record dane;
                      srec_input*sih;
                      switch(LOWORD(wParam))
                      {
                        case 2501: sih=new srec_input_file_ascii_hex(plik_import); break;
                        case 2502: sih=new srec_input_file_atmel_generic(plik_import); break;
                        case 2503: sih=new srec_input_file_dec_binary(plik_import); break;
                        case 2504: sih=new srec_input_file_emon52(plik_import); break;
                        case 2505: sih=new srec_input_file_fastload(plik_import); break;
                        case 2506: sih=new srec_input_file_four_packed_code(plik_import); break;
                        case 2507: sih=new srec_input_file_intel(plik_import); break;
                        case 2508: sih=new srec_input_file_mos_tech(plik_import); break;
                        case 2509: sih=new srec_input_file_srecord(plik_import); break;
                        case 2510: sih=new srec_input_file_os65v(plik_import); break;
                        case 2511: sih=new srec_input_file_signetics(plik_import); break;
                        case 2512: sih=new srec_input_file_spasm(plik_import); break;
                        case 2513: sih=new srec_input_file_tektronix(plik_import); break;
                        case 2514: sih=new srec_input_file_tektronix_extended(plik_import); break;
                        case 2515: sih=new srec_input_file_ti_tagged(plik_import); break;
                        case 2516: sih=new srec_input_file_wilson(plik_import); break;
                        case 2517: sih=new srec_input_file_cosmac(plik_import); break;
                        case 2518: sih=new srec_input_file_fairchild(plik_import); break;
                        case 2519: sih=new srec_input_file_formatted_binary(plik_import); break;
                        case 2520: sih=new srec_input_file_needham(plik_import); break;
                        case 2521: sih=new srec_input_file_spectrum(plik_import); break;
                      }
                      sih->set_quit(win_quitter);
                      while(sih->read(dane))
                      {
                        schoweklen=dane.get_length();
                        CopyMemory(schowek, dane.get_data(), schoweklen);
                        SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                        selected_begin=selected_end=dlugosc_pliku;
                      }
                      delete sih;
                      insert=insert_tmp;
                      delete[]schowek;
                      schowek=new unsigned char[schowek_len_tmp];
                      CopyMemory(schowek, schowek_tmp, schowek_len_tmp);
                      schoweklen=schowek_len_tmp;
                      delete[]schowek_tmp;
                      SetStatus();
                    }
                  break;
                case 2601:case 2602:case 2603:case 2604:case 2605:case 2606:case 2607:case 2608:case 2609:case 2610:
                case 2611:case 2612:case 2613:case 2614:case 2615:case 2616:case 2617:case 2618:case 2619:case 2620:
                case 2621:case 2622:
                    char plik_export[MAX_PATH];
                    plik_export[0] = 0;
                    OPENFILENAME export_ofn;
                    export_ofn.lStructSize = sizeof(OPENFILENAME);
                    export_ofn.hwndOwner = h_main_wnd;
                    export_ofn.hInstance = NULL;
                    export_ofn.lpstrFilter = NULL;
                    export_ofn.lpstrCustomFilter = NULL;
                    export_ofn.nMaxCustFilter = 0;
                    export_ofn.nFilterIndex = 1;
                    export_ofn.lpstrFile = plik_export;
                    export_ofn.nMaxFile = MAX_PATH;
                    export_ofn.lpstrFileTitle = NULL;
                    export_ofn.nMaxFileTitle = MAX_PATH;
                    export_ofn.lpstrInitialDir = NULL;
                    export_ofn.lpstrTitle = NULL;
                    export_ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
                    export_ofn.lpstrDefExt = NULL;
                    export_ofn.lCustData = 0L;
                    export_ofn.lpfnHook = NULL;
                    export_ofn.lpTemplateName = NULL;
                    if(GetSaveFileName(&export_ofn))
                    {
                      srec_record dane;
                      srec_output*sih;
                      switch(LOWORD(wParam))
                      {
                        case 2601: sih=new srec_output_file_ascii_hex(plik_export); break;
                        case 2602: sih=new srec_output_file_atmel_generic(plik_export, 1); break;
                        case 2622: sih=new srec_output_file_atmel_generic(plik_export, 0); break;
                        case 2603: sih=new srec_output_file_dec_binary(plik_export); break;
                        case 2604: sih=new srec_output_file_emon52(plik_export); break;
                        case 2605: sih=new srec_output_file_fastload(plik_export); break;
                        case 2606: sih=new srec_output_file_four_packed_code(plik_export); break;
                        case 2607: sih=new srec_output_file_intel(plik_export); break;
                        case 2608: sih=new srec_output_file_mos_tech(plik_export); break;
                        case 2609: sih=new srec_output_file_srecord(plik_export); break;
                        case 2610: sih=new srec_output_file_os65v(plik_export); break;
                        case 2611: sih=new srec_output_file_signetics(plik_export); break;
                        case 2612: sih=new srec_output_file_spasm(plik_export); break;
                        case 2613: sih=new srec_output_file_tektronix(plik_export); break;
                        case 2614: sih=new srec_output_file_tektronix_extended(plik_export); break;
                        case 2615: sih=new srec_output_file_ti_tagged(plik_export); break;
                        case 2616: sih=new srec_output_file_wilson(plik_export); break;
                        case 2617: sih=new srec_output_file_cosmac(plik_export); break;
                        case 2618: sih=new srec_output_file_fairchild(plik_export); break;
                        case 2619: sih=new srec_output_file_formatted_binary(plik_export); break;
                        case 2620: sih=new srec_output_file_needham(plik_export); break;
                        case 2621: sih=new srec_output_file_spectrum(plik_export); break;
                      }
                      //sih->set_quit(win_quitter);
                      //sih->write_start_address();
                      sih->write_data(0, pamiec, dlugosc_pliku);
                      delete sih;
                      SetStatus();
                    }
                  break;
                /*
                case 107:   // Insert File
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = nazwa_pliku_temp;
                    ofn.lpstrFileTitle = title;
                    ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
                    if(GetOpenFileName(&ofn))
                    {
                        plik = CreateFile(nazwa_pliku_temp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
                        dlugosc_pliku += GetFileSize(plik, NULL);
                        selected_end = selected_begin + GetFileSize(plik, NULL) - 1;
                        unsigned char* temp;
                        temp = pamiec;
                        pamiec = new unsigned char [dlugosc_pliku + STD_OVERALLOC];
                        bytes_allocated = dlugosc_pliku + STD_OVERALLOC;
                        CopyMemory(pamiec, temp, selected_begin + 1);
                        ReadFile(plik, pamiec + selected_begin, GetFileSize(plik, NULL), &(DWORD)br, NULL);
                        CopyMemory(pamiec + selected_begin + GetFileSize(plik, NULL), temp + selected_begin, dlugosc_pliku - selected_begin - GetFileSize(plik, NULL));
                        delete [] temp;
                        CloseHandle(plik);
                        ScrollHexplorer();
                        SetStatus();
                        InvalidateRect(hwnd, NULL, 0);
                        if(!cUndo::redirected_by_undo)
                        {
                            bool temp_ins = insert; insert = 1;
                            cUndo::undo[cUndo::step] = new cUndo(WM_COMMAND, 125, "Insert File");
                            insert = temp_ins;
                        }
                    }
                    break;*/
                case 200: case 201: case 202: case 203: case 204: case 205: case 206: case 207:
                case 208: case 209: case 210: case 211: case 212: case 213: case 214:
                    cRecent::SaveFileQuery();
                    cRecent::pliki[LOWORD(wParam) - 200].Open();
                    break;

                case 2000+'a': case 2000+'b': case 2000+'c': case 2000+'d': case 2000+'e': case 2000+'f': case 2000+'g':
                case 2000+'h': case 2000+'i': case 2000+'j': case 2000+'k': case 2000+'l': case 2000+'m': case 2000+'n':
                  cRecent::SaveFileQuery();
                  if(!lParam)
                    if(!
                      DialogBox(hInstance, "DISK", hwnd, DiskDlgProc)
                    )
                      break;

                  SetStatus("Loading drive, be patient...");
                  SetCursor(LoadCursor(NULL, IDC_WAIT));
                  drive_name=LOWORD(wParam)-2000;

                  dysk=new cDiskAccess(drive_name);
                  BytesPerSector=((cDiskAccess*)dysk)->GetBytesPerSector();
                  try
                  {
                    unsigned char* temp = pamiec;
                    pamiec = new unsigned char[BytesPerSector*SectorsLoaded + STD_OVERALLOC];
                    delete [] temp;
                    bytes_allocated = BytesPerSector*SectorsLoaded + STD_OVERALLOC;
                  }
                  catch(...)
                  {
                    MessageBox(hwnd, "Not enough memory to load sectors", szAppName, 0);
                    delete dysk;
                    break;
                  }
                  TotalDiskSize=dysk->GetSize();
                  if(!(dysk->Read(pamiec, BytesPerSector*SectorsLoaded, BytesPerSector*CurrentSector)))
                    MessageBox(hwnd, "Some problems occured while reading sectors", szAppName, 0);
                  delete dysk;
                  files=0;
                  insert=0;
                  dlugosc_pliku=BytesPerSector*SectorsLoaded;
                  nazwa_pliku[0]=0;

                  cUndo::Forget();
                  selected_begin=selected_end=0;
                  InvalidateRect(hwnd, NULL, 0);
                  saved = 2;
                  SetStatus();
                  ScrollHexplorer();

                  char caption[STD_BUF];
                  char dskinf[STD_BUF];
                  strcpy(caption, "ICY Hexplorer - [");
                  dskinf[0]=drive_name;dskinf[1]=0;
                  strcat(caption, dskinf);
                  strcat(caption, ": sector ");
                  _ui64toa(CurrentSector, dskinf, 10);
                  Dots(dskinf, 10);
                  strcat(caption, dskinf);
                  if(SectorsLoaded>1)
                  {
                    strcat(caption, " - ");
                    _itoa(CurrentSector+SectorsLoaded-1, dskinf, 10);
                    Dots(dskinf, 10);
                    strcat(caption, dskinf);
                  }
                  strcat(caption, " of ");
                  _ui64toa(TotalDiskSize/BytesPerSector, dskinf, 10);
                  Dots(dskinf, 10);
                  strcat(caption, dskinf);
                  strcat(caption, "]");
                  SetWindowText(h_main_wnd, caption);

                  break;
                case 2302:
                  if(!(TotalDiskSize/BytesPerSector));  else  // nic nie wczytano -> ponizsze warunki bez sensu
                  if(CurrentSector+SectorsLoaded>=TotalDiskSize/BytesPerSector)
                    CurrentSector=TotalDiskSize/BytesPerSector-SectorsLoaded;
                  else
                    CurrentSector+=SectorsLoaded;
                  SendMessage(h_client_wnd, WM_COMMAND, 2000+drive_name, 1);
                  break;
                case 2303:
                  if(CurrentSector-SectorsLoaded<0)
                    CurrentSector=0;
                  else
                    CurrentSector-=SectorsLoaded;
                  SendMessage(h_client_wnd, WM_COMMAND, 2000+drive_name, 1);
                  break;
                case 2304:
                  if(MessageBox(h_main_wnd, "Do you really want to save sectors?", szAppName, MB_YESNO | MB_ICONQUESTION)==IDYES)
                  {
                    dysk=new cDiskAccess(drive_name);
                    if(!(dysk->Write(pamiec, BytesPerSector*SectorsLoaded, BytesPerSector*CurrentSector)))
                      MessageBox(hwnd, "Cannot write to disk", szAppName, 0);
                    delete dysk;
                  }
                  break;
                case 110:  // Export to C
                    pucc = new ucContainer((unsigned char*)"unsigned char data[] = {\r\n", strlen("unsigned char data[] = {\r\n"));
                    for(int i = selected_begin; i <= selected_end; i++)
                    {
                        *pucc+='0';
                        *pucc+='x';
                        *pucc+=ToHex4bits(pamiec[i] >> 4);
                        *pucc+=ToHex4bits(pamiec[i]);
                        if(i!=selected_end)
                            *pucc+=',';
                        if(i % 16 == 15)
                            pucc->Push((unsigned char*)"\r\n", 2);
                    }
                    pucc->Push((unsigned char*)"\r\n};", 4);
                    pucc->CopyToClipboard(hwnd);
                    delete pucc;
                    MultiClipboard::Copy(pamiec + selected_begin, selected_end - selected_begin + 1);
                    break;
                case 111:   // Export to txt
                    pucc = new ucContainer;
                    for(int i = selected_begin; i <= selected_end; i++)
                    {
                        *pucc+=ToHex4bits(pamiec[i] >> 4);
                        *pucc+=ToHex4bits(pamiec[i]);
                        /*
                        if(i!=selected_end)
                            *pucc+=',';*/
                            if(i % 64 == 63)
                                pucc->Push((unsigned char*)"\r\n", 2);
                    }
                    pucc->CopyToClipboard(hwnd);
                    delete pucc;
                    MultiClipboard::Copy(pamiec + selected_begin, selected_end - selected_begin + 1);
                    break;
                case 112:   // Export to asm
                    pucc = new ucContainer((unsigned char*)"data db ", strlen("data db "));
                    //pucc->Push((unsigned char*)"data db ", strlen("data db "));
                    for(int i = selected_begin; i <= selected_end; i++)
                    {
                        *pucc+=ToHex4bits(pamiec[i] >> 4);
                        *pucc+=ToHex4bits(pamiec[i]);
                        *pucc+='h';
                        if(i % 16 == 15)
                            pucc->Push((unsigned char*)"\r\ndb ", 5);
                        else if(i!=selected_end)
                            *pucc+=',';
                    }
                    pucc->CopyToClipboard(hwnd);
                    delete pucc;
                    MultiClipboard::Copy(pamiec + selected_begin, selected_end - selected_begin + 1);
                    break;
                case 114:   // Filter text
                    pucc = new ucContainer;
                    for(int i = selected_begin; i <= selected_end; i++)
                        if(
                            (pamiec[i] > 31 && pamiec[i] < 127) &&
                            (
                                (i?(pamiec[i-1] > 31 && pamiec[i-1]<127):0) ||
                                (pamiec[i+1] > 31 && pamiec[i+1]<127)
                            )
                          )
                            *pucc+=pamiec[i];
                    pucc->CopyToClipboard(hwnd);
                    delete pucc;
                    MultiClipboard::Copy(pamiec + selected_begin, selected_end - selected_begin + 1);
                    break;
                case 116:
                  SetStatus("Disassembling...");
                  SetCursor(LoadCursor(NULL, IDC_WAIT));
                  pucc = new ucContainer;
                  int inst_len;
                  inst_len=0;
                  char tekst[STD_BUF];
                  for(int i = selected_begin; i <= selected_end+inst_len; i+=inst_len)
                  {
                    if(!(inst_len=sprint_address(tekst,STD_BUF,(char*)pamiec+i)))
                      break;
                    int len=strlen(tekst),
                      k=0;

                    while(tekst[k]!=9) k++;
                    //if(k!=strlen(tekst))
                      tekst[k]=' ';
                    strcat(tekst,"\r\n");
                    pucc->Push((unsigned char*)tekst,strlen(tekst));
                  }
                  pucc->CopyToClipboard(hwnd);
                  delete pucc;
                  MultiClipboard::Copy(pamiec + selected_begin, selected_end - selected_begin + 1);
                  SetStatus();
                  break;
                case 115:
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = nazwa_pliku_temp;
                    ofn.lpstrFileTitle = title;
                    ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
                    if(GetSaveFileName(&ofn))
                    {
                        plik = CreateFile(nazwa_pliku_temp, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
                        WriteFile(plik, pamiec + selected_begin, selected_end - selected_begin + 1, &(DWORD)br, NULL);
                        CloseHandle(plik);
                        MultiClipboard::Copy(pamiec + selected_begin, selected_end - selected_begin + 1);
                    }
                    break;
                case 121:
                    cUndo::undo[cUndo::step - 1]->Undo();
                    ScrollTo(selected_begin);
                    break;
                case 124:    // Copy
                    if(selected_end != dlugosc_pliku)
                    {
                        hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (selected_end - selected_begin) + 2);
                        pGlobal = (unsigned char*)GlobalLock(hGlobal);
                        CopyMemory(pGlobal, pamiec + selected_begin, (selected_end - selected_begin) + 1);
                        GlobalUnlock(hGlobal);
                        OpenClipboard(hwnd);
                        EmptyClipboard();
                        SetClipboardData(CF_TEXT, hGlobal);
                        CloseClipboard();

                        delete [] schowek;
                        schowek = new unsigned char[selected_end - selected_begin + 1];
                        CopyMemory(schowek, pamiec + selected_begin, selected_end - selected_begin + 1);
                        schoweklen = selected_end - selected_begin + 1;

                        MultiClipboard::Copy(pamiec + selected_begin, selected_end - selected_begin + 1);
                        SetStatus();
                    }
                    break;
                case 125:   // Paste
                    if(schoweklen)
                    {
                        selected_end = selected_begin + schoweklen - 1;
                        if(!cUndo::redirected_by_undo)
                            cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Paste");
                        if(insert)
                        {
                            dlugosc_pliku += schoweklen;
                            if(dlugosc_pliku + 100 > bytes_allocated)
                            {
                                unsigned char* temp;
                                temp = pamiec;
                                pamiec = new unsigned char [dlugosc_pliku + STD_OVERALLOC];
                                bytes_allocated = dlugosc_pliku + STD_OVERALLOC;
                                CopyMemory(pamiec, temp, selected_begin + 1);
                                CopyMemory(pamiec + selected_begin, schowek, schoweklen);
                                CopyMemory(pamiec + selected_begin + schoweklen, temp + selected_begin, dlugosc_pliku - selected_begin - schoweklen);
                                delete [] temp;
                            }
                            else
                            {
                                MoveMemory(pamiec + selected_begin + schoweklen, pamiec + selected_begin, dlugosc_pliku - selected_begin - schoweklen);
                                CopyMemory(pamiec + selected_begin, schowek, schoweklen);
                            }
                        }
                        else
                        {
                            int schoweklen_s;
                            if(selected_begin + schoweklen > dlugosc_pliku)
                            {
                                schoweklen_s = dlugosc_pliku - selected_begin;
                                selected_end = dlugosc_pliku - 1;
                                if(selected_end < 0)
                                    selected_end = 0;
                            }
                            else
                                schoweklen_s = schoweklen;
                            CopyMemory(pamiec + selected_begin, schowek, schoweklen_s);
                        }
                        saved = 0;
                        if(redraw)
                        {
                            ScrollHexplorer();
                            SetStatus();
                            InvalidateRect(hwnd, NULL, 0);
                        }
                    }
                    break;
                case 174:   // Paste external
                    OpenClipboard(hwnd);
                    hGlobal = GetClipboardData(CF_TEXT);
                    delete [] schowek;
                    schowek = new unsigned char[GlobalSize(hGlobal)];
                    pGlobal = (unsigned char*)GlobalLock(hGlobal);
                    strcpy((char*)schowek, (char*)pGlobal);
                    schoweklen = strlen((char*)schowek);
                    GlobalUnlock(hGlobal);
                    CloseClipboard();
                    SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                    break;
                case 175:  // Interpret external text as hex numbers
                {
                    int p;
                    bool hi = 1;
                    OpenClipboard(hwnd);
                    hGlobal = GetClipboardData(CF_TEXT);
                    delete [] schowek;
                    schowek = new unsigned char[GlobalSize(hGlobal)];
                    pGlobal = (unsigned char*)GlobalLock(hGlobal);
                    p = 0;
                    for(int i = 0; i < strlen((char*)pGlobal); i++)
                        switch(pGlobal[i])
                        {
                            case '0':
                            if(pGlobal[i+1]=='x'||pGlobal[i+1]=='X')
                                break;
                            case '1': case '2': case '3': case '4': case '5':
                            case '6': case '7': case '8': case '9': case 'a': case 'b':
                            case 'c': case 'd': case 'e': case 'f': case 'A': case 'B':
                            case 'C': case 'D': case 'E': case 'F':
                            if(hi)
                                schowek[p] = (DecryptHex(pGlobal[i]) << 4);
                            else
                            {
                                schowek[p] |= (DecryptHex(pGlobal[i]));
                                p++;
                            }
                            hi = !hi;
                        }
                    schoweklen = p;
                    GlobalUnlock(hGlobal);
                    CloseClipboard();
                    SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                    break;
                }
                case 176: // Interpret as floats
                {
                  _atox(hwnd, float(), true);
                  SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                  break;
                }
                case 177: // Interpret as doubles
                {
                  _atox(hwnd, double(), true);
                  SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                  break;
                }
                case 178:
                  _atox(hwnd, char(), false);
                  SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                  break;
                case 179:
                  _atox(hwnd, short(), false);
                  SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                  break;
                case 180:
                  _atox(hwnd, int(), false);
                  SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                  break;
                case 123:   // Cut
                    delete [] schowek;
                    schowek = new unsigned char[selected_end - selected_begin + 1];
                    CopyMemory(schowek, pamiec + selected_begin, selected_end - selected_begin + 1);
                    schoweklen = selected_end - selected_begin + 1;
                    hGlobal = GlobalAlloc(GHND | GMEM_SHARE, selected_end - selected_begin + 2);
                    pGlobal = (unsigned char*)GlobalLock(hGlobal);
                    CopyMemory(pGlobal, pamiec + selected_begin, selected_end - selected_begin + 1);
                    GlobalUnlock(hGlobal);
                    OpenClipboard(hwnd);
                    EmptyClipboard();
                    SetClipboardData(CF_TEXT, hGlobal);
                    CloseClipboard();
                case 126: SendMessage(h_client_wnd, WM_KEYDOWN, 46, 0); break;  // Delete
                case 700: case 701: case 702: case 703: case 704: case 705: case 706: case 707: case 708: case 709:
                case 710: case 711: case 712: case 713: case 714:
                    MultiClipboard::vc[LOWORD(wParam) - 700]->Paste();
                    break;
                case 136:   // Select All
                    if(dlugosc_pliku)
                    {
                        selected_begin = 0;
                        selected_end = dlugosc_pliku - 1;
                        SetStatus();
                        InvalidateRect(hwnd, NULL, 0);
                    }
                    break;
                case 135:   // Paste Chain
                    DialogBox(hInstance, "CHAIN", hwnd, ChainDlgProc);
                    break;
                case 137:  // Empty Selection
                    {
                        bool temp_ins = insert;
                        insert = 0;
                        cUndo::undo[cUndo::step] = new cUndo(WM_COMMAND, 125, "Reset Selection");
                        insert = temp_ins;
                    }
                    for(int i = selected_begin; i <= selected_end; pamiec[i++] = 0);
                    saved = 0;
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 134:   // Fill Selection
                    fmode = fFILL;
                    DialogBox(hInstance, "FILL", hwnd, FillDlgProc);
                    break;
                unsigned char tab[10000], *p, *q, *t, *MAX;
                case 143:   // Encrypt/Decrypt
                {
                    char*pass;
                    unsigned int pass_crc32;
                    pass = (char*)DialogBox(hInstance, "PASS", hwnd, NewValueDlgProc);
                    if(!pass)
                        break;

                    bool temp_ins = insert;
                    insert = 0;
                    cUndo::undo[cUndo::step] = new cUndo(WM_COMMAND, 125, "Encryption");
                    insert = temp_ins;

                    pass_crc32 = ChecksumDlg::UniversalCRC32((unsigned char*)pass, strlen(pass));

                    srand(pass_crc32);
                    for(int i = 0; i < 10000; tab[i++] = rand());
                    p = tab + 311;
                    q = tab + 5813;
                    t = tab;
                    MAX = tab + 10000;
                    for(int i = selected_begin; i <= selected_end; i++)
                    {
                        *t = *p++ + *q++;
                        pamiec[i] ^= *t++;
                        if(t >= MAX) t = tab;
                        if(p >= MAX) p = tab;
                        if(q >= MAX) q = tab;
                    }
                    saved = 0;
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                }
                case 140:   // Pseudo Random
                {
                    bool temp_ins = insert;
                    insert = 0;
                    cUndo::undo[cUndo::step] = new cUndo(WM_COMMAND, 125, "Pseudo Random");
                    insert = temp_ins;


                    srand(GetTickCount());
                    for(int i = 0; i < 10000; tab[i++] = rand());
                    p = tab + 311;
                    q = tab + 5813;
                    t = tab;
                    MAX = tab + 10000;
                    for(int i = selected_begin; i <= selected_end; i++)
                    {
                        *t = *p++ + *q++;
                        pamiec[i] = *t++;
                        if(t >= MAX) t = tab;
                        if(p >= MAX) p = tab;
                        if(q >= MAX) q = tab;
                    }
                    saved = 0;
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                }
                case 171:   // XOR Selection
                    fmode = fXOR;
                    DialogBox(hInstance, "FILL", hwnd, FillDlgProc);
                    break;
                case 172:   // OR Selection
                    fmode = fOR;
                    DialogBox(hInstance, "FILL", hwnd, FillDlgProc);
                    break;
                case 173:   // AND Selection
                    fmode = fAND;
                    DialogBox(hInstance, "FILL", hwnd, FillDlgProc);
                    break;
                case 138:   // Swap 16-bit
                    if(!cUndo::redirected_by_undo)
                        cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Swap Bytes (16 bit)");
                    for(int i = selected_begin; i < selected_end; i+=2)
                    {
                        int tmp;
                        tmp = pamiec[i+1];
                        pamiec[i+1] = pamiec[i];
                        pamiec[i] = tmp;
                    }
                    saved = 0;
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 139:   // Swap 32-bit
                    if(!cUndo::redirected_by_undo)
                        cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Swap Bytes (32 bit)");
                    for(int i = selected_begin; i < selected_end-2; i+=4)
                    {
                        int tmp;
                        tmp = pamiec[i+3];
                        pamiec[i+3] = pamiec[i];
                        pamiec[i] = tmp;
                        tmp = pamiec[i+2];
                        pamiec[i+2] = pamiec[i+1];
                        pamiec[i+1] = tmp;
                    }
                    saved = 0;
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 144:   // Swap 64-bit
                    if(!cUndo::redirected_by_undo)
                        cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Swap Bytes (64 bit)");
                    for(int i = selected_begin; i < selected_end-6; i+=8)
                    {
                        int tmp;
                        tmp = pamiec[i+7];
                        pamiec[i+7] = pamiec[i];
                        pamiec[i] = tmp;
                        tmp = pamiec[i+6];
                        pamiec[i+6] = pamiec[i+1];
                        pamiec[i+1] = tmp;
                        tmp = pamiec[i+5];
                        pamiec[i+5] = pamiec[i+2];
                        pamiec[i+2] = tmp;
                        tmp = pamiec[i+4];
                        pamiec[i+4] = pamiec[i+3];
                        pamiec[i+3] = tmp;
                    }
                    saved = 0;
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 159:
                    if(!cUndo::redirected_by_undo)
                        cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Flip Bytes");
                    for(int i = selected_begin; i < selected_begin+(selected_end-selected_begin)/2+1; i++)
                    {
                        int tmp;
                        tmp = pamiec[i];
                        pamiec[i] = pamiec[selected_end-(i-selected_begin)];
                        pamiec[selected_end-(i-selected_begin)]=tmp;
                    }
                    saved = 0;
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                case 127:   // Find
                    if(!DialogBox(hInstance, "FIND", hwnd, FindDlgProc))
                        break;
                    if(!hexFind_forward)
                    {
                        SendMessage(hwnd, WM_COMMAND, 130, 0);
                        break;
                    }
                case 129:
                    hexReplace_mode = 0;
                    if(!files && MessageBox(hwnd, "Search entire disk?", szAppName, MB_YESNO)==IDYES)
                    {
                      int ics;
                      do
                      {
                        ics=CurrentSector;
                        if(Find(1)) break;
                        SendMessage(h_client_wnd, WM_COMMAND, 2302, 0);  // load next sector
                      }
                      while(ics!=CurrentSector);
                    }
                    else
                    {
                      if(!Find(1))
                        MessageBox(hwnd, "Search bytes not found.", szAppName, 0);
                    }
                    break;
                case 130:
                    hexReplace_mode = 0;
                    if(!files && MessageBox(hwnd, "Search entire disk?", szAppName, MB_YESNO)==IDYES)
                    {
                      int ics;
                      do
                      {
                        ics=CurrentSector;
                        if(Find(0)) break;
                        SendMessage(h_client_wnd, WM_COMMAND, 2303, 0);  // load previous sector
                      }
                      while(ics!=CurrentSector);
                    }
                    else
                    {
                      if(!Find(0))
                        MessageBox(hwnd, "Search bytes not found.", szAppName, 0);
                    }
                    break;
                case 128:
                    DialogBox (hInstance, "REPLACE", hwnd, ReplaceDlgProc);
                    break;
                case 132: SendMessage(h_client_wnd, WM_KEYDOWN, 118, 0); break;
                case 133: SendMessage(h_client_wnd, WM_KEYDOWN, 119, 0); break;
                case 170:   // Negate Selection
                    for(int i = selected_begin; i <= selected_end; i++)
                        pamiec[i] = ~pamiec[i];
                    if(!cUndo::redirected_by_undo)
                        cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Negate Selection");
                    SetStatus();
                    InvalidateRect(hwnd, NULL, 0);
                    saved = 0;
                    break;
                case 131:
                    DialogBox (hInstance, "GOTO", hwnd, GoToDlgProc);
                    break;
                case 141:
                    DialogBox (hInstance, "HIGHLIGHT", hwnd, HighlightDlgProc);
                    InvalidateRect(h_client_wnd, NULL, 0);
                    break;
                case 251:
                    auto_column = 0;
                    SetColumnNum(column_num + 1);
                    InvalidateRect(h_client_wnd, NULL, 0);
                    break;
                case 252:
                    auto_column = 0;
                    SetColumnNum(column_num - 1);
                    InvalidateRect(h_client_wnd, NULL, 0);
                    break;
                case 142:   // Displays file information
                    MessageBox(
                        hwnd,
                        cFileType::filetype[cFileType::Find(pamiec)]->GetName(),
                        szAppName,
                        MB_OK
                    );
                    break;
                case 146:
                    ChildDlg::svd[ChildDlg::num] = new SimpleDataDlg();
                    if(align_structures)
                        ChildDlg::AlignAll();
                    break;
                case 147:
                    ChildDlg::svd[ChildDlg::num] = new ChecksumDlg();
                    if(align_structures)
                        ChildDlg::AlignAll();
                    break;
                case 149:
                    ChildDlg::svd[ChildDlg::num] = new OccurenceDlg();
                    if(align_structures)
                        ChildDlg::AlignAll();
                    break;
                case 154:
                    ChildDlg::svd[ChildDlg::num] = new PixelDlg();
                    if(align_structures)
                        ChildDlg::AlignAll();
                    break;
                case 158:
                    ChildDlg::svd[ChildDlg::num] = new DisassemblerDlg();
                    if(align_structures)
                        ChildDlg::AlignAll();
                    break;
                case 157:
                    ChildDlg::svd[ChildDlg::num] = new NavigatorDlg(position, cPosition::MAX_POSITION);
                    if(align_structures)
                        ChildDlg::AlignAll();
                    break;
                case 156:   // Calculator
                    ShellExecute(hwnd, "open", "calc.exe", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case 145:
                    DialogBox(hInstance, "OPTIONS", hwnd, OptionsDlgProc);
                    break;
                case 190: case 191: case 192: case 193: case 194: case 195: case 196: case 197: case 198: case 199:
                    position[LOWORD(wParam) - 190].GoTo();
                    break;
                case 160: case 161: case 162: case 163: case 164: case 165: case 166: case 167: case 168: case 169:
                    position[LOWORD(wParam) - 160].Set(selected_begin, "\0");
                    InvalidateRect(h_client_wnd, NULL, 0);
                    SetStatus();
                    break;
                case 299:
                    DialogBox(hInstance, "ADDSTRUCT", hwnd, AddStructDlgProc);
                    break;
                case 300: case 301: case 302: case 303: case 304: case 305: case 306: case 307: case 308: case 309:
                case 310: case 311: case 312: case 313: case 314: case 315: case 316: case 317: case 318: case 319:
                    ChildDlg::svd[ChildDlg::num] = new SViewerDlg(structure::st[LOWORD(wParam) - 300]);
                    if(align_structures)
                        ChildDlg::AlignAll();
                    break;
                case 260:
                    ChildDlg::svd[ChildDlg::num] = new PatternsDlg();
                    if(align_structures)
                        ChildDlg::AlignAll();
                    break;
                case 261:
                  ChildDlg::svd[ChildDlg::num] = new FourierDlg();
                    if(align_structures)
                        ChildDlg::AlignAll();
                  break;
                case 398:   // Record macro
                    char nazwa_makra[MAX_PATH];
                    nazwa_makra[0] = 0;
                    OPENFILENAME macro_ofn;
                    char hex_path[MAX_PATH];
                    macro_ofn.lStructSize = sizeof(OPENFILENAME);
                    macro_ofn.hwndOwner = h_main_wnd;
                    macro_ofn.hInstance = NULL;
                    macro_ofn.lpstrFilter = "Hexplorer Macros (*.hem)\0*.hem\0\0";
                    macro_ofn.lpstrCustomFilter = NULL;
                    macro_ofn.nMaxCustFilter = 0;
                    macro_ofn.nFilterIndex = 1;
                    macro_ofn.lpstrFile = nazwa_makra;
                    macro_ofn.nMaxFile = MAX_PATH;
                    macro_ofn.lpstrFileTitle = NULL;
                    macro_ofn.nMaxFileTitle = MAX_PATH;
                    GetModuleFileName(NULL, hex_path, MAX_PATH - 1);
                    macro_ofn.lpstrInitialDir = hex_path;
                    macro_ofn.lpstrTitle = NULL;
                    macro_ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
                    macro_ofn.lpstrDefExt = "hem";
                    macro_ofn.lCustData = 0L;
                    macro_ofn.lpfnHook = NULL;
                    macro_ofn.lpTemplateName = NULL;
                    if(GetSaveFileName(&macro_ofn))
                        macro::Activate(nazwa_makra);
                    break;
                case 399:   // Stop
                    macro::recording = 0;
                    DrawMenuBar(h_main_wnd);
                    break;
                case 400: case 401: case 402: case 403: case 404: case 405: case 406: case 407: case 408: case 409:
                case 410: case 411: case 412: case 413: case 414: case 415: case 416: case 417: case 418: case 419:
                    {
                      char*val=(char*)DialogBox(hInstance, "REPEAT", h_client_wnd, NewValueDlgProc);
                      unsigned int rep;
                      if(val)
                      {
                        rep=atoi(val);
                        if(rep==0) rep=1;
                        for(int i=0;i<rep;i++)
                          macro::mac[LOWORD(wParam) - 400]->Run();
                      }
                    }
                    break;
                case 150:
                    char help_file[MAX_PATH];
                    //GetModuleFileName(NULL, help_file, MAX_PATH - 1);
                    //strcpy(help_file + strlen(help_file) - 13, "help.html");
    GetModulePath(help_file);
    strcat(help_file, "help.html");

                    plik = CreateFile(help_file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
                    if(plik == INVALID_HANDLE_VALUE)
                        MessageBox(hwnd, "Help file not found", szAppName, 0);
                    else
                    {
                        CloseHandle(plik);
                        ShellExecute(hwnd, "open", help_file, NULL, NULL, SW_SHOWNORMAL);
                    }
                    break;
                case 151:
                    ShellExecute(hwnd, "open", "http://www.icy.prv.pl", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case 153:
                    ShellExecute(hwnd, "open", "mailto:mdudek@wszib.edu.pll?subject=Hexplorer_bug", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case 155:
                    hiVer=InternetOpen(szAppName, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
                    if(!(hiFile=InternetOpenUrl(hiVer, "http://213.76.247.16/~icy/hex_ver.ids", NULL, 0, 0, 0)))
                    {
                        MessageBox(hwnd, "Sorry, the server doesn't respond, try later", szAppName, MB_OK);
                        InternetCloseHandle(hiVer);
                        break;
                    }
                    int r;
                    InternetReadFile(hiFile, temp, 2000, &(DWORD)r);
                    InternetCloseHandle(hiFile);
                    InternetCloseHandle(hiVer);
                    if(memcmp(szAppName, temp, strlen(szAppName)))
                    {
                        if(IDYES==MessageBox(hwnd, "It seems that there's newer version available, \nDo you wish to download it now?", szAppName, MB_YESNO))
                            ShellExecute(hwnd, "open", "http://artemis.wszib.edu.pl/~mdudek/moje/hex_setup.exe", NULL, NULL, SW_SHOWNORMAL);
                    }
                    else
                        MessageBox(hwnd, "You are currently using the newest version of the program", szAppName, MB_OK);
                    break;
                case 152:
                    dll_logo = LoadLibrary("about.dll");
                    if(dll_logo)
                        DialogBox (dll_logo, "ABOUT", hwnd, AboutDlgProc);
                    else
                        MessageBox(hwnd, "Program files corrupted", szAppName, MB_OK);

                #if BETA
                case 997:
                     RunTest();
                #endif
                  break;
            }
            return 0;
        case WM_USER + 1:   // Replace
        {
            if(!cUndo::redirected_by_undo)
                cUndo::undo[cUndo::step] = new cUndo(message, wParam, "Replace");
            int sel = selected_end - selected_begin + 1;
            if(schoweklen != sel)
            {
                dlugosc_pliku += schoweklen - sel;
                if(dlugosc_pliku + 100 > bytes_allocated)
                {
                    unsigned char* temp;
                    temp = pamiec;
                    pamiec = new unsigned char [dlugosc_pliku + STD_OVERALLOC];
                    bytes_allocated = dlugosc_pliku + STD_OVERALLOC;
                    CopyMemory(pamiec, temp, selected_begin);
                    CopyMemory(pamiec + selected_begin + schoweklen, temp + selected_end + 1, dlugosc_pliku - selected_begin - sel);
                    delete [] temp;
                }
                else
                    MoveMemory(pamiec + selected_begin + schoweklen, pamiec + selected_end + 1, dlugosc_pliku - selected_end);
            }
            CopyMemory(pamiec + selected_begin, schowek, schoweklen);
            selected_end = selected_begin + schoweklen - 1;
            saved = 0;
            if(redraw)
            {
                ScrollHexplorer();
                SetStatus();
                InvalidateRect(hwnd, NULL, 0);
            }
            return 0;
        }
        case WM_PAINT:
            unsigned char reserve_c[100];
            unsigned char*c;
            c = reserve_c + 50;

            int komorka, komorka_disasm;
            int to_highlight;
            to_highlight = 0;

            si.cbSize = sizeof(si);
            si.fMask = SIF_POS;
            GetScrollInfo(h_client_wnd, SB_VERT, &si);
            pg_offset = si.nPos;

            DeleteObject(SelectObject(hdc, CreateSolidBrush(hexColors::actual->kolor[256])));
            PatBlt(hdc, PANEL_POSITION_SIZE, 0, cxClient - PANEL_POSITION_SIZE, cyClient, PATCOPY);

            int panel_left_x, panel_pixel_x, panel_right_x, panel_y, line_num;
            panel_y = 0;

            komorka = komorka_disasm = pg_offset * column_num;
            line_num = cyClient / cyChar + 1;
            cPosition::PrepareForDraw(position, komorka, komorka + line_num * column_num);
            for(int y = 0; y < line_num; y++)
            {
                panel_left_x = PANEL_POSITION_SIZE;
                panel_right_x = cxChar * (column_num<<1) + PANEL_POSITION_SIZE + PANEL_OFFSET + column_separators;
                panel_pixel_x = cxChar * column_num*3 + PANEL_POSITION_SIZE + PANEL_OFFSET + column_separators + 10;

                for(int x = 0; x < column_num; x++)
                {
                    if(komorka < dlugosc_pliku)
                    {
                        if(komorka == selected_end)
                            SetTextColor(hdc, hexColors::actual->kolor[255]);   // contrast!
                        else
                            SetTextColor(hdc, hexColors::actual->kolor[pamiec[komorka]]);

                        c[0] = ToHex4bits(pamiec[komorka] >> 4);
                        c[1] = ToHex4bits(pamiec[komorka]);
                        TextOut(hdc, panel_left_x, panel_y, (char*)c, 2);
                        TextOut(hdc, panel_right_x, panel_y, (char*)(pamiec + komorka), 1);

                        if(highlight_size && !memcmp(pamiec + komorka, highlight, highlight_size))
                            to_highlight = highlight_size;
                        if(highlightbeta_size && !memcmp(pamiec + komorka, highlightbeta, highlightbeta_size))
                            to_highlight = highlightbeta_size;
                        if(highlightgamma_size && !memcmp(pamiec + komorka, highlightgamma, highlightgamma_size))
                            to_highlight = highlightgamma_size;

                        if(highlight_struct == komorka)
                            to_highlight = highlight_struct_size;


                        if(komorka >= selected_begin && komorka < selected_end)
                        {
                            if(to_highlight)
                                to_highlight--;

                            DeleteObject(SelectObject(hdc, CreateSolidBrush(0x401540)));
                            PatBlt(hdc, panel_left_x - 2, panel_y, cxChar + cxChar, cyChar - 1, PATINVERT);
                            PatBlt(hdc, panel_right_x, panel_y, cxChar, cyChar - 1, PATINVERT);

                            if(komorka==selected_begin)
                            {
                               MoveToEx(hdc, panel_left_x - 2, panel_y+cyChar, 0);
                               LineTo(hdc, panel_left_x - 2, panel_y);
                               LineTo(hdc, panel_left_x - 2+cxChar + cxChar, panel_y);

                               MoveToEx(hdc, panel_right_x+cxChar, panel_y, 0);
                               LineTo(hdc, panel_right_x, panel_y);
                               if(x)
                                 LineTo(hdc, panel_right_x, panel_y+cyChar);
                            }
                            else if(komorka<selected_begin+column_num)
                            {
                               MoveToEx(hdc, panel_left_x - 2, panel_y, 0);
                               LineTo(hdc, panel_left_x - 2+cxChar + cxChar, panel_y);

                               MoveToEx(hdc, panel_right_x, panel_y, 0);
                               LineTo(hdc, panel_right_x+cxChar, panel_y);

                            }
                            else if(komorka>selected_end-column_num)
                            {
                               MoveToEx(hdc, panel_left_x - 2, panel_y+cyChar, 0);
                               LineTo(hdc, panel_left_x - 2+cxChar + cxChar, panel_y+cyChar);

                               MoveToEx(hdc, panel_right_x, panel_y+cyChar, 0);
                               LineTo(hdc, panel_right_x+cxChar, panel_y+cyChar);
                            }
                        }
                        else if(to_highlight)
                        {
                            DeleteObject(SelectObject(hdc, CreateSolidBrush(0xffff)));
                            PatBlt(hdc, panel_left_x - 2, panel_y, cxChar + cxChar, cyChar - 1, PATINVERT);
                            PatBlt(hdc, panel_right_x, panel_y, cxChar, cyChar - 1, PATINVERT);
                            to_highlight--;
                        }

                        for(int i = 0; i < cPosition::draw_posnum; i++)
                            if(cPosition::draw_pos[i] == komorka)
                            {
                                PatBlt(hdc, panel_left_x - 2, panel_y, cxChar + cxChar, cyChar - 1, DSTINVERT);
                                PatBlt(hdc, panel_right_x, panel_y, cxChar, cyChar - 1, DSTINVERT);
                            }
                    }
                    if(komorka == selected_end)
                    {
                        if(split)
                        {
                            DeleteObject(SelectObject(hdc, CreateSolidBrush(0x606060)));   // 0x606060
                            PatBlt(hdc, panel_left_x - 2, panel_y, cxChar + cxChar, cyChar - 1, PATINVERT);

                            DeleteObject(SelectObject(hdc, CreateSolidBrush(0xff))); //0xff0050
                            PatBlt(hdc, panel_right_x, panel_y, cxChar, cyChar - 1, PATINVERT);
                        }
                        else
                        {
                            DeleteObject(SelectObject(hdc, CreateSolidBrush(0xff)));
                            PatBlt(hdc, panel_left_x - 2, panel_y, cxChar + cxChar, cyChar - 1, PATINVERT);

                            DeleteObject(SelectObject(hdc, CreateSolidBrush(0x606060)));
                            PatBlt(hdc, panel_right_x, panel_y, cxChar, cyChar - 1, PATINVERT);
                        }
                    }
                    komorka++;
                    panel_left_x += cxChar + cxChar;
                    if(!((x+1)%column_group))
                        panel_left_x += cxChar;
                    panel_right_x += cxChar;
                }
                panel_y += cyChar;
            }

            MoveToEx(hdc, cxChar*(column_num<<1) + PANEL_POSITION_SIZE + PANEL_OFFSET - 5 + column_separators, 0, NULL);
            LineTo(hdc, cxChar*(column_num<<1) + PANEL_POSITION_SIZE + PANEL_OFFSET - 5 + column_separators, cyClient);
            MoveToEx(hdc, panel_pixel_x - 5, 0, NULL);
            LineTo(hdc, panel_pixel_x - 5, cyClient);

            ekran = BeginPaint(hwnd, &ps);
            BitBlt(ekran, 0, 0, cxClient, cyClient, hdc, 0, 0, SRCCOPY);
            EndPaint(hwnd, &ps);
            return 0;
        case WM_DESTROY:
            cUndo::Forget();
            delete [] pamiec;
            DeleteObject(SelectObject(hdc, GetStockObject(BLACK_BRUSH)));
            DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
            DeleteDC(hdc);
            DeleteObject(bmp);
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpszArgument, int nFunsterStil)
{
    MSG msg;
    WNDCLASSEX wc, wc_main, wc_tool_bar;
    RECT rect;
    TOOLINFO ti;

    ::hInstance = hInstance;

    disassemble_init(0, INTEL_SYNTAX);

    if(lpszArgument[0] == '\"')   // czasami winda przysyla sciezke w cudzyslowiu - trzeba go obciac
    {
        strcpy(nazwa_pliku, lpszArgument + 1);
        nazwa_pliku[strlen(nazwa_pliku)-1] = 0;
    }
    else
        strcpy(nazwa_pliku, lpszArgument);

    wc_main.hInstance = hInstance;
    wc_main.lpszClassName = "MAIN";
    wc_main.lpfnWndProc = OknoGlowne;
    wc_main.style = CS_HREDRAW | CS_VREDRAW;
    wc_main.cbSize = sizeof(WNDCLASSEX);
    wc_main.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE (500));
    wc_main.hIconSm = NULL;
    wc_main.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc_main.lpszMenuName = "MAIN_MENU";
    wc_main.cbClsExtra = 0;
    wc_main.cbWndExtra = 0;
    wc_main.hbrBackground = NULL;
    if(!RegisterClassEx(&wc_main))
        return 0;

    wc_tool_bar.hInstance = hInstance;
    wc_tool_bar.lpszClassName = "TOOL";
    wc_tool_bar.lpfnWndProc = ProceduraTool;
    wc_tool_bar.style = 0;
    wc_tool_bar.cbSize = sizeof(WNDCLASSEX);
    wc_tool_bar.hIcon = NULL;
    wc_tool_bar.hIconSm = NULL;
    wc_tool_bar.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc_tool_bar.lpszMenuName = NULL;
    wc_tool_bar.cbClsExtra = 0;
    wc_tool_bar.cbWndExtra = 0;
    wc_tool_bar.hbrBackground = NULL;
    if(!RegisterClassEx(&wc_tool_bar))
        return 0;

    wc.hInstance = hInstance;
    wc.lpszClassName = szAppName;
    wc.lpfnWndProc = ProceduraOkna;
    wc.style = CS_DBLCLKS;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hIcon = NULL;
    wc.hIconSm = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = NULL;
    if(!RegisterClassEx(&wc))
        return 0;


    h_main_wnd = CreateWindowEx(
           0,
           "MAIN",
           "ICY Hexplorer",
           WS_OVERLAPPEDWINDOW,
           CW_USEDEFAULT,
           CW_USEDEFAULT,
           640,
           344,
           HWND_DESKTOP,
           NULL,
           hInstance,
           NULL
           );


    InitCommonControls();
    status_bar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, szAppName, h_main_wnd, 101);
    int t[7];
    t[0] = 70;
    t[1] = 150 + 90;
    t[2] = 320 + 90;
    t[3] = 500 + 60;
    t[4] = -1;
    SendMessage(status_bar, SB_SETPARTS, 5, (LPARAM)t);

    status_bar_size = 0;
    SendMessage(status_bar, SB_GETRECT, 0, (LPARAM)&rect);
    status_bar_size += rect.bottom - rect.top;
    SendMessage(status_bar, SB_GETBORDERS, 0, (LPARAM)t);
    status_bar_size += t[1];

    h_tool_bar = CreateWindowEx(
           0,
           "TOOL",
           NULL,
           WS_CHILD | WS_VISIBLE,
           0,
           0,
           300,
           100,
           h_main_wnd,
           NULL,
           hInstance,
           NULL
           );


    h_client_wnd = CreateWindowEx(
           0,
           szAppName,
           NULL,
           WS_VSCROLL | WS_CHILD | WS_VISIBLE | WS_EX_ACCEPTFILES,
           0,
           0,
           300,
           100,
           h_main_wnd,
           (HMENU)102,
           hInstance,
           NULL
           );


    h_tool_tip = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
                                TOOLTIPS_CLASS,
                                NULL,
                                WS_POPUP | WS_BORDER | TTS_ALWAYSTIP,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                h_tool_bar,
                                (HMENU) NULL,
                                hInstance,
                                NULL);

    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = h_tool_bar;
    ti.hinst = hInstance;
    char* tips[] = {"Open", "Cut", "Copy", "Paste", "Find", "Replace", "Find Next", "Go To Address", "Reset Selection", "One column less", "One column more"};
    for(int i = 0; i < 11; i++)
    {
        ti.uId = i;
        ti.lpszText = tips[i];
        ti.rect.top = 0;
        ti.rect.bottom = 50;
        ti.rect.left = i * 48 + 4;
        ti.rect.right = ti.rect.left + 48;
        SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
    }

    hPopupRadixMenu = LoadMenu(hInstance, "RADIX");
    hPopupRadixMenu = GetSubMenu(hPopupRadixMenu, 0);

    hPopupBPPMenu = LoadMenu(hInstance, "BPP");
    hPopupBPPMenu = GetSubMenu(hPopupBPPMenu, 0);

    hPopupCut = LoadMenu(hInstance, "CUT");
    hPopupCut = GetSubMenu(hPopupCut, 0);

    SendMessage(h_main_wnd, WM_USER, 0, 0);
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    disassemble_cleanup();
    return msg.wParam;
}

void SetStatus(char* info) //  = 0
{
    char tekst[STD_BUF];
    int len;
    int value_size = selected_end - selected_begin + 1;
    if(insert)
        SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM)"Insert");
    else
        SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM)"Overwrite");
    strcpy(tekst, "Pos: ");
    itoa(selected_end, tekst + 5, 16);
    strcat(tekst, "h (");
    len = strlen(tekst);
    itoa(selected_end, tekst + len, 10);
    Dots(tekst + len, 10);
    strcat(tekst, ")");
    SendMessage(status_bar, SB_SETTEXT, 1, (LPARAM)tekst);
    unsigned __int64 ex_value = 0;

    switch(value_size)
    {
        case 1: strcpy(tekst, "Byte: "); break;
        case 2: strcpy(tekst, "Word: "); break;
        case 3: strcpy(tekst, "3 bytes: "); break;
        case 4: strcpy(tekst, "Double Word: "); break;
        case 5: strcpy(tekst, "5 bytes: "); break;
        case 6: strcpy(tekst, "6 bytes: "); break;
        case 7: strcpy(tekst, "7 bytes: "); break;
        case 8: strcpy(tekst, "Quad: "); break;
        default:
            strcpy(tekst, "Selection: ");
            _ui64toa(value_size, tekst + strlen(tekst), 10);
            strcat(tekst, " bytes");
    }
    if(value_size < 9)
    {
        int l = strlen(tekst);
        for(int i = selected_begin; i <= selected_end; i++)
            ex_value |= (unsigned __int64)pamiec[i] << ((i - selected_begin) << 3);
        _ui64toa(ex_value, tekst + l, 10);
        //Dots(tekst + l, 10);
    }
    SendMessage(status_bar, SB_SETTEXT, 2, (LPARAM)tekst);
    strcpy(tekst, "Size: ");
    len = strlen(tekst);
    _ui64toa(dlugosc_pliku, tekst + len, 10);
    Dots(tekst + len, 10);
    if(dlugosc_pliku == 1)
        strcat(tekst, " byte");
    else
        strcat(tekst, " bytes");
    SendMessage(status_bar, SB_SETTEXT, 3, (LPARAM)tekst);
    if(!info)
    {
        if(saved == 1)
            SendMessage(status_bar, SB_SETTEXT, 4, (LPARAM)"Saved");
        else if(saved == 0)
            SendMessage(status_bar, SB_SETTEXT, 4, (LPARAM)"Not saved");
        else
            SendMessage(status_bar, SB_SETTEXT, 4, (LPARAM)"Original");

        SViewerDlg::DisplayMembers();
    }
    else
        SendMessage(status_bar, SB_SETTEXT, 4, (LPARAM)info);
}

void ScrollHexplorer(int pos) //  = pg_offset
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = dlugosc_pliku / column_num;
    si.nPage = cyClient / cyChar;
    si.nPos = pos;
    SetScrollInfo(h_client_wnd, SB_VERT, &si, TRUE);
    GetScrollInfo(h_client_wnd, SB_VERT, &si);
    if(si.nPos != pg_offset)
    {
        ScrollWindow(h_client_wnd, 0, cyChar * (pg_offset - si.nPos), NULL, NULL);
        UpdateWindow(h_client_wnd);
    }
}

void ScrollTo(int aktualna_komorka)
{
    if(aktualna_komorka / column_num - pg_offset == cyClient / cyChar)
        ScrollHexplorer(pg_offset + 1);
    else if(aktualna_komorka / column_num - pg_offset > cyClient / cyChar)
        ScrollHexplorer((aktualna_komorka / column_num) - (cyClient / cyChar - 1));
    else if(aktualna_komorka / column_num < pg_offset)
        ScrollHexplorer(aktualna_komorka / column_num);
}

void SetColumnNum(int num)
{
    column_num = num;
    if(!column_num)
        column_num = 1;
    else if(column_num > 128)
        column_num = 128;
    column_separators = column_num / column_group * cxChar;
    InvalidateRect(h_client_wnd, NULL, 0);
    ScrollHexplorer();
}

BOOL CALLBACK OptionsDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    char tekst[200];
    RECT rect;
    switch(message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            SetFocus(GetDlgItem(hDlg, 1000));
            SendMessage(GetDlgItem(hDlg, 1000), CB_ADDSTRING, 0, (LPARAM)"Fixed Roman");
            SendMessage(GetDlgItem(hDlg, 1000), CB_ADDSTRING, 0, (LPARAM)"System Font");
            SendMessage(GetDlgItem(hDlg, 1000), CB_ADDSTRING, 0, (LPARAM)"System Fixed Font");
            SendMessage(GetDlgItem(hDlg, 1000), CB_ADDSTRING, 0, (LPARAM)"Default GUI Font");
            SendMessage(GetDlgItem(hDlg, 1000), CB_ADDSTRING, 0, (LPARAM)"Oem Fixed Font");
            SendMessage(GetDlgItem(hDlg, 1000), CB_SETCURSEL, Font::actual, 0);

            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Matrix");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"White background, special signs marked");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Autumn (For text)");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Black on white");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Green on black");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Yellow on blue");
            SendMessage(GetDlgItem(hDlg, 1001), CB_SETCURSEL, hexColors::scheme, 0);

            itoa(column_num, tekst, 10);
            SetWindowText(GetDlgItem(hDlg, 1002), tekst);
            itoa(column_group, tekst, 10);
            SetWindowText(GetDlgItem(hDlg, 1003), tekst);
            if(auto_column)
            {
                SendMessage(GetDlgItem(hDlg, 1004), BM_SETCHECK, 1, 0);
                EnableWindow(GetDlgItem(hDlg, 1002), FALSE);
                EnableWindow(GetDlgItem(hDlg, 1009), FALSE);
            }

            if(always_ontop)
                SendMessage(GetDlgItem(hDlg, 1100), BM_SETCHECK, 1, 0);
            if(align_structures)
                SendMessage(GetDlgItem(hDlg, 1101), BM_SETCHECK, 1, 0);
            if(hide_toolbar)
                SendMessage(GetDlgItem(hDlg, 1102), BM_SETCHECK, 1, 0);
            if(using_unicode)
                SendMessage(GetDlgItem(hDlg, 1107), BM_SETCHECK, 1, 0);

            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    font[SendMessage(GetDlgItem(hDlg, 1000), CB_GETCURSEL, 0, 0)].Set(hdc);

                    hexColors::scheme = SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);
                    hColors[SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0)].Set();

                    GetWindowText(GetDlgItem(hDlg, 1003), tekst, 200);
                    column_group = atoi(tekst);
                    if(column_group < 2)
                        column_group = 2;
                    else if(column_group > 128)
                        column_group = 128;
                    GetWindowText(GetDlgItem(hDlg, 1002), tekst, 200);
                    SetColumnNum(atoi(tekst));
                    auto_column = SendMessage(GetDlgItem(hDlg, 1004), BM_GETCHECK, 0, 0)==BST_CHECKED?1:0;
                    if(auto_column)
                        SetColumnNum((cxClient - PANEL_OFFSET - 200) / (cxChar*3));

                    always_ontop = SendMessage(GetDlgItem(hDlg, 1100), BM_GETCHECK, 0, 0)==BST_CHECKED?1:0;
                    if(always_ontop)
                        SetWindowPos(h_main_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    else
                        SetWindowPos(h_main_wnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    align_structures = SendMessage(GetDlgItem(hDlg, 1101), BM_GETCHECK, 0, 0)==BST_CHECKED?1:0;
                    if(align_structures)
                        SViewerDlg::AlignAll();
                    hide_toolbar = SendMessage(GetDlgItem(hDlg, 1102), BM_GETCHECK, 0, 0)==BST_CHECKED?1:0;
                    using_unicode = SendMessage(GetDlgItem(hDlg, 1107), BM_GETCHECK, 0, 0)==BST_CHECKED?1:0;
                    GetClientRect(h_main_wnd, &rect);
                    SendMessage(h_main_wnd, WM_SIZE, SIZE_RESTORED, (rect.bottom << 16) | rect.right);

                    InvalidateRect(h_client_wnd, NULL, 0);
                    EndDialog(hDlg, 0);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
                case 1004:
                    if(SendMessage(GetDlgItem(hDlg, 1004), BM_GETCHECK, 0, 0)==BST_CHECKED)
                    {
                        EnableWindow(GetDlgItem(hDlg, 1002), FALSE);
                        EnableWindow(GetDlgItem(hDlg, 1009), FALSE);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, 1002), TRUE);
                        EnableWindow(GetDlgItem(hDlg, 1009), TRUE);
                    }
                    return FALSE;
                case 1005:
                    IShellLink *psl;
                    IPersistFile* pPf;
                    LPITEMIDLIST pidl;
                    char buf[256];
                    char running_directory[MAX_PATH];
                    unsigned short int wsz[256];
                    SHGetSpecialFolderLocation(NULL, CSIDL_SENDTO, &pidl);
                    SHGetPathFromIDList(pidl, buf);
                    strcat(buf, "\\Hexplorer.lnk");
                    GetModuleFileName(NULL, running_directory, MAX_PATH - 1);
                    plik = CreateFile(buf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
                    if(plik == INVALID_HANDLE_VALUE)
                    {
                        CoInitialize(NULL);
                        CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
                        psl->SetPath(running_directory);
                        psl->SetDescription(szAppName);
                        psl->QueryInterface(IID_IPersistFile, (void**)&pPf);
                        MultiByteToWideChar(CP_ACP, 0, buf, -1, (WCHAR*)wsz, MAX_PATH);
                        GetModuleFileName (NULL, buf, MAX_PATH - 1);
                        psl->SetIconLocation(buf, 0);
                        pPf->Save((WCHAR*)wsz, TRUE);
                        pPf->Release();
                        psl->Release();
                        CoUninitialize();
                    }
                    else
                        CloseHandle(plik);

                    return FALSE;
            }
    }
    return FALSE;
}

BOOL CALLBACK TimeDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    static FILETIME f_time[3];
    static SYSTEMTIME s_time[3];
    static unsigned int attr;
    char tekst[STD_BUF];
    int kind;
    HANDLE plik;
    switch(message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Creation Time");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Last Access Time");
            SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)"Last Write Time");

            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"January");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"February");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"March");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"April");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"May");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"June");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"July");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"August");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"September");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"October");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"November");
            SendMessage(GetDlgItem(hDlg, 1010), CB_ADDSTRING, 0, (LPARAM)"December");


            //MessageBox(hDlg, nazwa_pliku, "hex", MB_OK);
            plik = CreateFile(nazwa_pliku, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

            GetFileTime(plik, &f_time[0], &f_time[1], &f_time[2]);

            CloseHandle(plik);
            for(int i=0; i < 3; i++)
            {
                FileTimeToSystemTime(&f_time[i], &s_time[i]);
                s_time[i].wHour += 2;
            }
            SendMessage(GetDlgItem(hDlg, 1001), CB_SETCURSEL, 0, 0);

            attr = GetFileAttributes(nazwa_pliku);

            if(attr & FILE_ATTRIBUTE_ARCHIVE)
                SendMessage(GetDlgItem(hDlg, 1002), BM_SETCHECK, 1, 0);
            if(attr & FILE_ATTRIBUTE_HIDDEN)
                SendMessage(GetDlgItem(hDlg, 1003), BM_SETCHECK, 1, 0);
            if(attr & FILE_ATTRIBUTE_READONLY)
                SendMessage(GetDlgItem(hDlg, 1004), BM_SETCHECK, 1, 0);
            if(attr & FILE_ATTRIBUTE_SYSTEM)
                SendMessage(GetDlgItem(hDlg, 1005), BM_SETCHECK, 1, 0);

            SendMessage(hDlg, WM_COMMAND, 1001 | (CBN_SELCHANGE<<16), 0);
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    attr = 0;
                    attr |= SendMessage(GetDlgItem(hDlg, 1002), BM_GETCHECK, 0, 0)==BST_CHECKED?FILE_ATTRIBUTE_ARCHIVE:0;
                    attr |= SendMessage(GetDlgItem(hDlg, 1003), BM_GETCHECK, 0, 0)==BST_CHECKED?FILE_ATTRIBUTE_HIDDEN:0;
                    attr |= SendMessage(GetDlgItem(hDlg, 1004), BM_GETCHECK, 0, 0)==BST_CHECKED?FILE_ATTRIBUTE_READONLY:0;
                    attr |= SendMessage(GetDlgItem(hDlg, 1005), BM_GETCHECK, 0, 0)==BST_CHECKED?FILE_ATTRIBUTE_SYSTEM:0;
                    SetFileAttributes(nazwa_pliku, attr);

                    for(int i = 0; i < 3; i++)
                    {
                        s_time[i].wHour -= 2;
                        SystemTimeToFileTime(&s_time[i], &f_time[i]);
                    }
                    plik = CreateFile(nazwa_pliku, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
                    SetFileTime(plik, &f_time[0], &f_time[1], &f_time[2]);
                    CloseHandle(plik);
                    EndDialog(hDlg, 0);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
                case 1000:
                    kind = SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);
                    GetLocalTime(&s_time[kind]);
                    SendMessage(hDlg, WM_COMMAND, 1001 | (CBN_SELCHANGE<<16), 0);
                    return FALSE;
                case 1001:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        kind = SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);

                        itoa(s_time[kind].wDay, tekst, 10);
                        SetWindowText(GetDlgItem(hDlg, 1009), tekst);
                        SendMessage(GetDlgItem(hDlg, 1010), CB_SETCURSEL, s_time[kind].wMonth - 1, 0);
                        itoa(s_time[kind].wYear, tekst, 10);
                        SetWindowText(GetDlgItem(hDlg, 1011), tekst);

                        itoa(s_time[kind].wHour, tekst, 10);
                        SetWindowText(GetDlgItem(hDlg, 1006), tekst);
                        itoa(s_time[kind].wMinute, tekst, 10);
                        SetWindowText(GetDlgItem(hDlg, 1007), tekst);
                        itoa(s_time[kind].wSecond, tekst, 10);
                        SetWindowText(GetDlgItem(hDlg, 1008), tekst);
                    }
                    return FALSE;
                case 1009:  // Day
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        kind = SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);
                        GetWindowText(GetDlgItem(hDlg, 1009), tekst, 200);
                        s_time[kind].wDay = atoi(tekst);
                    }
                    return FALSE;
                case 1010:  // Month
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        kind = SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);
                        s_time[kind].wMonth = SendMessage(GetDlgItem(hDlg, 1010), CB_GETCURSEL, 0, 0) + 1;
                    }
                    return FALSE;
                case 1011:  // Year
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        kind = SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);
                        GetWindowText(GetDlgItem(hDlg, 1011), tekst, 200);
                        s_time[kind].wYear = atoi(tekst);
                    }
                    return FALSE;
                case 1006:  // Hour
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        kind = SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);
                        GetWindowText(GetDlgItem(hDlg, 1006), tekst, 200);
                        s_time[kind].wHour = atoi(tekst);
                    }
                    return FALSE;
                case 1007:  // Minute
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        kind = SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);
                        GetWindowText(GetDlgItem(hDlg, 1007), tekst, 200);
                        s_time[kind].wMinute = atoi(tekst);
                    }
                    return FALSE;
                case 1008:  // Second
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        kind = SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);
                        GetWindowText(GetDlgItem(hDlg, 1008), tekst, 200);
                        s_time[kind].wSecond = atoi(tekst);
                    }
                    return FALSE;
            }
    }
    return FALSE;
}

