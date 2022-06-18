#ifndef HEX_POSITION
#define HEX_POSITION

#include"hexplorer.h"
#include"hex_common.h"

extern int column_num;

class cPosition
{
    friend class cRecent;
private:
    static int num;
    int lp;
    int pos;
    char description[50];
public:
    const static int MAX_POSITION = 10;
    cPosition();
    cPosition(cPosition&);
    ~cPosition();
    void GoTo();
    void Reset();
    void Set(int p, char* d = "\0\0");
    void SetMenu(HMENU hMenu, int p);
    bool DrawContextMenu(HINSTANCE,HWND,int,int,int);
    //void Draw(HDC hdc, int x, int y, int p);

    static int draw_pos[MAX_POSITION];
    static int draw_posnum;
    static void PrepareForDraw(cPosition cp[], int begin, int end)
    {
        draw_posnum = 0;
        for(int i = 0; i < MAX_POSITION; i++)
        {
            if(cp[i].pos > begin && cp[i].pos < end)
                draw_pos[draw_posnum++] = cp[i].pos;
        }
    }
    inline bool Draw(HDC hdc, int x, int y, int p)
    {
        if(p == (pos/column_num))
        {
            TextOut(hdc, x, y, description, strlen(description));
            return 1;
        }
        return 0;
    }
    void PrintForNavigator(char*p, int radix);
};

#endif
