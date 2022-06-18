#include<windows.h>

class ucContainer
{
private:
    static const int OVER_ALLOC = 5000;
    unsigned char* data;
    int bytes_allocated;
    int size;
    int error;
    bool Realloc(int);
    void CopyToCustomClipboard();
public:
    ucContainer(int init_size = OVER_ALLOC);
    ucContainer(unsigned char*, int);
    ~ucContainer() { delete [] data; }
    ucContainer& operator+=(unsigned char);
    bool Push(unsigned char*, int);
    inline void PutAt(int pos, unsigned char c) { data[pos] = c; }
    unsigned char* GetData() { return data; }
    void CopyToClipboard(HWND);
    bool Empty();
};

