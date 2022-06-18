#include"mclip.h"

extern unsigned char* schowek;
extern int schoweklen;
extern HWND h_client_wnd;
extern unsigned char hexFind [];
extern int hexFind_size;
extern bool hexFind_forward;
extern const int STD_BUF;

MultiClipboard*MultiClipboard::vc[MultiClipboard::MAX_CLIP];
int MultiClipboard::total = 0;

MultiClipboard::MultiClipboard(unsigned char*d, int l)
{
    len = l;
    data = new unsigned char[len];
    CopyMemory(data, d, len);
    serial = total;
    total++;
}

MultiClipboard::MultiClipboard(HANDLE plik)
{
    DWORD br;
    ReadFile(plik, &len, sizeof(int), &br, NULL);
    data = new unsigned char[len];
    ReadFile(plik, data, len, &br, NULL);
    serial = total;
    total++;
}

MultiClipboard::~MultiClipboard()
{
    delete [] data;
    total--;
    for(int i = serial; i < total; i++)
    {
        vc[i] = vc[i + 1];
        vc[i]->serial--;
    }
}

void MultiClipboard::Paste()
{
    unsigned char*schowek_tmp = schowek;
    int schoweklen_tmp = schoweklen;
    schowek = data;
    schoweklen = len;
    SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
    schowek = schowek_tmp;
    schoweklen = schoweklen_tmp;
}

bool MultiClipboard::Find(HWND hwnd)
{
    if(len<STD_BUF)
    {
        hexFind_size = len;
        CopyMemory(hexFind, data, len);
        return ::Find(hexFind_forward);
    }
    return 1;
}

void MultiClipboard::SetMenu(HMENU hMenu)
{
    MENUITEMINFO mio;
    char tekst[400];
    int length, x;
    if(len>50) length = 50; else length = len;
    mio.cbSize = sizeof(MENUITEMINFO);
    mio.fMask = MIIM_TYPE;
    mio.fType = MFT_STRING;
    mio.dwTypeData = tekst;
    BytesToHex(tekst, data, length);
    Dots(tekst, 16);
    strcat(tekst, " - ");
    x=strlen(tekst);
    CopyMemory(tekst+x, data, length);
    tekst[x+length]=0;
    CopyMemory(tekst+40, "...\0", 4);

    EnableMenuItem(hMenu, 700 + serial, MF_ENABLED);
    SetMenuItemInfo(hMenu, 700 + serial, FALSE, &mio);
}

void MultiClipboard::Write(HANDLE plik)
{
    DWORD br;
    WriteFile(plik, &len, sizeof(int), &br, NULL);
    WriteFile(plik, data, len, &br, NULL);
}

void MultiClipboard::ReadAll()
{
    HANDLE plik;
    int tot;
    DWORD br;
    char nazwa_pliku[MAX_PATH];
    GetModulePath(nazwa_pliku);
    strcat(nazwa_pliku, "mclip.dat");
    plik = CreateFile(nazwa_pliku, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(plik != INVALID_HANDLE_VALUE)
    {
        ReadFile(plik, &tot, sizeof(int), &br, NULL);
        for(int i = 0; i < tot; i++)
            vc[MultiClipboard::total] = new MultiClipboard(plik);
        CloseHandle(plik);
    }
}

void MultiClipboard::WriteAll()
{
    HANDLE plik;
    DWORD br;
    char nazwa_pliku[MAX_PATH];
    GetModulePath(nazwa_pliku);
    strcat(nazwa_pliku, "mclip.dat");
    plik = CreateFile(nazwa_pliku, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    WriteFile(plik, &total, sizeof(int), &br, NULL);
    for(int i = 0; i < total; i++)
        vc[i]->Write(plik);
    CloseHandle(plik);
    while(total)
        delete vc[0];
}

void MultiClipboard::Copy(unsigned char*d, int l)
{
    if(!l)
        return;
    for(int i = 0; i < total; i++)
        if(vc[i]->len == l)
            if(!memcmp(vc[i]->data, d, l))
                return;
    if(total==MAX_CLIP)
        delete vc[0];
    vc[total] = new MultiClipboard(d, l);
}

