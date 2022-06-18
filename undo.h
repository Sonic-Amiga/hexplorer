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

    // Undo actions stack
    static int step;
    static cUndo* undo[20000];

    void Undo();
public:
    cUndo(int m, int w, char* n);
    ~cUndo();

    static bool redirected_by_undo;
    void SetMenu(HMENU hMenu, int pos);
    static void Forget();
    static void Store(int m, int w, char *n) {
        undo[step++] = new cUndo(m, w, n);
    }
    static void Execute() {
        undo[--step]->Undo();
        delete undo[step];
    }
    static cUndo* GetLast() {
        return step ? undo[step - 1] : nullptr;
    }
    static bool empty() {
        return step == 0;
    }
};

#endif
