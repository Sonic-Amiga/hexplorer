#include"hexplorer.h"
#include"hex_common.h"

class cFileType
{
private:
    unsigned char* naglowek;
    int naglowek_len;
    //char nazwa[50];
    char*nazwa;
    static int liczba;
    cFileType(unsigned char* n, char* nzw, int l);
    bool Compare(unsigned char* sprawdz);
public:
    static cFileType* filetype[300];
    ~cFileType();
    char* GetName() { return nazwa; }
    static int Find(unsigned char* sprawdz);
    static void ReadAll();
};

