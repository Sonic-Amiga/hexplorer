#ifndef HEX_MEDIA
#define HEX_MEDIA
#define HEX_MEDIA_ERR_NT_SYSTEM 1
#define HEX_MEDIA_ERR_ACCESSING_MEDIA 2
#define HEX_MEDIA_ERR_ACCESSING_DISK_GEOMETRY 3
#define HEX_MEDIA_ERR_NOT_SUPPORTED 4

#include<windows.h>
#include<winioctl.h>

class cMediaAccess
{
protected:
  static const int STD_BUF=200;
  int iErrorCode;
  __int64 lSize;
  bool bInsert;
  int br;
  char pname[STD_BUF];
public:
  cMediaAccess() : iErrorCode(0) {};
  virtual ~cMediaAccess() {};
  virtual unsigned char&operator[](__int64 laddr)=0;
  virtual bool Read(unsigned char*pBuff, unsigned int iCount, __int64 lOffset)=0;
  virtual bool Write(unsigned char*pBuff, unsigned int iCount, __int64 lOffset)=0;
  virtual bool Insert(unsigned char*pBuff, unsigned int iCount, __int64 lOffset)=0;
  virtual bool Delete(unsigned int iCount, __int64 lOffset)=0;
  __int64 GetSize() {return lSize;}
  int GetLastError() {return iErrorCode;}

};

class cDiskAccess : public cMediaAccess
{
protected:
  HANDLE hDisk;
  DISK_GEOMETRY diskGeometry;
public:
  cDiskAccess(char num);
  ~cDiskAccess(){CloseHandle(hDisk);}
  unsigned char&operator[](__int64 laddr);
  bool Read(unsigned char*pBuff, unsigned int iCount, __int64 lOffset);
  bool Write(unsigned char*pBuff, unsigned int iCount, __int64 lOffset);
  bool Insert(unsigned char*pBuff, unsigned int iCount, __int64 lOffset) {iErrorCode=HEX_MEDIA_ERR_NOT_SUPPORTED; return false;}
  bool Delete(unsigned int iCount, __int64 lOffset) {iErrorCode=HEX_MEDIA_ERR_NOT_SUPPORTED; return false;}
  int GetBytesPerSector() {return diskGeometry.BytesPerSector;}
};

#endif
