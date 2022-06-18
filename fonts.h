#include"hexplorer.h"

class Font
{
private:
    static int num;
    int lp;
    HFONT type;
public:
    static int actual;
    Font(HFONT t);
    void Set(HDC hdc);
};

class hexColors
{
public:
    COLORREF kolor[258];
    static hexColors* actual;
    static char scheme;
    hexColors(int init = 0);
    void Set();
};

