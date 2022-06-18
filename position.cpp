#include"position.h"

extern HWND h_client_wnd;
extern int selected_begin, selected_end, hexFind_from, column_num, dlugosc_pliku;

int cPosition::num = 0;

int cPosition::draw_pos[MAX_POSITION];
int cPosition::draw_posnum;


cPosition::cPosition()
{
    //description = 0;
    Reset();
    lp = num++;
}
cPosition::cPosition(cPosition&wzorzec)
{
    lp = wzorzec.lp;
    pos = wzorzec.pos;
    strcpy(description, wzorzec.description);
    num++;
}
cPosition::~cPosition()
{
    num--;
}
void cPosition::GoTo()
{
    int diff;
    diff = selected_end - selected_begin;
    if(pos + diff < dlugosc_pliku)
    {
        selected_begin = pos;
        selected_end = selected_begin + diff;
        hexFind_from = selected_begin;
        ScrollTo(pos);
        SetStatus();
        InvalidateRect(h_client_wnd, NULL, 0);
    }
}
void cPosition::Reset()
{
    pos = 0;
    strcpy(description, "Begining of the file");
}
void cPosition::Set(int p, char* d) //  = "\0\0"
{
    pos = p;
    if(d[0])
        strcpy(description, d);
    else
    {
        strcpy(description, "Position #");
        _ui64toa(lp, description + 10, 10);
    }
}
void cPosition::SetMenu(HMENU hMenu, int p)
{
    char napis[200];
    strcpy(napis, "#&");
    _ui64toa(p, napis + strlen(napis), 10);
    strcat(napis, " (");
    strcat(napis, description);
    strcat(napis, ")");
    MENUITEMINFO mio;
    mio.cbSize = sizeof(MENUITEMINFO);
    mio.fMask = MIIM_TYPE;
    mio.fType = MFT_STRING;
    mio.dwTypeData = napis;
    mio.cch = strlen(napis);
    if(mio.cch > 40)
    {
        mio.dwTypeData += mio.cch - 40;
        mio.cch = 40;
    }
    SetMenuItemInfo(hMenu, 190 + p, FALSE, &mio);
}
bool cPosition::DrawContextMenu(HINSTANCE hInstance, HWND hwnd,int x, int y, int p)
{
                if(p==pos)
                {
                  HMENU hPopupClear;
                  hPopupClear = LoadMenu(hInstance, "REM_BOOK");
                  hPopupClear = GetSubMenu(hPopupClear, 0);
                  char nazwa[100];
                  strcpy(nazwa, description);
                  strcat(nazwa, " in Navigator");

    MENUITEMINFO mio;
    mio.cbSize = sizeof(MENUITEMINFO);
    mio.fMask = MIIM_TYPE;
    mio.fType = MFT_STRING;
    mio.dwTypeData = nazwa;
    mio.cch = strlen(nazwa);
    //EnableMenuItem(hMenu, 200 + pos, MF_ENABLED);
    SetMenuItemInfo(hPopupClear, 157, FALSE, &mio);

                  TrackPopupMenu(hPopupClear, TPM_RIGHTBUTTON, x, y, 0, hwnd, NULL);
                  return true;
                }
                return false;

}
void cPosition::PrintForNavigator(char*p, int radix)
{
    int len;
    itoa(pos, p, radix);
    Dots(p, radix);

    for(int k = 15 - strlen(p); k >= 0; k--)
        strcat(p, "  ");


    len = strlen(p);
    if((pos - selected_begin)<0)
    {
      len++;
      strcat(p, "-");
      itoa(-(pos - selected_begin), p + len, radix);
    }
    else
      itoa(pos - selected_begin, p + len, radix);

    Dots(p + len, radix);

    strcat(p, "    (");
    strcat(p, description);
    strcat(p, ")");
}


