#ifndef HEX_TOOL
#define HEX_TOOL

#include<math.h>
#include"hexplorer.h"
#include"undo.h"
#include"position.h"
#include"mclip.h"
#include"hex_common.h"

enum type
{
    t_byte = 0,
    t_sbyte = 1,
    t_word = 2,
    t_sword = 3,
    t_dword = 4,
    t_sdword = 5,
    t_quad = 6,
    t_squad = 7,
    t_float = 8,
    t_double = 9,

    t_word_motorola = 10,
    t_dword_motorola = 11,
    t_quad_motorola = 12,
};


class member
{
private:
    char name[30];
    type typ;
    int rep;
public:
    member();
    member(const char* n, type t, int r = 1);
    void GetDigit(const void* adres, char* numpos, int radix, bool dots);
    void DeriveInfo(const void* adres, char* dst, int radix = 10);
    int GetSize();
    void SetValue(const void* adres, char* szNewValue);
    void Write(HANDLE plik);
};

class structure
{
private:
    char nazwa[30];
    int mem_num;
    int serial;
    member* skladniki[200];
    void Write(HANDLE plik);
public:
    static int num;
    static structure* st[500];
    structure(char* n);
    ~structure();
    void AddMember(const char* n, type t, int r = 1);
    void AddMember(member* m, int numer);
    void RemoveMember(int numer);
    void CloneMember(int numer);
    void SetValue(int m, char* szNewValue);
    void SetHighlight(int m);
    char* GetName() {  return nazwa;   }
    int GetSerial() { return serial; }
    int GetSize();
    int PrintMembers(char tekst[200][200], int radix = 10);
    static void ReadAll();
    static void WriteAll();
};

class ChildDlg
{
protected:
    HWND hDlg;
    int serial;
    void Align();
    void InitTool();
public:
    static int num;
    static ChildDlg* svd[30];
    ChildDlg();
    virtual ~ChildDlg();
    virtual void Display() = 0;
    static ChildDlg* GetThis(HWND h);
    static void free(HWND hDlg);
    static void AlignAll();
    static void DisplayMembers();
};

class ListChildDlg : public ChildDlg
{
protected:
    int top_index;
    unsigned char radix;
public:
    ListChildDlg();
    void SetTopIndex(int index)
    {
        top_index = index;
    }
    void SetRadix(unsigned char r);
};

class SViewerDlg : public ListChildDlg
{
private:
    structure* tresc;
    HWND h_tool_tip;
public:
    SViewerDlg(structure* s);
    ~SViewerDlg()
    {
        DestroyWindow(h_tool_tip);
    }
    void Display();
    static void SetValue(HWND hDlg, int m, char* szNewValue);
    static void SetHighlight(HWND hDlg, int m);
    static void AddMember(HWND hDlg, int numer);
    static void RemoveMember(HWND hDlg, int numer);
    static void DeleteStructure(HWND hDlg);
    static void Clone(HWND hDlg, int numer);
};

class SimpleDataDlg : public ListChildDlg
{
private:
    static member all_data[13];
    HWND h_tool_tip;
public:
    SimpleDataDlg();
    void Display();
    void SetValue();
    void SetHighlight();
    void CopyToClipboard();
};


class ChecksumDlg : public ListChildDlg
{
private:
    HWND h_tool_tip;
    void Checksum8(int begin, int end, char* t);
    void Checksum16(int begin, int end, char* t);
    void Checksum32(int begin, int end, char* t);
    void Checksum64(int begin, int end, char* t);
    const static unsigned int crc16table[256];
    void CRC16(int begin, int end, char* t);
    const static unsigned int crctttable[256];
    void CRC16CCITT(int begin, int end, char* t);
    static unsigned int crc32table[256];
    void CRC32(int begin, int end, char* t);
    void SHA(int begin, int end, char* t);
    void MD5(int begin, int end, char* t);
    void RIPEMD(int begin, int end, char* t);
public:
    ChecksumDlg();
    ~ChecksumDlg()
    {
        DestroyWindow(h_tool_tip);
    }
    void Display();
    void CopyToClipboard(int c);
    static void InitCRC32();
    static unsigned int UniversalCRC32(unsigned char*, int);
};

class OccurenceDlg : public ListChildDlg
{
public:
    OccurenceDlg();
    void Display();
    void SetHighlight();
};

class NavigatorDlg : public ListChildDlg
{
protected:
    cPosition *pos[200];
    int pos_num;
    HWND h_tool_tip;
public:
    NavigatorDlg(cPosition p[], int);
    ~NavigatorDlg();
    void Display();
    void GoTo(int);
    void Reset(int);
};

class NavigatorFindDlg : public NavigatorDlg
{
public:
    NavigatorFindDlg(cPosition p[], int, char*title);
    ~NavigatorFindDlg();
};

void DrawPixels(void*);
class PixelDlg : public ChildDlg
{
    friend void DrawPixels(void*);
private:
    int column_num;
    unsigned char bpp;
    volatile unsigned long newest_thread;
    HWND h_tool_tip;
public:
    PixelDlg(int col = 250);
    ~PixelDlg()
    {
        DestroyWindow(h_tool_tip);
    }
    void Display();
    void SetBpp(int b);;
    void SetColumns();
};

void CalculatePatterns(void*);
class PatternsDlg : public ListChildDlg
{
    friend void CalculatePatterns(void*);
private:
    volatile unsigned long newest_thread;
    HWND h_tool_tip;
    int MIN_COL;
    int MAX_COL;
    static constexpr double MAX_DOUBLE = 1.7E308;
public:
    PatternsDlg();
    ~PatternsDlg()
    {
        newest_thread=0;
        Sleep(0);
        DestroyWindow(h_tool_tip);
    }
    void Display();
    void SetColumns();
    void ReadColumns();
};

void CalculateFourier(void*);
class FourierDlg : public ListChildDlg
{
    friend void CalculateFourier(void*);
private:
    volatile unsigned long newest_thread;
    HWND h_tool_tip;
    //double*x,*y;
    //int num;
    char file[MAX_PATH];
public:
    FourierDlg();
    ~FourierDlg()
    {
      //delete[]x;
      //delete[]y;
      newest_thread=0;
      Sleep(0);
      DestroyWindow(h_tool_tip);
    }
    //bool Alloc(int);
    void Display();
    void Save();
    //void ReadColumns();
};

class DisassemblerDlg : public ListChildDlg
{
private:
    static const int INST_NUM=100;
    int inst_len[INST_NUM];
public:
    DisassemblerDlg();
    void Display();
    void Jump();
    void SetHighlight();
};
#endif
