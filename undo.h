#ifndef HEX_UNDO
#define HEX_UNDO

#include"hexplorer.h"

class cUndo
{
private:
    int message, wParam;
    int s_begin, s_end;
    bool u_insert;
    unsigned char* schowek;
    int schoweklen;
    char* nazwa;
public:
    static int step;
    static bool redirected_by_undo;
    static cUndo* undo[20000];
    cUndo(int m, int w, char* n);
    ~cUndo();
    void SetMenu(HMENU hMenu, int pos);
    void Undo();
    static void Forget();
};

#endif
