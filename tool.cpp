#include"tool.h"

extern HINSTANCE hInstance;
extern HWND h_main_wnd, h_client_wnd, h_tool_bar;
extern int selected_begin, selected_end, highlight_struct, highlight_struct_size, dlugosc_pliku,
    cxClient, cyClient, schoweklen,
    thread_num;
extern unsigned char highlight[];
extern int highlight_size;
extern unsigned char* pamiec, *schowek;
extern char* szAppName;
extern bool insert, saved, align_structures;
extern int sys_COLOR_3DFACE;
extern HMENU hPopupRadixMenu, hPopupBPPMenu;

char*simple_data_names[]={
  "byte ",
  "signed byte ",
  "word ",
  "signed word ",
  "double word ",
  "signed double word ",
  "quad ",
  "signed quad ",
  "float ",
  "double ",
  "word motorola ",
  "double word motorola",
  "quad motorola ",
};

// class member

member::member()
{
    strcpy(name, "noname");
    typ = t_byte;
    rep = 1;
}

member::member(const char* n, type t, int r) //  = 1
{
    strcpy(name, n);
    typ = t;
    rep = r;
}

void member::GetDigit(const void* adres, char* numpos, int radix, bool dots)
{
  if(typ>9)
    IntelMotorola((unsigned char*)adres, GetSize());
  switch(typ)
  {
      case t_byte:
          _ultoa(*(unsigned char*)adres, numpos, radix);
          if(dots)Dots(numpos, radix);
          break;
      case t_sbyte:
          itoa(*(signed char*)adres, numpos, radix);
          if(dots)Dots(numpos, radix);
          break;
      case t_word: case t_word_motorola:
          _ultoa(*(unsigned short*)adres, numpos, radix);
          if(dots)Dots(numpos, radix);
          break;
      case t_sword:
          itoa(*(signed short*)adres, numpos, radix);
          if(dots)Dots(numpos, radix);
          break;
      case t_dword: case t_dword_motorola:
          _ultoa(*(unsigned int*)adres, numpos, radix);
          if(dots)Dots(numpos, radix);
          break;
      case t_sdword:
          itoa(*(signed int*)adres, numpos, radix);
          if(dots)Dots(numpos, radix);
          break;
      case t_quad: case t_quad_motorola:
          _ui64toa(*(unsigned __int64*)adres, numpos, radix);
          if(dots)Dots(numpos, radix);
          break;
      case t_squad:
          _i64toa(*(signed __int64*)adres, numpos, radix);
          if(dots)Dots(numpos, radix);
          break;
      case t_float:
          gcvt(*(float*)adres, 12, numpos);
          break;
      case t_double:
          gcvt(*(double*)adres, 12, numpos);
  }
  if(typ>9)
    IntelMotorola((unsigned char*)adres, GetSize());

}

void member::DeriveInfo(const void* adres, char* dst, int radix) //  = 10
{
    strcpy(dst, simple_data_names[typ]);
    strcat(dst, name);
    if(rep == 1)
    {
        char* numpos;
        strcat(dst, " = ");
        numpos = dst + strlen(dst);
        GetDigit(adres,numpos,radix,1);
    }
    else
    {
        strcat(dst, "[");
        itoa(rep, dst + strlen(dst), 10);
        strcat(dst, "]");
    }
}

int member::GetSize()
{
    int size;
    switch(typ)
    {
        case t_byte:
        case t_sbyte:
            size = sizeof(char);
            break;
        case t_word:
        case t_sword:
        case t_word_motorola:
            size = sizeof(short);
            break;
        case t_dword:
        case t_sdword:
        case t_dword_motorola:
        case t_float:
            size = sizeof(int);
            break;
        case t_quad:
        case t_squad:
        case t_quad_motorola:
        case t_double:
            size = sizeof(__int64);
    }
    return size * rep;
}

void member::SetValue(const void* adres, char* szNewValue)
{
    if((unsigned char*)adres + GetSize() <= pamiec + dlugosc_pliku)
    {
        int sb_temp = selected_begin;
        int se_temp = selected_end;
        selected_begin = (unsigned int)adres - (unsigned int)pamiec;
        selected_end = selected_begin + GetSize() - 1;
        bool temp_ins = insert;
        insert = 0;
        cUndo::Store(WM_COMMAND, 125, "Set Value");
        insert = temp_ins;
        selected_begin = sb_temp;
        selected_end = se_temp;
        saved = 0;
        if(rep == 1)
        {
            switch(typ)
            {
                case t_byte:
                    *(unsigned char*)adres = atoi(szNewValue);
                    return;
                case t_sbyte:
                    *(signed char*)adres = atoi(szNewValue);
                    return;
                case t_word: case t_word_motorola:
                    *(unsigned short*)adres = atoi(szNewValue);
                    break;
                case t_sword:
                    *(signed short*)adres = atoi(szNewValue);
                    return;
                case t_dword: case t_dword_motorola:
                    *(unsigned int*)adres = atoi(szNewValue);
                    break;
                case t_sdword:
                    *(signed int*)adres = atoi(szNewValue);
                    return;
                case t_quad: case t_quad_motorola:
                    *(unsigned __int64*)adres = _atoi64(szNewValue);
                    break;
                case t_squad:
                    *(signed __int64*)adres = _atoi64(szNewValue);
                    return;
                case t_float:
                    *(float*)adres = atof(szNewValue);
                    return;
                case t_double:
                    *(double*)adres = atof(szNewValue);
                    return;
            }
            if(typ>9)
              IntelMotorola((unsigned char*)adres, GetSize());
        }
        else
            rep = atoi(szNewValue);
    }
}

void member::Write(HANDLE plik)
{
    DWORD br;
    WriteFile(plik, name, strlen(name) + 1, &br, 0);
    WriteFile(plik, &typ, sizeof(type), &br, 0);
    WriteFile(plik, &rep, sizeof(int), &br, 0);
}

// class structure

BOOL CALLBACK AddStructDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
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
                    char tekst[200];
                    GetWindowText(GetDlgItem(hDlg, 1000), tekst, 200);
                    structure::Register(new structure(tekst));
                    EndDialog(hDlg, 0);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
            }
    }
    return FALSE;
}

BOOL CALLBACK AddMemberDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    static member m;
    switch(message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            SetFocus(GetDlgItem(hDlg, 1000));
            for(int i=0;i<13;i++)
              SendMessage(GetDlgItem(hDlg, 1001), CB_ADDSTRING, 0, (LPARAM)simple_data_names[i]);
            SendMessage(GetDlgItem(hDlg, 1001), CB_SETCURSEL, 0, 0);
            SetWindowText(GetDlgItem(hDlg, 1002), "1");
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    char tekst[200];
                    type typ;
                    int rep;

                    GetWindowText(GetDlgItem(hDlg, 1002), tekst, 200);
                    rep = atoi(tekst);
                    typ = (type)SendMessage(GetDlgItem(hDlg, 1001), CB_GETCURSEL, 0, 0);
                    GetWindowText(GetDlgItem(hDlg, 1000), tekst, 200);
                    m = member(tekst, typ, rep);
                    EndDialog(hDlg, (int)&m);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return FALSE;
            }
    }
    return FALSE;
}

int structure::num = 0;
structure* structure::st[500];

void structure::Write(HANDLE plik)
{
    DWORD br;
    WriteFile(plik, nazwa, strlen(nazwa) + 1, &br, 0);
    WriteFile(plik, &mem_num, sizeof(int), &br, 0);
    for(int i = 0; i < mem_num; i++)
        skladniki[i]->Write(plik);
}

structure::structure(char* n)
{
    serial = num;
    strcpy(nazwa, n);
    mem_num = 0;
    num++;
    MENUITEMINFO mio;
    mio.cbSize = sizeof(MENUITEMINFO);
    mio.fMask = MIIM_TYPE | MIIM_ID;
    mio.wID = 300 + serial;
    mio.fType = MFT_STRING;
    mio.dwTypeData = nazwa;
    mio.cch = strlen(nazwa);
    HMENU struct_menu = GetSubMenu(GetMenu(h_main_wnd), 4);
    InsertMenuItem(struct_menu, serial, TRUE, &mio);
}

structure::~structure()
{
    MENUITEMINFO mio;
    mio.cbSize = sizeof(MENUITEMINFO);
    mio.fMask = MIIM_TYPE | MIIM_ID;

    DeleteMenu(GetMenu(h_main_wnd), 300 + serial, MF_BYCOMMAND);
    for(int i = 0; i < mem_num; i++)
        delete skladniki[i];
    num--;
    for(int i = serial; i < num; i++)
    {
        st[i] = st[i + 1];
        st[i]->serial--;
        mio.wID = 300 + st[i]->serial;
        mio.fType = MFT_STRING;
        mio.dwTypeData = st[i]->nazwa;
        mio.cch = strlen(st[i]->nazwa);
        SetMenuItemInfo(GetMenu(h_main_wnd), 300 + st[i]->serial + 1, FALSE, &mio);
    }
}

void structure::AddMember(const char* n, type t, int r) //  = 1
{
    skladniki[mem_num++] = new member(n, t, r);
}
void structure::AddMember(member* m, int numer)
{
    if(numer==LB_ERR)
        skladniki[mem_num] = new member(*m);
    else
    {
        for(int i = mem_num; i > numer; i--)
            skladniki[i] = skladniki[i - 1];
        skladniki[numer] = new member(*m);
    }
    mem_num++;
}
void structure::RemoveMember(int numer)
{
    if(mem_num)
    {
        delete skladniki[numer];
        mem_num--;
        for(int i = numer; i < mem_num; i++)
            skladniki[i] = skladniki[i + 1];
    }
}
void structure::CloneMember(int numer)
{
    if(mem_num)
    {
        skladniki[mem_num] = new member(*skladniki[mem_num - 1]);
        for(int i = mem_num - 1; i > numer; i--)
            skladniki[i] = skladniki[i - 1];
        mem_num++;
    }
}
void structure::SetValue(int m, char* szNewValue)
{
    int offset = 0;
    for(int i = 0; i < m; i++)
        offset += skladniki[i]->GetSize();
    skladniki[m]->SetValue(pamiec + selected_begin + offset, szNewValue);
}
void structure::SetHighlight(int m)
{
    int offset = 0;
    for(int i = 0; i < m; i++)
        offset += skladniki[i]->GetSize();
    highlight_struct = selected_begin + offset;
    highlight_struct_size = skladniki[m]->GetSize();
    InvalidateRect(h_client_wnd, NULL, 0);
}
int structure::GetSize()
{
    int size = 0;
    for(int i = 0; i < mem_num; i++)
        size += skladniki[i]->GetSize();
    return size;
}
int structure::PrintMembers(char tekst[200][200], int radix) //  = 10
{
    int offset = 0;
    for(int i = 0; i < mem_num; i++)
    {
        skladniki[i]->DeriveInfo(pamiec + selected_begin + offset, tekst[i], radix);
        offset += skladniki[i]->GetSize();
    }
    return mem_num;
}
void structure::ReadAll()
{
    HANDLE plik;
    unsigned char b[10000];
    unsigned char* bufor = b;
    int file_size, to_create, to_add_members;
    DWORD br;
    char module_name[MAX_PATH];
    //GetModuleFileName(NULL, module_name, MAX_PATH - 1);
    //strcpy(module_name + strlen(module_name) - 10, "structures.dat");
    GetModulePath(module_name);
    strcat(module_name, "structures.dat");
    plik = CreateFile(module_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    //MessageBox(h_main_wnd, module_name, szAppName, 0);
    if(plik == INVALID_HANDLE_VALUE)
        MessageBox(h_main_wnd, "Program files corrupted", szAppName, 0);
    else
    {
        file_size = GetFileSize(plik, NULL);
        ReadFile(plik, bufor, file_size, &br, NULL);
        CloseHandle(plik);
        to_create = *(int*)bufor;   bufor += sizeof(int);
        for(int i = 0; i < to_create; i++)
        {
            structure *s = new structure((char*)bufor);

            structure::Register(s);
            bufor += strlen((char*)bufor) + 1;
            to_add_members = *(int*)bufor;  bufor += sizeof(int);
            for(int k = 0; k < to_add_members; k++)
            {
                s->AddMember(
                    (char*)bufor,
                    *(type*)(bufor + strlen((char*)bufor) + 1),
                    *(int*)(bufor + strlen((char*)bufor) + 1 + sizeof(type))
                    );
                bufor += strlen((char*)bufor) + 1 + sizeof(type) + sizeof(int);
            }
        }
    }
}
void structure::WriteAll()
{
    HANDLE plik;
    DWORD br;
    char module_name[MAX_PATH];
    //GetModuleFileName(NULL, module_name, MAX_PATH - 1);
    //strcpy(module_name + strlen(module_name) - 13, "structures.dat");
    GetModulePath(module_name);
    strcat(module_name, "structures.dat");

    plik = CreateFile(module_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    WriteFile(plik, &num, sizeof(int), &br, 0);
    for(int i = 0; i < num; i++)
        st[i]->Write(plik);
    CloseHandle(plik);
    while(num)
        delete st[0];
}

// class ChildDlg

int ChildDlg::num = 0;
ChildDlg* ChildDlg::svd[30];

void ChildDlg::Align()
{
    int iHeightChange;
    RECT rect;
    POINT p;

    GetWindowRect(hDlg, &rect);
    iHeightChange=rect.bottom-rect.top;
    SetWindowPos(hDlg, 0, 0, 0, rect.right-rect.left, cyClient/num, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    iHeightChange=cyClient/num-iHeightChange;
    HWND hLista=GetDlgItem(hDlg, 1001);
    GetWindowRect(hLista, &rect);
    SetWindowPos(hLista, 0, 0, 0, rect.right-rect.left, rect.bottom-rect.top+iHeightChange, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

    GetWindowRect(hDlg, &rect);
    p.x = rect.right - rect.left;
    p.y = rect.bottom - rect.top;
    p.x = cxClient - p.x;
    p.y *= serial;
    ClientToScreen(h_client_wnd, &p);
    SetWindowPos(hDlg, 0, p.x, p.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}
void ChildDlg::InitTool()
{
    RECT rect;
    POINT p;
    SetWindowLong(hDlg, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
    GetWindowRect(hDlg, &rect);
    p.x = rect.right - rect.left;
    p.y = rect.bottom - rect.top;
    p.x = cxClient - p.x;
    p.y *= serial;
    ClientToScreen(h_client_wnd, &p);
    SetWindowPos(hDlg, 0, p.x, p.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
    ShowWindow(hDlg, SW_SHOW);
}

ChildDlg::ChildDlg()
{
    serial = num;
    num++;
}
ChildDlg::~ChildDlg()
{
    DestroyWindow(hDlg);
    num--;
    for(int i = serial; i < num; i++)
    {
        svd[i] = svd[i + 1];
        svd[i]->serial--;
    }
    if(align_structures)
        AlignAll();
}
ChildDlg* ChildDlg::GetThis(HWND h)
{
    for(int i = 0; i < num; i++)
        if(svd[i]->hDlg == h)
            return svd[i];
    return 0;
}
void ChildDlg::free(HWND hDlg)
{
    delete GetThis(hDlg);
}
void ChildDlg::AlignAll()
{
    for(int k = 0; k < num; k++)
        svd[k]->Align();
}
void ChildDlg::DisplayMembers()
{
    for(int k = 0; k < num; k++)
        svd[k]->Display();
}

// class ListChildDlg

ListChildDlg::ListChildDlg()
{
    top_index = 0;
    radix = 10;
}
void ListChildDlg::SetRadix(unsigned char r)
{
    radix = r;
    Display();
}

// class SViewerDlg

BOOL CALLBACK SViewDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
            SetFocus(GetDlgItem(hDlg, 1000));
            for(int i = 0; i < structure::num; i++)
                SendMessage(GetDlgItem(hDlg, 1000), CB_ADDSTRING, 0, (LPARAM)structure::st[i]->GetName());
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 1000:
                    if(HIWORD(wParam) == CBN_DROPDOWN)
                    {
                        SendMessage(GetDlgItem(hDlg, 1000), CB_RESETCONTENT, 0, 0);
                        for(int i = 0; i < structure::num; i++)
                            SendMessage(GetDlgItem(hDlg, 1000), CB_ADDSTRING, 0, (LPARAM)structure::st[i]->GetName());
                    }
                    else if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        SViewerDlg::DisplayMembers();
                    }
                    break;
                case 1001:
                    if(HIWORD(wParam) == LBN_KILLFOCUS)
                    {
                        ((SViewerDlg*)ChildDlg::GetThis(hDlg))->SetTopIndex(SendMessage(GetDlgItem(hDlg, 1001), LB_GETTOPINDEX, 0, 0));
                        highlight_struct_size = 0;
                    }
                    else if(HIWORD(wParam) == LBN_SELCHANGE)
                        SViewerDlg::SetHighlight(hDlg, SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0));
                    else if(HIWORD(wParam) == LBN_DBLCLK)
                    {
                        char* val = (char*)DialogBox(hInstance, "NEWVALUE", h_client_wnd, NewValueDlgProc);
                        if(val)
                            SViewerDlg::SetValue(hDlg, SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0), val);
                    }
                    break;
                case 1002:
                    SViewerDlg::DeleteStructure(hDlg);
                    break;
                case 1003:
                    SViewerDlg::AddMember(hDlg, SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0));
                    break;
                case 1004:
                    SViewerDlg::RemoveMember(hDlg, SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0));
                    break;
                case 1005:
                    POINT point;
                    RECT rect;
                    GetClientRect(hDlg, &rect);
                    point.x = rect.right;
                    point.y = 0;
                    ClientToScreen(hDlg, &point);
                    TrackPopupMenu(hPopupRadixMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hDlg, NULL);
                    break;
                case 1007:
                    SViewerDlg::Clone(hDlg, SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0));
                    break;
                case 200:
                    ((SViewerDlg*)ChildDlg::GetThis(hDlg))->SetRadix(2);
                    break;
                case 201:
                    ((SViewerDlg*)ChildDlg::GetThis(hDlg))->SetRadix(8);
                    break;
                case 202:
                    ((SViewerDlg*)ChildDlg::GetThis(hDlg))->SetRadix(10);
                    break;
                case 203:
                    ((SViewerDlg*)ChildDlg::GetThis(hDlg))->SetRadix(16);
                    break;
            }
            return FALSE;
        /*
        case WM_KILLFOCUS:
            ((SViewerDlg*)ChildDlg::GetThis(hDlg))->SetTopIndex(SendMessage(GetDlgItem(hDlg, 1001), LB_GETTOPINDEX, 0, 0));
            return FALSE;*/
        case WM_CLOSE:
            ChildDlg::free(hDlg);
            return FALSE;
    }
    return FALSE;
}

SViewerDlg::SViewerDlg(structure* s)
{
    char tekst[100];
    tresc = s;
    hDlg = CreateDialog(hInstance, "SVIEW", h_client_wnd, SViewDlgProc);
    SendMessage(GetDlgItem(hDlg, 1000), CB_SETCURSEL, tresc->GetSerial(), 0);
    SendMessage(GetDlgItem(hDlg, 1002), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(530)));
    SendMessage(GetDlgItem(hDlg, 1003), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(531)));
    SendMessage(GetDlgItem(hDlg, 1004), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(532)));
    SendMessage(GetDlgItem(hDlg, 1005), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(533)));
    SendMessage(GetDlgItem(hDlg, 1007), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(535)));

    TOOLINFO ti;
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
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = h_tool_bar;
    ti.hinst = hInstance;
    char* tips[] = {"Select structure", "", "Delete structure", "Add member", "Remove member", "Radix", "Structure size", "Clone member"};
    for(int i = 0; i < 8; i++)
    {
        ti.uId = (unsigned int)GetDlgItem(hDlg, 1000 + i);
        ti.lpszText = tips[i];
        SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
    }
    InitTool();
    Display();
}

void SViewerDlg::Display()
{
    char skladniki[200][200];
    char tekst[50];

    int selected = SendMessage(GetDlgItem(hDlg, 1000), CB_GETCURSEL, 0, 0);
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, FALSE, 0);
    SendMessage(GetDlgItem(hDlg, 1001), LB_RESETCONTENT, 0, 0);
    if(selected != CB_ERR)
    {
        tresc = structure::st[selected];
        int mem_num = tresc->PrintMembers(skladniki, radix);
        for(int i = 0; i < mem_num; i++)
            SendMessage(GetDlgItem(hDlg, 1001), LB_INSERTSTRING, i, (LPARAM)skladniki[i]);
        itoa(tresc->GetSize(), tekst, 10);
        strcat(tekst, " bytes");
        SetWindowText(GetDlgItem(hDlg, 1006), tekst);
    }
    else
        SetWindowText(GetDlgItem(hDlg, 1006), "Empty");
    SendMessage(GetDlgItem(hDlg, 1001), LB_SETTOPINDEX, top_index, 0);
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, TRUE, 0);
}
void SViewerDlg::SetValue(HWND hDlg, int m, char* szNewValue)
{
    SViewerDlg* s = (SViewerDlg*)GetThis(hDlg);
    s->tresc->SetValue(m, szNewValue);
    InvalidateRect(h_client_wnd, NULL, 0);
    SetStatus();
}
void SViewerDlg::SetHighlight(HWND hDlg, int m)
{
    SViewerDlg* s = (SViewerDlg*)GetThis(hDlg);
    s->tresc->SetHighlight(m);
}
void SViewerDlg::AddMember(HWND hDlg, int numer)
{
    SViewerDlg* s = (SViewerDlg*)GetThis(hDlg);
    if(SendMessage(GetDlgItem(hDlg, 1000), CB_GETCURSEL, 0, 0) != CB_ERR)
    {
        member* new_member;
        new_member = (member*)DialogBox(hInstance, "ADDMEMBER", h_client_wnd, AddMemberDlgProc);
        if(new_member)
        {
            s->tresc->AddMember(new_member, numer);
            SViewerDlg::DisplayMembers();
        }
    }
}
void SViewerDlg::RemoveMember(HWND hDlg, int numer)
{
    SViewerDlg* s = (SViewerDlg*)GetThis(hDlg);
    if(
        SendMessage(GetDlgItem(hDlg, 1000), CB_GETCURSEL, 0, 0) != CB_ERR &&
        SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0) != LB_ERR
      )
    {
        s->tresc->RemoveMember(numer);
        SViewerDlg::DisplayMembers();
    }
}
void SViewerDlg::DeleteStructure(HWND hDlg)
{
    SViewerDlg* s = (SViewerDlg*)GetThis(hDlg);
    if(SendMessage(GetDlgItem(hDlg, 1000), CB_GETCURSEL, 0, 0) != CB_ERR && MessageBox(h_main_wnd, "Delete this structure?", szAppName, MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        delete s->tresc;
        SendMessage(GetDlgItem(hDlg, 1000), CB_SETCURSEL, (WPARAM)CB_ERR, 0);
        SViewerDlg::DisplayMembers();
    }
}
void SViewerDlg::Clone(HWND hDlg, int numer)
{
    SViewerDlg* s = (SViewerDlg*)GetThis(hDlg);
    if(
        SendMessage(GetDlgItem(hDlg, 1000), CB_GETCURSEL, 0, 0) != CB_ERR &&
        SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0) != LB_ERR
      )
    {
        s->tresc->CloneMember(numer);
        SViewerDlg::DisplayMembers();
    }
}

// class SimpleDataDlg

BOOL CALLBACK SimpleDataDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    char tekst[200];
    //static int top_index;
    switch(message)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 1001:
                    if(HIWORD(wParam) == LBN_KILLFOCUS)
                    {
                        ((SimpleDataDlg*)ChildDlg::GetThis(hDlg))->SetTopIndex(SendMessage(GetDlgItem(hDlg, 1001), LB_GETTOPINDEX, 0, 0));
                        highlight_struct_size = 0;
                    }
                    else if(HIWORD(wParam) == LBN_SELCHANGE)
                        ((SimpleDataDlg*)ChildDlg::GetThis(hDlg))->SetHighlight();
                    else if(HIWORD(wParam) == LBN_DBLCLK)
                        ((SimpleDataDlg*)ChildDlg::GetThis(hDlg))->SetValue();
                    break;
                case 1009:
                  ((SimpleDataDlg*)ChildDlg::GetThis(hDlg))->CopyToClipboard();
                  break;
                case 1008:
                    POINT point;
                    RECT rect;
                    GetClientRect(hDlg, &rect);
                    point.x = 0;
                    point.y = 0;
                    ClientToScreen(hDlg, &point);
                    TrackPopupMenu(hPopupRadixMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hDlg, NULL);
                    break;
                case 200:
                    ((SimpleDataDlg*)ChildDlg::GetThis(hDlg))->SetRadix(2);
                    break;
                case 201:
                    ((SimpleDataDlg*)ChildDlg::GetThis(hDlg))->SetRadix(8);
                    break;
                case 202:
                    ((SimpleDataDlg*)ChildDlg::GetThis(hDlg))->SetRadix(10);
                    break;
                case 203:
                    ((SimpleDataDlg*)ChildDlg::GetThis(hDlg))->SetRadix(16);
                    break;
            }
            return TRUE;
        case WM_CLOSE:
            ChildDlg::free(hDlg);
            return FALSE;
    }
    return FALSE;
}

member SimpleDataDlg::all_data[13] =
{
    member(" ", t_byte, 1),
    member(" ", t_sbyte, 1),
    member(" ", t_word, 1),
    member(" ", t_sword, 1),
    member(" ", t_dword, 1),
    member(" ", t_sdword, 1),
    member(" ", t_quad, 1),
    member(" ", t_squad, 1),
    member(" ", t_float, 1),
    member(" ", t_double, 1),
    member(" ", t_word_motorola, 1),
    member(" ", t_dword_motorola, 1),
    member(" ", t_quad_motorola, 1),
};

SimpleDataDlg::SimpleDataDlg()
{
    hDlg = CreateDialog(hInstance, "SIMPLEVIEW", h_client_wnd, SimpleDataDlgProc);
    SendMessage(GetDlgItem(hDlg, 1008), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(533)));
    SendMessage(GetDlgItem(hDlg, 1009), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(535)));//clone

    TOOLINFO ti;
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
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = h_tool_bar;
    ti.hinst = hInstance;
    char* tips[] = {"Radix", "Copy to clipboard"};

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1008);
    ti.lpszText = tips[0];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1009);
    ti.lpszText = tips[1];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    InitTool();
    Display();
}
void SimpleDataDlg::Display()
{
    char tekst[200];
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, FALSE, 0);
    SendMessage(GetDlgItem(hDlg, 1001), LB_RESETCONTENT, 0, 0);
    for(int i = 0; i < 13; i++)
    {
      all_data[i].DeriveInfo(pamiec + selected_begin, tekst, radix);
      SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    }
    SendMessage(GetDlgItem(hDlg, 1001), LB_SETTOPINDEX, top_index, 0);
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, TRUE, 0);
}
void SimpleDataDlg::SetValue()
{
    char* val = (char*)DialogBox(hInstance, "NEWVALUE", h_client_wnd, NewValueDlgProc);
    if(val)
    {
        all_data[SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0)].SetValue(pamiec + selected_begin, val);
        InvalidateRect(h_client_wnd, NULL, 0);
        SetStatus();
    }
}

void SimpleDataDlg::SetHighlight()
{
    highlight_struct = selected_begin;
    highlight_struct_size = all_data[SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0)].GetSize();
    InvalidateRect(h_client_wnd, NULL, 0);
}

void SimpleDataDlg::CopyToClipboard()
{
    HGLOBAL hGlobal;
    unsigned char* pGlobal;
    char tekst[200];

    all_data[SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0)].GetDigit(pamiec + selected_begin, tekst, radix,0);

    hGlobal = GlobalAlloc(GHND | GMEM_SHARE, strlen(tekst) + 1);
    pGlobal = (unsigned char*)GlobalLock(hGlobal);
    CopyMemory(pGlobal, tekst, strlen(tekst) + 1);
    GlobalUnlock(hGlobal);
    OpenClipboard(hDlg);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hGlobal);
    CloseClipboard();
    delete [] schowek;
    schowek = new unsigned char[strlen(tekst)];
    CopyMemory(schowek, tekst, strlen(tekst));
    schoweklen = strlen(tekst);
    MultiClipboard::Copy(schowek, schoweklen);

}

// class ChecksumDlg

BOOL CALLBACK ChecksumDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    char tekst[200];
    static int top_index;
    switch(message)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 1001:
                    if(HIWORD(wParam) == LBN_KILLFOCUS)
                    {
                        ((ChecksumDlg*)ChildDlg::GetThis(hDlg))->SetTopIndex(SendMessage(GetDlgItem(hDlg, 1001), LB_GETTOPINDEX, 0, 0));
                        highlight_struct_size = 0;
                    }
                    else if(HIWORD(wParam) == LBN_DBLCLK)
                        ((ChecksumDlg*)ChildDlg::GetThis(hDlg))->CopyToClipboard(SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0));
                    break;
            }
            return TRUE;
        case WM_CLOSE:
            ChildDlg::free(hDlg);
            return FALSE;
    }
    return FALSE;
}

//const unsigned int ChecksumDlg::crc16table[256];
//const unsigned int ChecksumDlg::crctttable[256];
    const unsigned int ChecksumDlg::crc16table[]=
    {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
    };
    const unsigned int ChecksumDlg::crctttable[]=
    {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
        0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
        0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
        0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
        0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
        0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
        0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
        0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
        0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
        0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
        0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
        0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
        0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
        0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
    };


unsigned int ChecksumDlg::crc32table[256];

void ChecksumDlg::Checksum8(int begin, int end, char* t)
{
    unsigned char checksum = 0;
    for(unsigned int i = begin; i <= end; i++)
        checksum ^= (unsigned char)pamiec[i];
    _ui64toa(checksum, t, radix);
    Dots(t, radix);
}
void ChecksumDlg::Checksum16(int begin, int end, char* t)
{
    unsigned short checksum = 0;
    for(unsigned int i = begin; i <= end; i++)
        checksum ^= (unsigned short)pamiec[i] << (((i - begin) % 2) << 3);
    _ui64toa(checksum, t, radix);
    Dots(t, radix);
}
void ChecksumDlg::Checksum32(int begin, int end, char* t)
{
    unsigned int checksum = 0;
    for(unsigned int i = begin; i <= end; i++)
        checksum ^= (unsigned int)pamiec[i] << (((i - begin) % 4) << 3);
    _ui64toa(checksum, t, radix);
    Dots(t, radix);
}
void ChecksumDlg::Checksum64(int begin, int end, char* t)
{
    unsigned __int64 checksum = 0;
    for(unsigned int i = begin; i <= end; i++)
        checksum ^= (unsigned __int64)pamiec[i] << (((i - begin) % 8) << 3);
    _ui64toa(checksum, t, radix);
    Dots(t, radix);
}
void ChecksumDlg::CRC16(int begin, int end, char* t)
{
    unsigned short CRC = 0;
    for(unsigned int i = begin; i <= end; i++)
        CRC = (CRC >> 8) ^ crc16table[(CRC^pamiec[i]) & 0xff];
    _ui64toa(CRC, t, radix);
    Dots(t, radix);
}


void ChecksumDlg::CRC16CCITT(int begin, int end, char* t)
{
    unsigned short CRC = 0xffff;
    for(unsigned int i = begin; i <= end; i++)
        CRC = (CRC << 8) ^ crctttable[(CRC>>8)^pamiec[i]];
    _ui64toa(CRC, t, radix);
    Dots(t, radix);
}

void ChecksumDlg::CRC32(int begin, int end, char* t)
{
    unsigned int CRC = 0xffffffff;
    for(unsigned int i = begin; i <= end; i++)
        CRC = ((CRC) >> 8) ^ crc32table[(pamiec[i]) ^ ((CRC) & 0x000000FF)];
    _ui64toa(CRC, t, radix);
    Dots(t, radix);
}

void ChecksumDlg::SHA(int begin, int end, char* t)
{
  char temp[STD_BUF];
  CSHA sha;
  sha.Reset();
  sha.AddData((char*)pamiec+begin, end-begin+1);
  // 20 bajtow    3601ce27d60d49dcbcdddf3035b8e963344bd4aa
  sha.FinalDigest(temp);
  BytesToHex(t, (unsigned char*)temp, 20);
}

void ChecksumDlg::MD5(int begin, int end, char* t)
{
  char temp[STD_BUF];
  CMD5 md5;
  md5.Reset();
  md5.AddData((char*)pamiec+begin, end-begin+1);
  md5.FinalDigest(temp);
  BytesToHex(t, (unsigned char*)temp, 16);
}

void ChecksumDlg::RIPEMD(int begin, int end, char* t)
{
  char temp[STD_BUF];
  CRIPEMD ripemd;
  ripemd.Reset();
  ripemd.AddData((char*)pamiec+begin, end-begin+1);
  ripemd.FinalDigest(temp);
  BytesToHex(t, (unsigned char*)temp, 16);
}

ChecksumDlg::ChecksumDlg()
{
    //InitCRC32();

    hDlg = CreateDialog(hInstance, "UVIEW", h_client_wnd, ChecksumDlgProc);
    SetWindowText(hDlg, "Checksums");
    InitTool();
    Display();
}
void ChecksumDlg::Display()
{
    char tekst[200];
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, FALSE, 0);
    SendMessage(GetDlgItem(hDlg, 1001), LB_RESETCONTENT, 0, 0);

    strcpy(tekst, "Checksum 8 = ");
    Checksum8(selected_begin, selected_end, tekst + strlen(tekst));
    SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    strcpy(tekst, "Checksum 16 = ");
    Checksum16(selected_begin, selected_end, tekst + strlen(tekst));
    SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    strcpy(tekst, "Checksum 32 = ");
    Checksum32(selected_begin, selected_end, tekst + strlen(tekst));
    SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    strcpy(tekst, "Checksum 64 = ");
    Checksum64(selected_begin, selected_end, tekst + strlen(tekst));
    SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    strcpy(tekst, "CRC16 = ");
    CRC16(selected_begin, selected_end, tekst + strlen(tekst));
    SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    strcpy(tekst, "CRC16CCITT = ");
    CRC16CCITT(selected_begin, selected_end, tekst + strlen(tekst));
    SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    strcpy(tekst, "CRC32 = ");
    CRC32(selected_begin, selected_end, tekst + strlen(tekst));
    SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    strcpy(tekst, "SHA ");
    SHA(selected_begin, selected_end, tekst + strlen(tekst));
    SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    strcpy(tekst, "MD5 ");
    MD5(selected_begin, selected_end, tekst + strlen(tekst));
    SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    strcpy(tekst, "RIPEMD ");
    RIPEMD(selected_begin, selected_end, tekst + strlen(tekst));
    SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);

    SendMessage(GetDlgItem(hDlg, 1001), LB_SETTOPINDEX, top_index, 0);
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, TRUE, 0);
}
void ChecksumDlg::CopyToClipboard(int c)
{
    if(MessageBox(h_main_wnd, "Copy to clipboard?", szAppName, MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        HGLOBAL hGlobal;
        unsigned char* pGlobal;
        char tekst[200];
        switch(c)
        {
            case 0:  Checksum8(selected_begin, selected_end, tekst);  break;
            case 1:  Checksum16(selected_begin, selected_end, tekst);  break;
            case 2:  Checksum32(selected_begin, selected_end, tekst);  break;
            case 3:  Checksum64(selected_begin, selected_end, tekst);  break;
            case 4:  CRC16(selected_begin, selected_end, tekst);  break;
            case 5:  CRC16CCITT(selected_begin, selected_end, tekst);  break;
            case 6:  CRC32(selected_begin, selected_end, tekst);  break;
            case 7:  SHA(selected_begin, selected_end, tekst);  break;
        }
        hGlobal = GlobalAlloc(GHND | GMEM_SHARE, strlen(tekst) + 1);
        pGlobal = (unsigned char*)GlobalLock(hGlobal);
        CopyMemory(pGlobal, tekst, strlen(tekst) + 1);
        GlobalUnlock(hGlobal);
        OpenClipboard(hDlg);
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hGlobal);
        CloseClipboard();
        delete [] schowek;
        schowek = new unsigned char[strlen(tekst)];
        CopyMemory(schowek, tekst, strlen(tekst));
        schoweklen = strlen(tekst);
        MultiClipboard::Copy(schowek, schoweklen);
    }
}

void ChecksumDlg::InitCRC32()
{
    unsigned int dwCrc;
    for(int i = 0; i < 256; i++)
    {
        dwCrc = i;
        for(int j = 8; j > 0; j--)
        {
            if(dwCrc & 1)
                dwCrc = (dwCrc >> 1) ^ 0xedb88320;
            else
                dwCrc >>= 1;
        }
        crc32table[i] = dwCrc;
    }
}

unsigned int ChecksumDlg::UniversalCRC32(unsigned char*data, int len)
{
    unsigned int CRC = 0xffffffff;
    for(unsigned char*i = data; i < data+len; i++)
        CRC = ((CRC) >> 8) ^ crc32table[(*i) ^ ((CRC) & 0x000000FF)];
    return CRC;
}

// class OccurenceDlg

BOOL CALLBACK OccurenceDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    char tekst[200];
    switch(message)
    {
        case WM_INITDIALOG:
            SetWindowText(hDlg, "Byte occurence");
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 1001:
                    if(HIWORD(wParam) == LBN_KILLFOCUS)
                    {
                        ((OccurenceDlg*)ChildDlg::GetThis(hDlg))->SetTopIndex(SendMessage(GetDlgItem(hDlg, 1001), LB_GETTOPINDEX, 0, 0));
                    }
                    else if(HIWORD(wParam) == LBN_DBLCLK)
                        ((OccurenceDlg*)ChildDlg::GetThis(hDlg))->SetHighlight();
                    break;
            }
            return TRUE;
        case WM_CLOSE:
            ChildDlg::free(hDlg);
            return FALSE;
    }
    return FALSE;
}

OccurenceDlg::OccurenceDlg()
{
    hDlg = CreateDialog(hInstance, "UVIEW", h_client_wnd, OccurenceDlgProc);
    InitTool();
    Display();
}
void OccurenceDlg::Display()
{
    char t[200];
    int cslen;
    unsigned int occurences[256];

    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, FALSE, 0);
    SendMessage(GetDlgItem(hDlg, 1001), LB_RESETCONTENT, 0, 0);


    for(int i = 0; i < 256; i++)
        occurences[i] = 0;

    for(int i = selected_begin; i <= selected_end; i++)
        occurences[pamiec[i]]++;

    for(int i = 0; i < 256; i++)
    {
        _ui64toa(i, t, 10);
        strcat(t, ":");

        for(int k = 20 - strlen(t); k >= 0; k--)
            strcat(t, " ");

        t[strlen(t) + 1] = 0;
        t[strlen(t)] = (char)i;
        for(int k = 40 - strlen(t); k >= 0; k--)
            strcat(t, " ");

        _ui64toa(occurences[i], t + strlen(t), 10);
        strcat(t, " (");
        _gcvt((double)occurences[i]*100/((selected_end-selected_begin+1)?(selected_end-selected_begin+1):1), 5, t+strlen(t));

        strcat(t, "%) ");
        SendMessage(GetDlgItem(hDlg, 1001), LB_INSERTSTRING, i, (LPARAM)t);
    }

    SendMessage(GetDlgItem(hDlg, 1001), LB_SETTOPINDEX, top_index, 0);
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, TRUE, 0);
}
void OccurenceDlg::SetHighlight()
{
    if(MessageBox(h_main_wnd, "Highlight selected byte?", szAppName, MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        highlight[0] = SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0);
        highlight_size = 1;
        InvalidateRect(h_client_wnd, NULL, 0);
    }
}

// class NavigatorDlg

BOOL CALLBACK NavigatorDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    char tekst[200];
    static int top_index;
    switch(message)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 1001:
                    if(HIWORD(wParam) == LBN_KILLFOCUS)
                    {
                        ((NavigatorDlg*)ChildDlg::GetThis(hDlg))->SetTopIndex(SendMessage(GetDlgItem(hDlg, 1001), LB_GETTOPINDEX, 0, 0));
                        //highlight_struct_size = 0;
                    }
                    else if(HIWORD(wParam) == LBN_DBLCLK)
                    {
                        //position[SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0)]->GoTo();
                        ((NavigatorDlg*)ChildDlg::GetThis(hDlg))->SetTopIndex(SendMessage(GetDlgItem(hDlg, 1001), LB_GETTOPINDEX, 0, 0));
                        ((NavigatorDlg*)ChildDlg::GetThis(hDlg))->GoTo(
                            SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0)
                            );
                    }
                    break;
                case 1005:
                    POINT point;
                    RECT rect;
                    GetClientRect(hDlg, &rect);
                    point.x = rect.right;
                    point.y = 0;
                    ClientToScreen(hDlg, &point);
                    TrackPopupMenu(hPopupRadixMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hDlg, NULL);
                    break;
                case 1006:
                    //SendMessage(h_client_wnd, WM_COMMAND, 131, 0);
                    ((NavigatorDlg*)ChildDlg::GetThis(hDlg))->Reset(
                        SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0)
                        );
                    break;
                case 200:
                    ((NavigatorDlg*)ChildDlg::GetThis(hDlg))->SetRadix(2);
                    break;
                case 201:
                    ((NavigatorDlg*)ChildDlg::GetThis(hDlg))->SetRadix(8);
                    break;
                case 202:
                    ((NavigatorDlg*)ChildDlg::GetThis(hDlg))->SetRadix(10);
                    break;
                case 203:
                    ((NavigatorDlg*)ChildDlg::GetThis(hDlg))->SetRadix(16);
                    break;
            }
            return TRUE;
        case WM_CLOSE:
            ChildDlg::free(hDlg);
            return FALSE;
    }
    return FALSE;
}

NavigatorDlg::NavigatorDlg(cPosition p[], int n)
{
    pos_num = n;
    for(int i = 0; i < pos_num; i++)
        pos[i] = &p[i];

    hDlg = CreateDialog(hInstance, "NAVIGATORVIEW", h_client_wnd, NavigatorDlgProc);

    SendMessage(GetDlgItem(hDlg, 1005), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(533)));
    SendMessage(GetDlgItem(hDlg, 1006), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(530)));
    InitTool();
    Display();
    TOOLINFO ti;
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
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = h_tool_bar;
    ti.hinst = hInstance;
    ti.uId = (unsigned int)GetDlgItem(hDlg, 1005);
    ti.lpszText = "Radix";
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
    ti.uId = (unsigned int)GetDlgItem(hDlg, 1006);
    ti.lpszText = "Reset";
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
}
NavigatorDlg::~NavigatorDlg()
{
    DestroyWindow(h_tool_tip);
}

void NavigatorDlg::Display()
{
    char tekst[256];
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, FALSE, 0);
    SendMessage(GetDlgItem(hDlg, 1001), LB_RESETCONTENT, 0, 0);

    for(int i = 0; i < pos_num; i++)
    {
        pos[i]->PrintForNavigator(tekst, radix);
        SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    }
    SendMessage(GetDlgItem(hDlg, 1001), LB_SETTOPINDEX, top_index, 0);
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, TRUE, 0);
}

void NavigatorDlg::GoTo(int num)
{
    pos[num]->GoTo();
}

void NavigatorDlg::Reset(int num)
{
    if(num != LB_ERR)
    {
        pos[num]->Reset();
        InvalidateRect(h_client_wnd, NULL, 0);
        Display();
    }
}

// class NavigatorFindDlg

NavigatorFindDlg::NavigatorFindDlg(cPosition p[], int n, char*title) : NavigatorDlg(p, n)
{
    for(int i = 0; i < pos_num; i++)
       pos[i] = new cPosition(p[i]);

    char t[200];
    strcpy(t, "Navigator ");
    int offs = strlen(t);
    strcat(t, title);
    Dots(t+offs, 16);
    SetWindowText(hDlg, t);
}

NavigatorFindDlg::~NavigatorFindDlg()
{
    for(int i = 0; i < pos_num; i++)
        delete pos[i];
    DestroyWindow(h_tool_tip);
}


// class PixelDlg

BOOL CALLBACK PixelDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 1000:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                        ((PixelDlg*)ChildDlg::GetThis(hDlg))->SetBpp(SendMessage(GetDlgItem(hDlg, 1000), CB_GETCURSEL, 0, 0));
                    break;
                case 1002:
                    ((PixelDlg*)ChildDlg::GetThis(hDlg))->SetColumns();
                    break;
                case 1007:
                    POINT point;
                    point.x = 0;
                    point.y = 0;
                    ClientToScreen(hDlg, &point);
                    TrackPopupMenu(hPopupBPPMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hDlg, NULL);
                    break;
                case 200:
                    ((PixelDlg*)ChildDlg::GetThis(hDlg))->SetBpp(1);
                    break;
                case 205:
                    ((PixelDlg*)ChildDlg::GetThis(hDlg))->SetBpp(4);
                    break;
                case 201:
                    ((PixelDlg*)ChildDlg::GetThis(hDlg))->SetBpp(8);
                    break;
                case 202:
                    ((PixelDlg*)ChildDlg::GetThis(hDlg))->SetBpp(16);
                    break;
                case 203:
                    ((PixelDlg*)ChildDlg::GetThis(hDlg))->SetBpp(24);
                    break;
                case 204:
                    ((PixelDlg*)ChildDlg::GetThis(hDlg))->SetBpp(32);
            }
            return TRUE;
        case WM_CLOSE:
            ChildDlg::free(hDlg);
            return FALSE;
    }
    return FALSE;
}

PixelDlg::PixelDlg(int col) //  = 250
{
    char tekst[200];
    column_num = col;
    bpp = 24;
    newest_thread = 0;
    hDlg = CreateDialog(hInstance, "PIXELVIEW", h_client_wnd, PixelDlgProc);
    SendMessage(GetDlgItem(hDlg, 1007), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(500)));
    itoa(column_num, tekst, 10);
    SetWindowText(GetDlgItem(hDlg, 1003), tekst);
    TOOLINFO ti;
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
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = h_tool_bar;
    ti.hinst = hInstance;
    char* tips[] = {"Size of view", "Bits per pixel", "Set this column number", "Enter new column number"};
    for(int i = 0; i < 5; i++)
    {
        ti.uId = (unsigned int)GetDlgItem(hDlg, 1000 + i);
        ti.lpszText = tips[i];
        SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
    }

    InitTool();
    Display();
}
void PixelDlg::Display()
{
    newest_thread = _beginthread(DrawPixels, 0, this);
}
void PixelDlg::SetBpp(int b)
{
    bpp = b;
    Display();
}
void PixelDlg::SetColumns()
{
    char tekst[200];
    GetWindowText(GetDlgItem(hDlg, 1003), tekst, 200);
    column_num = atoi(tekst);
    Display();
}

void DrawPixels(void* pParam)
{
    thread_num++;
    unsigned long me = ((PixelDlg*)pParam)->newest_thread;
    const int Y_OFFSET = 30;
    RECT rect;
    HDC ekran, hdc;
    HBITMAP bmp;
    char tekst[50];
    unsigned char *p, *origin;
    p = origin = pamiec + selected_begin;
    GetClientRect(((PixelDlg*)pParam)->hDlg, &rect);

    ekran = CreateDC("DISPLAY", 0, 0, 0);
    hdc = CreateCompatibleDC(ekran);
    bmp = CreateCompatibleBitmap(ekran, rect.right, rect.bottom);
    SelectObject(hdc, bmp);
    DeleteDC(ekran);

    SelectObject(hdc, CreateSolidBrush(sys_COLOR_3DFACE));
    PatBlt(hdc, 0, Y_OFFSET, rect.right, rect.bottom, PATCOPY);
    switch(((PixelDlg*)pParam)->bpp)
    {
        case 1:
            unsigned char mask;
            mask = 1;
            for(int y = Y_OFFSET; y < rect.bottom; y++)
                for(int x = 0; x < ((PixelDlg*)pParam)->column_num; x++)
                {
                    if(p < pamiec + dlugosc_pliku)
                    {
                        SetPixel(hdc, x, y, (*p)&mask?0xffffff:0);
                        if(mask==0x80)
                        {
                            mask = 1;
                            p++;
                        }
                        else
                            mask <<= 1;
                    }
                    if(me!=((PixelDlg*)pParam)->newest_thread)
                        goto leave;
                }
            break;

          /******
          Thanks Klaus :)
          ******/
          case 4:
          {
            bool lo_nibble = false;
            for(int y = Y_OFFSET; y < rect.bottom; y++)
                for(int x = 0; x < ((PixelDlg*)pParam)->column_num; x++)
                {
                    if(p < pamiec + dlugosc_pliku)
                    {
                      DWORD color = 0;
                        if(lo_nibble)
                        {
                           color = (*p) & 0x0F;
                           lo_nibble = false;
                           p++;
                        }
                        else
                        {
                           color = (*p) >> 4;
                           lo_nibble = true;
                        }

                        // windows standard palette
                        static COLORREF g_crColor[] = { 0x00000000,0x80000000,0x00800000,0x80800000,
                                                        0x00008000,0x80008000,0x00808000,0xC0C0C000,
                                                        0x80808000,0xFF000000,0x00FF0000,0xFFFF0000,
                                                        0x0000FF00,0xFF00FF00,0x00FFFF00,0xFFFFFF00 };

                        SetPixel(hdc, x, y, g_crColor[color & 0x0F]);
                    }

                    if(me!=((PixelDlg*)pParam)->newest_thread)
                        goto leave;
                }
            }
            break;

        case 8:
            for(int y = Y_OFFSET; y < rect.bottom; y++)
                for(int x = 0; x < ((PixelDlg*)pParam)->column_num; x++)
                {
                    if(p < pamiec + dlugosc_pliku)
                    {
                        SetPixel(hdc, x, y, (int)(*p)<<8);
                        p++;
                    }
                    if(me!=((PixelDlg*)pParam)->newest_thread)
                        goto leave;
                }
            break;
        case 16:
            for(int y = Y_OFFSET; y < rect.bottom; y++)
                for(int x = 0; x < ((PixelDlg*)pParam)->column_num; x++)
                {
                    if(p < pamiec + dlugosc_pliku)
                    {
                        unsigned int word;
                        unsigned int color;
                        word = *(unsigned short*)p;
                        color = (word&0x1f)<<3;
                        color |= (word&0x7e0)<<5;
                        color |= (word&0xf800)<<8;
                        SetPixel(hdc, x, y, color);
                        p+=2;
                    }
                    if(me!=((PixelDlg*)pParam)->newest_thread)
                        goto leave;
                }
            break;
        case 24:
            for(int y = Y_OFFSET; y < rect.bottom; y++)
                for(int x = 0; x < ((PixelDlg*)pParam)->column_num; x++)
                {
                    if(p < pamiec + dlugosc_pliku)
                    {
                        SetPixel(hdc, x, y, *(int*)p);
                        p+=3;
                    }
                    if(me!=((PixelDlg*)pParam)->newest_thread)
                        goto leave;
                }
            break;
        case 32:
            for(int y = Y_OFFSET; y < rect.bottom; y++)
                for(int x = 0; x < ((PixelDlg*)pParam)->column_num; x++)
                {
                    if(p < pamiec + dlugosc_pliku)
                    {
                        SetPixel(hdc, x, y, *(int*)p);
                        p+=4;
                    }
                    if(me!=((PixelDlg*)pParam)->newest_thread)
                        goto leave;
                }
    }

    ekran = GetDC(((PixelDlg*)pParam)->hDlg);
    BitBlt(ekran, 0, Y_OFFSET, rect.right, rect.bottom - Y_OFFSET, hdc, 0, Y_OFFSET, SRCCOPY);
    ReleaseDC(((PixelDlg*)pParam)->hDlg, ekran);

    itoa((int)(p - origin), tekst, 10);
    strcat(tekst, " bytes");
    SetWindowText(GetDlgItem(((PixelDlg*)pParam)->hDlg, 1000), tekst);

    leave:
    DeleteObject(SelectObject(hdc, GetStockObject(BLACK_BRUSH)));
    DeleteDC(hdc);
    DeleteObject(bmp);
    thread_num--;
    _endthread();
}

// class PatternsDlg

BOOL CALLBACK PatternsDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 1001:
                    if(HIWORD(wParam) == LBN_KILLFOCUS)
                        ((PatternsDlg*)ChildDlg::GetThis(hDlg))->SetTopIndex(SendMessage(GetDlgItem(hDlg, 1001), LB_GETTOPINDEX, 0, 0));
                    break;
                case 1002:
                    ((PatternsDlg*)ChildDlg::GetThis(hDlg))->Display();
                    break;
                case 1008:
                    //SetCursor(LoadCursor(NULL, IDC_WAIT));
                    SendMessage(h_client_wnd, WM_COMMAND, 136, 0);
                    break;
            }
            return TRUE;
        case WM_CLOSE:
            ChildDlg::free(hDlg);
            return FALSE;
    }
    return FALSE;
}

PatternsDlg::PatternsDlg()
{
    char tekst[200];
    newest_thread = 0;
    hDlg = CreateDialog(hInstance, "PATTERNVIEW", h_client_wnd, PatternsDlgProc);
    SendMessage(GetDlgItem(hDlg, 1008), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(534)));

    SetWindowText(GetDlgItem(hDlg, 1000), "5");
    SetWindowText(GetDlgItem(hDlg, 1003), "499");
    MIN_COL = 5;
    MAX_COL = 501;

    TOOLINFO ti;
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
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = h_tool_bar;
    ti.hinst = hInstance;
    char* tips[] = {"Select all", "Enter minimum pattern length", "Enter maximum pattern length", "Set these lengths"};

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1008);
    ti.lpszText = tips[0];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1000);
    ti.lpszText = tips[1];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1003);
    ti.lpszText = tips[2];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1002);
    ti.lpszText = tips[3];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    InitTool();
    Display();
}

void PatternsDlg::Display()
{
    ReadColumns();
    newest_thread = _beginthread(CalculatePatterns, 0, this);
}

void PatternsDlg::ReadColumns()
{
    char t[100];
    GetWindowText(GetDlgItem(hDlg, 1000), t, 100);
    MIN_COL = atoi(t);
    if(MIN_COL < 2)
        MIN_COL = 2;
    else if(MIN_COL > 5000)
        MIN_COL = 5000;

    GetWindowText(GetDlgItem(hDlg, 1003), t, 100);
    MAX_COL = atoi(t);
    if(MAX_COL < 3)
        MAX_COL = 3;
    else if(MAX_COL > 6000)
        MAX_COL = 6000;

    if(MIN_COL>=MAX_COL)
        MIN_COL=MAX_COL-1;

}

void CalculatePatterns(void*pParam)
{
    thread_num++;
    char t[200];
    unsigned long me = ((PatternsDlg*)pParam)->newest_thread;
    double entropy[6500];
    int min;

    SendMessage(GetDlgItem(((PatternsDlg*)pParam)->hDlg, 1001), LB_RESETCONTENT, 0, 0);
    SendMessage(GetDlgItem(((PatternsDlg*)pParam)->hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)"Analysing...");
    SendMessage(GetDlgItem(((PatternsDlg*)pParam)->hDlg, 1001), WM_SETREDRAW, FALSE, 0);
    SendMessage(GetDlgItem(((PatternsDlg*)pParam)->hDlg, 1001), LB_RESETCONTENT, 0, 0);


    for(int i = ((PatternsDlg*)pParam)->MIN_COL; i < ((PatternsDlg*)pParam)->MAX_COL; i++)
        entropy[i]=0;


    for(int col = ((PatternsDlg*)pParam)->MIN_COL; col<((PatternsDlg*)pParam)->MAX_COL; col++)
    {
        int diff;
        for(int i = selected_begin; i <= selected_end - col; i++)
        {
            diff=pamiec[i]-pamiec[i+col];
            if(diff<0)
                diff=-diff;
            entropy[col] += diff;
            if(me!=((PatternsDlg*)pParam)->newest_thread)
                goto leave;
        }
    }

    for(int k = 0; k < 20; k++)
    {
        min = ((PatternsDlg*)pParam)->MIN_COL;
        for(int i = ((PatternsDlg*)pParam)->MIN_COL; i < ((PatternsDlg*)pParam)->MAX_COL; i++)
            if(entropy[i]<entropy[min])
                min = i;
        itoa(min, t, 10);
        strcat(t, " bytes               (entropy = ");
        _gcvt(entropy[min], 8, t + strlen(t));
        strcat(t, ")");
        SendMessage(GetDlgItem(((PatternsDlg*)pParam)->hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)t);
        entropy[min] = ((PatternsDlg*)pParam)->MAX_DOUBLE;
    }
    leave:
    SendMessage(GetDlgItem(((PatternsDlg*)pParam)->hDlg, 1001), WM_SETREDRAW, TRUE, 0);
    thread_num--;
    _endthread();
}

// class fourier

BOOL CALLBACK FourierDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
            for(int i=0; i < 10; i++)
              SendMessage(GetDlgItem(hDlg, 1003), CB_ADDSTRING, 0, (LPARAM)simple_data_names[i]);
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 1001:
                    if(HIWORD(wParam) == LBN_KILLFOCUS)
                        ((FourierDlg*)ChildDlg::GetThis(hDlg))->SetTopIndex(SendMessage(GetDlgItem(hDlg, 1001), LB_GETTOPINDEX, 0, 0));
                    break;
                case 1003:
                  if(HIWORD(wParam)!=CBN_SELCHANGE) break;
                case 1002:
                    ((FourierDlg*)ChildDlg::GetThis(hDlg))->Display();
                    break;
                case 1008:
                    SendMessage(h_client_wnd, WM_COMMAND, 136, 0);
                    break;
                case 1009:
                  ((FourierDlg*)ChildDlg::GetThis(hDlg))->Save();
                  break;
            }
            return TRUE;
        case WM_CLOSE:
            ChildDlg::free(hDlg);
            return FALSE;
    }
    return FALSE;
}

FourierDlg::FourierDlg()
{
    char tekst[200];
    newest_thread=0;
    *file=0;
    hDlg = CreateDialog(hInstance, "FOURIERVIEW", h_client_wnd, FourierDlgProc);
    SendMessage(GetDlgItem(hDlg, 1003), CB_SETCURSEL, 0, 0);
    SendMessage(GetDlgItem(hDlg, 1008), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(534)));
    SendMessage(GetDlgItem(hDlg, 1009), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(535)));

    TOOLINFO ti;
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
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = h_tool_bar;
    ti.hinst = hInstance;
    char* tips[] = {"Select all", "Enter minimal trigger value to add to the list", "Select data type", "Set these parameters", "Save transform in file"};

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1008);
    ti.lpszText = tips[0];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1000);
    ti.lpszText = tips[1];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1003);
    ti.lpszText = tips[2];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1002);
    ti.lpszText = tips[3];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    ti.uId = (unsigned int)GetDlgItem(hDlg, 1009);
    ti.lpszText = tips[4];
    SendMessage(h_tool_tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    InitTool();
    Display();
}

void FourierDlg::Display()
{
  newest_thread=_beginthread(CalculateFourier, 0, this);
}

void FourierDlg::Save()
{
  OPENFILENAME ofnf;
  ofnf.lStructSize = sizeof(OPENFILENAME);
  ofnf.hwndOwner = hDlg;
  ofnf.hInstance = NULL;
  ofnf.lpstrFilter = "Text Files (*.txt)\0*.txt\0\0";
  ofnf.lpstrCustomFilter = NULL;
  ofnf.nMaxCustFilter = 0;
  ofnf.nFilterIndex = 1;
  ofnf.lpstrFile = file;
  ofnf.nMaxFile = MAX_PATH;
  ofnf.lpstrFileTitle = NULL;
  ofnf.nMaxFileTitle = MAX_PATH;
  ofnf.lpstrInitialDir = NULL;
  ofnf.lpstrTitle = NULL;
  ofnf.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
  ofnf.lpstrDefExt = "txt";
  ofnf.lCustData = 0L;
  ofnf.lpfnHook = NULL;
  ofnf.lpTemplateName = NULL;
  if(GetSaveFileName(&ofnf))
    newest_thread=_beginthread(CalculateFourier, 0, this);
}

void CalculateFourier(void*pParam)
{
  thread_num++;
  char txt[STD_BUF];
  unsigned long me = ((FourierDlg*)pParam)->newest_thread;
  SendMessage(GetDlgItem(((FourierDlg*)pParam)->hDlg, 1001), LB_RESETCONTENT, 0, 0);
  SendMessage(GetDlgItem(((FourierDlg*)pParam)->hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)"Analysing...");
  SendMessage(GetDlgItem(((FourierDlg*)pParam)->hDlg, 1001), WM_SETREDRAW, FALSE, 0);
  int data_mode=SendMessage(GetDlgItem(((FourierDlg*)pParam)->hDlg, 1003), CB_GETCURSEL, 0, 0);
  int order=0, points=(selected_end-selected_begin+1);
  switch(data_mode)
  {
    case 0: case 1:
      points/=sizeof(char); break;
    case 2: case 3:
      points/=sizeof(short int); break;
    case 4: case 5: case 8:
      points/=sizeof(int); break;
    case 6: case 7: case 9:
      points/=sizeof(__int64); break;
  }

  while(1u<<order<points)
    order++;

  int size=1u<<order;
  double*x,*y;
  try
  {
    x=new double[size];
    ZeroMemory(x,sizeof(double)*size);
    y=new double[size];
    ZeroMemory(y,sizeof(double)*size);
  }
  catch(...)
  {
    SendMessage(GetDlgItem(((FourierDlg*)pParam)->hDlg, 1001), WM_SETREDRAW, TRUE, 0);
    SendMessage(GetDlgItem(((FourierDlg*)pParam)->hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)"Not enough memory");
    thread_num--;
    _endthread();
  }

  switch(data_mode)
  {
    case 0: for(int i=0;i<points;i++) x[i]=*((unsigned char*)(pamiec+selected_begin)+i); break;
    case 1: for(int i=0;i<points;i++) x[i]=*((char*)(pamiec+selected_begin)+i); break;
    case 2: for(int i=0;i<points;i++) x[i]=*((unsigned short int*)(pamiec+selected_begin)+i); break;
    case 3: for(int i=0;i<points;i++) x[i]=*((short int*)(pamiec+selected_begin)+i); break;
    case 4: for(int i=0;i<points;i++) x[i]=*((unsigned int*)(pamiec+selected_begin)+i); break;
    case 5: for(int i=0;i<points;i++) x[i]=*((int*)(pamiec+selected_begin)+i); break;
    case 6: for(int i=0;i<points;i++) x[i]=*((unsigned __int64*)(pamiec+selected_begin)+i); break;
    case 7: for(int i=0;i<points;i++) x[i]=*((__int64*)(pamiec+selected_begin)+i); break;
    case 8: for(int i=0;i<points;i++) x[i]=*((float*)(pamiec+selected_begin)+i); break;
    case 9: for(int i=0;i<points;i++) x[i]=*((double*)(pamiec+selected_begin)+i); break;
  }


/*
 unsigned int n,l,e,f,i,j,o,o1,j1,i1,k;
 double u,v,z,c,s,p,q,r,t,w,a;
 n=1u<<order;
 for(l=1;l<=order;l++)
 {
   u=1.0;
   v=0.0;
   e=1u<<(order-l+1);
   f=e/2;

   z=M_PI/f;

   c=cos(z);
   s=-sin(z);

   //if(param==FFT)
   //s=-s;

   for(j=1;j<=f;j++)
   {
     for(i=j;i<=n;i+=e)
     {
       o=i+f-1;
       o1=i-1;
       p=x[o1]+x[o];
       r=x[o1]-x[o];
       q=y[o1]+y[o];
       t=y[o1]-y[o];
       x[o]=r*u-t*v;
       y[o]=t*u+r*v;
       x[o1]=p;
       y[o1]=q;
       if(me!=((FourierDlg*)pParam)->newest_thread)
         goto leave;
     }
     w=u*c-v*s;
     v=v*c+u*s;
     u=w;
   }
 }
 j=1;
 for(i=1;i<n;i++)
 {
   if(i<j)
   {
     j1=j-1;
     i1=i-1;
     p=x[j1];
     q=y[j1];
     x[j1]=x[i1];
     y[j1]=y[i1];
     x[i1]=p;
     y[i1]=q;
   }
   k=n/2;
   while(k<j)
   {
     j=j-k;
     k=k/2;
   }
   j+=k;
 }
*/
/////////////

/*
   This computes an in-place complex-to-complex FFT
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform
*/
short int dir=1;
long m=order;

   long n,i,i1,j,k,i2,l,l1,l2;
   double c1,c2,tx,ty,t1,t2,u1,u2,z;

   /* Calculate the number of points */
   n = 1;
   for (i=0;i<m;i++)
      n *= 2;

   /* Do the bit reversal */
   i2 = n >> 1;
   j = 0;
   for (i=0;i<n-1;i++) {
      if (i < j) {
         tx = x[i];
         ty = y[i];
         x[i] = x[j];
         y[i] = y[j];
         x[j] = tx;
         y[j] = ty;
      }
      k = i2;
      while (k <= j) {
         j -= k;
         k >>= 1;
      }
      j += k;
   }

   /* Compute the FFT */
   c1 = -1.0;
   c2 = 0.0;
   l2 = 1;
   for (l=0;l<m;l++) {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0;
      u2 = 0.0;
      for (j=0;j<l1;j++) {
         for (i=j;i<n;i+=l2) {
            i1 = i + l1;
            t1 = u1 * x[i1] - u2 * y[i1];
            t2 = u1 * y[i1] + u2 * x[i1];
            x[i1] = x[i] - t1;
            y[i1] = y[i] - t2;
            x[i] += t1;
            y[i] += t2;
            if(me!=((FourierDlg*)pParam)->newest_thread)
              goto leave;
         }
         z =  u1 * c1 - u2 * c2;
         u2 = u1 * c2 + u2 * c1;
         u1 = z;
      }
      c2 = sqrt((1.0 - c1) / 2.0);
      if (dir == 1)
         c2 = -c2;
      c1 = sqrt((1.0 + c1) / 2.0);
   }

   /* Scaling for forward transform */
   if (dir == 1) {
      for (i=0;i<n;i++) {
         x[i] /= n;
         y[i] /= n;
      }
   }





  for(int i=0;i<points;i++)
    x[i]=sqrt(x[i]*x[i]+y[i]*y[i]);


  SendMessage(GetDlgItem(((FourierDlg*)pParam)->hDlg, 1001), LB_RESETCONTENT, 0, 0);

  int max_index;
  max_index=1;  // 0 is special; see FT def
  for(int i=0;i<50&&i<points/2;i++)
  {
    for(int p=1;p<points/2-1;p++)
      if(x[p]>x[max_index])
        max_index=p;

    _gcvt(x[max_index], 8, txt);
    strcat(txt, " every ");
    itoa((int)((double)points/max_index), txt+strlen(txt), 10);
    strcat(txt, " ");
    strcat(txt, simple_data_names[data_mode]);
    strcat(txt, "(s)");
    SendMessage(GetDlgItem(((FourierDlg*)pParam)->hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)txt);
    x[max_index]=0;
  }

  if(*((FourierDlg*)pParam)->file)
  {
    HANDLE plik;
    DWORD bw;
    char tmp_txt[STD_BUF];

    plik = CreateFile(((FourierDlg*)pParam)->file, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    for(int i=1;i<points;i++)
    {
      _gcvt(x[i], 10, tmp_txt);
      strcat(tmp_txt, "\r\n");
      WriteFile(plik, tmp_txt, strlen(tmp_txt), &bw, NULL);
    }
    CloseHandle(plik);
    *((FourierDlg*)pParam)->file=0;   // means - i wrote, file closed
  }


  leave:
  delete[]x;
  delete[]y;
  SendMessage(GetDlgItem(((FourierDlg*)pParam)->hDlg, 1001), WM_SETREDRAW, TRUE, 0);
  thread_num--;
  _endthread();
}

// class disassembler

BOOL CALLBACK DisassemblerDlgProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    char tekst[200];
    switch(message)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 1001:
                    if(HIWORD(wParam) == LBN_KILLFOCUS)
                    {
                        ((DisassemblerDlg*)ChildDlg::GetThis(hDlg))->SetTopIndex(SendMessage(GetDlgItem(hDlg, 1001), LB_GETTOPINDEX, 0, 0));
                        highlight_struct_size = 0;
                    }
                    else if(HIWORD(wParam) == LBN_SELCHANGE)
                        ((DisassemblerDlg*)ChildDlg::GetThis(hDlg))->SetHighlight();
                    else if(HIWORD(wParam) == LBN_DBLCLK)
                        ((DisassemblerDlg*)ChildDlg::GetThis(hDlg))->Jump();
                    break;
            }
            return TRUE;
        case WM_CLOSE:
            ChildDlg::free(hDlg);
            return FALSE;
    }
    return FALSE;
}

DisassemblerDlg::DisassemblerDlg()
{
    hDlg = CreateDialog(hInstance, "UVIEW", h_client_wnd, DisassemblerDlgProc);
    SetWindowText(hDlg, "Disassembler");
    InitTool();
    Display();
}

void DisassemblerDlg::Display()
{
    char tekst[STD_BUF];
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, FALSE, 0);
    SendMessage(GetDlgItem(hDlg, 1001), LB_RESETCONTENT, 0, 0);
    int komorka_disasm=selected_begin;
    for(int i = 0; i < INST_NUM; i++)
    {

        inst_len[i]=sprint_address(tekst, STD_BUF, (char*)(pamiec+komorka_disasm));
        komorka_disasm+=inst_len[i];

        int len=strlen(tekst),j=0;
        while(tekst[j]!=9)j++;
        tekst[j]=' ';

        for(int k=i;k<15-i;k++)
          MoveMemory(tekst+k+1,tekst+k,len-j+1);

        SendMessage(GetDlgItem(hDlg, 1001), LB_ADDSTRING, 0, (LPARAM)tekst);
    }
    SendMessage(GetDlgItem(hDlg, 1001), LB_SETTOPINDEX, top_index, 0);
    SendMessage(GetDlgItem(hDlg, 1001), WM_SETREDRAW, TRUE, 0);
}

void DisassemblerDlg::Jump()
{
    int inst_num=SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0);
    int inst_offset=selected_begin;
    for(int i=0; i<inst_num;i++)
      inst_offset+=inst_len[i];
    selected_begin=selected_end=inst_offset;
    InvalidateRect(h_client_wnd, NULL, 0);
    top_index=0;
    Display();
}
void DisassemblerDlg::SetHighlight()
{
    int inst_num=SendMessage(GetDlgItem(hDlg, 1001), LB_GETCURSEL, 0, 0);
    int inst_offset=0;
    for(int i=0; i<inst_num;i++)
      inst_offset+=inst_len[i];

    highlight_struct = selected_begin+inst_offset;
    highlight_struct_size = inst_len[inst_num];
    InvalidateRect(h_client_wnd, NULL, 0);
}

