#include"macro.h"

extern HWND h_main_wnd, h_client_wnd;

macro* macro::mac[100];
macro* macro::recording = 0;
int macro::num = 0;

macro::macro(char* n)
{
    nazwa = new char[MAX_PATH];
    strcpy(nazwa, n);
    action_num = 0;
    recording = this;
    MENUITEMINFO mio;
    mio.cbSize = sizeof(MENUITEMINFO);
    mio.fMask = MIIM_TYPE | MIIM_ID;
    mio.wID = 400 + num;
    mio.fType = MFT_STRING;
    mio.dwTypeData = nazwa;
    mio.cch = strlen(nazwa);
    for(mio.dwTypeData+=strlen(mio.dwTypeData); mio.dwTypeData[-1] != '\\'; mio.dwTypeData--);
    InsertMenuItem(GetMenu(h_main_wnd), 665, FALSE, &mio);
    modified = 0;
    num++;
}
macro::~macro()
{
    if(modified)
    {
        HANDLE plik;
        DWORD bw;
        plik = CreateFile(nazwa, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
        WriteFile(plik, &action_num, sizeof(int), &bw, NULL);
        for(int i = 0; i < action_num; i++)
            WriteFile(plik, (char*)action[i]+sizeof(HWND), sizeof(UINT)+sizeof(WPARAM)+sizeof(LPARAM), &bw, NULL);
        CloseHandle(plik);
    }
    delete nazwa;
    for(int i = 0; i < action_num; i++)
        delete action[i];
    num--;
}
void macro::Activate(char* n)
{
    for(int i = 0; i < num; i++)
        if(!_stricmp(mac[i]->nazwa, n))
        {
            recording = mac[i];
            for(int k = 0; k < mac[i]->action_num; k++)
                delete mac[i]->action[k];
            mac[i]->action_num = 0;
            mac[i]->modified = 1;
            return;
        }
    mac[num] = new macro(n);
    mac[num-1]->modified = 1;
}
void macro::AddAction(MSG* m)
{
    action[action_num] = new MSG;
    memcpy(action[action_num], m, sizeof(MSG));
    action_num++;
}
void macro::Run()
{
    for(int i = 0; i < action_num; i++)
        SendMessage(
            h_client_wnd,
            action[i]->message,
            action[i]->wParam,
            action[i]->lParam
            );
}
void macro::ReadAll()
{
    HANDLE plik;
    WIN32_FIND_DATA wfa;
    char hem_path[MAX_PATH];
    GetModulePath(hem_path);
    strcat(hem_path, "*.hem");
    //MessageBox(NULL, hem_path, NULL, 0);
    HANDLE search = FindFirstFile(hem_path, &wfa);
    if(search != INVALID_HANDLE_VALUE)
        do
        {
            int atoms;
            DWORD br;
            MSG atom;
            int n = num;

            GetModulePath(hem_path);
            strcat(hem_path, wfa.cFileName);
            mac[n] = new macro(hem_path);
            plik = CreateFile(hem_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            ReadFile(plik, &atoms, sizeof(int), &br, NULL);
            for(int i = 0; i < atoms; i++)
            {
                ReadFile(plik, (char*)&atom+sizeof(HWND), sizeof(UINT)+sizeof(WPARAM)+sizeof(LPARAM), &br, NULL);
                mac[n]->AddAction(&atom);
            }
            CloseHandle(plik);
        }
        while(FindNextFile(search, &wfa));
    FindClose(search);
    recording = 0;
}
void macro::Clear()
{
    int i = 0;
    while(num)
        delete mac[i++];
}

