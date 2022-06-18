#include"container.h"

extern unsigned char* schowek;
extern int schoweklen;

bool ucContainer::Realloc(int new_size)
{
    try
    {
        unsigned char* temp = data;
        data = new unsigned char[new_size];
        CopyMemory(data, temp, size);
        delete [] temp;
        bytes_allocated = new_size;
    }
    catch(...)
    {
        error = 1;
        return 0;
    }
    return 1;
}

void ucContainer::CopyToCustomClipboard()
{
    delete [] schowek;
    schowek = new unsigned char[size];
    CopyMemory(schowek, data, size);
    schoweklen = size;
}

ucContainer::ucContainer(int init_size) //  = OVER_ALLOC
{
    data = 0;
    error = 0;
    size=0;
    Realloc(init_size);
}

ucContainer::ucContainer(unsigned char*init, int init_size)
{
    data = 0;
    error = 0;
    size=0;
    Realloc(init_size + OVER_ALLOC);
    size = init_size;
    CopyMemory(data, init, init_size);
}

ucContainer& ucContainer::operator+=(unsigned char c)
{
    if(size + 100 > bytes_allocated)
        if(!Realloc(size + OVER_ALLOC))
            return *this;
    data[size++] = c;
    return *this;
}

bool ucContainer::Push(unsigned char* c, int len)
{
    if(size + len + 100 > bytes_allocated)
        if(!Realloc(size + len + OVER_ALLOC))
            return 0;
    CopyMemory(data + size, c, len);
    size+=len;
    return 1;
}

void ucContainer::CopyToClipboard(HWND hwnd)
{
    HGLOBAL hGlobal;
    unsigned char* pGlobal;
    hGlobal = GlobalAlloc(GHND | GMEM_SHARE, size + 1);
    pGlobal = (unsigned char*)GlobalLock(hGlobal);
    CopyMemory(pGlobal, data, size);
    GlobalUnlock(hGlobal);
    OpenClipboard(hwnd);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hGlobal);
    CloseClipboard();
    CopyToCustomClipboard();
}

bool ucContainer::Empty()
{
  int tmp=size;
  size=0;
  return tmp;
}
