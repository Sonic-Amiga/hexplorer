#include"hex_common.h"

#if BETA
extern HWND h_main_wnd, h_client_wnd;
extern bool insert;

void Info(char* tekst){ MessageBox(NULL, tekst, "ICY Hexplorer BETA TEST", 0); }
void InfoDig(unsigned int i)
{
    char t[STD_BUF];
    _ui64toa(i, t, 10);
    Info(t);
}   
void WyswietlBlad() { InfoDig(GetLastError()); }

void Type(int num)
{
    for(int i = 0; i < num; i++)
        SendMessage(h_main_wnd, WM_CHAR, i%17+0x30, 0);
}

void CutPaste(int num)
{
    for(int i = 0; i < num; i++)
    {
        SendMessage(h_client_wnd, WM_COMMAND, 123, 0);
        SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
    }
}

void RunTest1(int num)
{
    insert = 1;
    for(int i = 0; i < num; i++)
    {
        Type(100);
        SendMessage(h_client_wnd, WM_COMMAND, 136, 0);
        CutPaste(10);
    }
}

void RunTest()
{
    RunTest1(100);
}
#endif

void GetModulePath(char* p)
{
    GetModuleFileName(NULL, p, MAX_PATH - 1);
    p[strlen(p) - 10] = 0;
}

void CenterWindow(HWND hwnd)
{
    RECT r;
    int x, y;
    GetWindowRect(hwnd, &r);
    x=(r.right-r.left)/2;
    y=(r.bottom-r.top)/2;
    GetWindowRect(GetParent(hwnd), &r);
    x=(r.left+r.right)/2-x;
    y=(r.top+r.bottom)/2-y;
    SetWindowPos(hwnd, HWND_NOTOPMOST, x, y, 0, 0, SWP_NOSIZE);
}

void Dots(char* t, int radix)
{
    int cslen = strlen(t);
    if(radix==10)
    {
        for(int i = cslen - 1; i > 0; i--)
            if((cslen - i - 1) % 3 == 2)
            {
                for(int k = strlen(t); k >= i; k--)
                    t[k + 1] = t[k];
                t[i] = SEPARATOR;
            }
    }
    else if(radix==16 || radix==8)
    {
        for(int i = cslen - 1; i > 0; i--)
            if((cslen - i - 1) % 4 == 3)
            {
                for(int k = strlen(t); k >= i; k--)
                    t[k + 1] = t[k];
                t[i] = SEPARATOR;
            }
    }
    else if(radix==2)
    {
        for(int i = cslen - 1; i > 0; i--)
            if((cslen - i - 1) % 8 == 7)
            {
                for(int k = strlen(t); k >= i; k--)
                    t[k + 1] = t[k];
                t[i] = SEPARATOR;
            }
    }

}

unsigned char DecryptHex(unsigned char znak)
{
    switch(znak)
    {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a': case 'A': return 0xA;
        case 'b': case 'B': return 0xB;
        case 'c': case 'C': return 0xC;
        case 'd': case 'D': return 0xD;
        case 'e': case 'E': return 0xE;
        case 'f': case 'F': return 0xF;
    }
    return 0;
}

void TextToHex(char* dest, unsigned char* src)
{
    while(*src)
    {
        unsigned char chr = *src;

        if (strip_bit7) {
            chr |= 0x80;
        }

        *dest = ToHex4bits(chr >> 4);
        dest++;
        *dest = ToHex4bits(chr);
        dest++;
        src++;
    }
    *dest = 0;
}

void BytesToHex(char* dest, unsigned char* src, int size)
{
    for(int i = 0; i < size; i++)
    {
        *dest = ToHex4bits(*src >> 4);
        dest++;
        *dest = ToHex4bits(*src);
        dest++;
        src++;
    }
    *dest = 0;
}

void HexToText(unsigned char* dest, char* src, int size)
{
    for(int i = 0; i < size; i++)
    {
        dest[i] &= 0x0F;
        dest[i] |= (DecryptHex(src[i << 1]) << 4);

        dest[i] &= 0xF0;
        dest[i] |= DecryptHex(src[(i << 1) + 1]);
    }
}

void ToUnicode(char*str)
{
    int i=strlen(str);
    while(str)
    {
      str[i*2]=str[i];
      str[i*2+1]=0;
      str--;
    }
}

INT_PTR CALLBACK NewValueDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    static char szNew[STD_BUF];
    switch(message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            SetFocus(GetDlgItem(hDlg, 1000));
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    //char tekst[STD_BUF];
                    GetWindowText(GetDlgItem(hDlg, 1000), szNew, 200);
                    EndDialog(hDlg, (INT_PTR)szNew);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
            }
    }
    return FALSE;
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hDlg, 0);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
            }
    }
    return FALSE;
}


