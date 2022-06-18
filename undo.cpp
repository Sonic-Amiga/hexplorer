#include"undo.h"

extern HWND h_client_wnd;
extern int selected_begin, selected_end, schoweklen;
extern unsigned char* pamiec, *schowek;
extern bool insert, redirected_by_undo;

int cUndo::step = 0;
bool cUndo::redirected_by_undo = 0;
cUndo* cUndo::undo[20000];

cUndo::cUndo(int m, int w, char* n)
{
    message = m;
    wParam = w;
    s_begin = selected_begin;
    s_end = selected_end;
    u_insert = insert;

    if(
        (message == WM_KEYDOWN && wParam == 46) ||
        (message == WM_CHAR && !u_insert) ||
        (message == WM_COMMAND && wParam == 125 && !u_insert) ||
        (message == WM_USER + 1)
      )
    {
        schoweklen = selected_end - selected_begin + 1;
        schowek = new unsigned char[schoweklen];
        CopyMemory(schowek, pamiec + selected_begin, schoweklen);
    }
    else
    {
        schowek = 0;
        schoweklen = 0;
    }

    if(message == WM_USER + 1)
        s_end = s_begin + ::schoweklen - 1;

    nazwa = n;
    step++;
}

cUndo::~cUndo()
{
    step--;
    delete [] schowek;
}

void cUndo::SetMenu(HMENU hMenu, int pos)
{
    char napis[200];
    strcpy(napis, "Undo ");
    strcat(napis, nazwa);
    strcat(napis, "\tCtrl+Z");
    MENUITEMINFO mio;
    mio.cbSize = sizeof(MENUITEMINFO);
    mio.fMask = MIIM_TYPE;
    mio.fType = MFT_STRING;
    mio.dwTypeData = napis;
    mio.cch = strlen(napis);
    SetMenuItemInfo(hMenu, pos, FALSE, &mio);
}

void cUndo::Undo()
{
    redirected_by_undo = 1;
    if(message == WM_KEYDOWN)
    {
        if(wParam == 46)
        {
            unsigned char* temp = ::schowek;
            int templen = ::schoweklen;
            bool temp_ins = insert;
            ::schowek = schowek;
            ::schoweklen = schoweklen;
            insert = 1;
            selected_begin = selected_end = s_begin;
            SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
            ::schowek = temp;
            ::schoweklen = templen;
            insert = temp_ins;
        }
        else if(wParam == 118)
        {
            selected_begin = s_begin;
            selected_end = s_end;
            SendMessage(h_client_wnd, WM_KEYDOWN, 119, 0);
        }
        else if(wParam == 119)
        {
            selected_begin = s_begin;
            selected_end = s_end;
            SendMessage(h_client_wnd, WM_KEYDOWN, 118, 0);
        }
    }
    else if(message == WM_CHAR)
    {
        if(u_insert)
        {
            selected_begin = selected_end = s_end;
            SendMessage(h_client_wnd, WM_KEYDOWN, 46, 0);
        }
        else
        {
            selected_begin = selected_end = s_end;
            SendMessage(h_client_wnd, WM_KEYDOWN, 46, 0);
            unsigned char* temp = ::schowek;
            int templen = ::schoweklen;
            bool temp_ins = insert;
            ::schowek = schowek;
            ::schoweklen = 1;
            insert = 1;
            SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
            ::schowek = temp;
            ::schoweklen = templen;
            insert = temp_ins;
        }
    }
    else if(message == WM_COMMAND)
    {
        if(wParam == 125)
        {
            selected_begin = s_begin;
            selected_end = s_end;
            if(u_insert)
                SendMessage(h_client_wnd, WM_KEYDOWN, 46, 0);
            else
            {
                unsigned char* temp = ::schowek;
                int templen = ::schoweklen;
                bool temp_ins = insert;
                ::schowek = schowek;
                ::schoweklen = schoweklen;
                insert = 0;
                SendMessage(h_client_wnd, WM_COMMAND, 125, 0);
                ::schowek = temp;
                ::schoweklen = templen;
                insert = temp_ins;
            }
        }
        else
        {
            selected_begin = s_begin;
            selected_end = s_end;
            SendMessage(h_client_wnd, message, wParam, 0);
        }
    }
    else if(message == WM_USER + 1)
    {
        selected_begin = s_begin;
        selected_end = s_end;
        unsigned char* temp = ::schowek;
        int templen = ::schoweklen;
        ::schowek = schowek;
        ::schoweklen = schoweklen;
        SendMessage(h_client_wnd, message, 0, 0);
        ::schowek = temp;
        ::schoweklen = templen;
    }
    redirected_by_undo = 0;
    SetStatus();
    delete this;
}

void cUndo::Forget()
{
    while(cUndo::step)
        delete cUndo::undo[cUndo::step - 1];
}

