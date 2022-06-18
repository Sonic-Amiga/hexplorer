#include"filetype.h"

extern int dlugosc_pliku;
extern char* szAppName;

int cFileType::liczba = 0;
cFileType* cFileType::filetype[300];

cFileType::cFileType(unsigned char* n, char* nzw, int l)
{
    naglowek = new unsigned char[l];
    naglowek_len = l;
    CopyMemory(naglowek, n, l);
    nazwa=new char[strlen(nzw)+1];
    strcpy(nazwa, nzw);
    liczba++;
}
bool cFileType::Compare(unsigned char* sprawdz)
{
    return !memcmp(naglowek, sprawdz, naglowek_len);
}
cFileType::~cFileType()
{
    delete [] naglowek;
    delete[]nazwa;
    liczba--;
}
int cFileType::Find(unsigned char* sprawdz)
{
    for(int i = 2; i < liczba; i++)
        if(filetype[i]->Compare(sprawdz))
            return i;
    for(int i = 0; i < dlugosc_pliku; i++)
        if((sprawdz[i] <= 31 || sprawdz[i] >= 127) && sprawdz[i] != 0x0d && sprawdz[i] != 0x0a && sprawdz[i] != 0x09)
            return 0;
    return 1;
}

void cFileType::ReadAll()
{
    HANDLE plik;
    unsigned char bufor[50000];
    char module_name[MAX_PATH];
    int file_size, pos, name_size, header_size;
    DWORD br;
    //WIN32_FIND_DATA w32fd;

    filetype[liczba] = new cFileType(NULL, "Unknown", 0);
    filetype[liczba] = new cFileType(NULL, "Text file", 0);

    //GetModuleFileName(NULL, module_name, MAX_PATH - 1);
    //strcpy(module_name + strlen(module_name) - 13, "headers.dat");
    GetModulePath(module_name);
    strcat(module_name, "headers.dat");



    plik = CreateFile(module_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(plik == INVALID_HANDLE_VALUE)
        MessageBox(NULL, "Program files corrupted", szAppName, 0);
    else
    {
        file_size = GetFileSize(plik, NULL);
        ReadFile(plik, bufor, file_size, &br, NULL);
        CloseHandle(plik);

        pos = 0;
        while(pos < file_size)
        {
            name_size = strlen((char*)(bufor + pos));
            header_size = (int)bufor[pos + name_size + 1];
            filetype[liczba] = new cFileType(
                bufor + pos + name_size + 2,
                (char*)(bufor + pos),
                header_size);
            pos += name_size + header_size + 3;
        }
    }
}

