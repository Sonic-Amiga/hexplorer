#ifndef HEX_MCLIP
#define HEX_MCLIP

#include"hexplorer.h"
#include"hex_common.h"

class MultiClipboard
{
public:
    const static int MAX_CLIP = 15;
private:
    unsigned char*data;
    int len;
    int serial;
public:
    static int total;
    static MultiClipboard*vc[MAX_CLIP];
    MultiClipboard(unsigned char*, int);
    MultiClipboard(HANDLE);
    ~MultiClipboard();
    void Paste();
    bool Find(HWND);
    void SetMenu(HMENU);
    void Read(HANDLE);
    void Write(HANDLE);
    static void ReadAll();
    static void WriteAll();
    static void Copy(unsigned char*, int);
};
#endif
