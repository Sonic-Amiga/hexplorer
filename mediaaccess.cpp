#include"mediaaccess.h"

cDiskAccess::cDiskAccess(char num)
{
  OSVERSIONINFO versionInfo;
  versionInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
  GetVersionEx(&versionInfo);
  if(versionInfo.dwPlatformId!=VER_PLATFORM_WIN32_NT)
  {
    iErrorCode=HEX_MEDIA_ERR_NT_SYSTEM;
    return;
  }
  //char disk[MAX_PATH];
  strcpy(pname, "\\\\.\\a:");
  pname[4]=num;
  if(
    (
      hDisk=CreateFile(pname, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)
    )
    ==INVALID_HANDLE_VALUE
  )
  {
    iErrorCode=HEX_MEDIA_ERR_ACCESSING_MEDIA;
    return;
  }

  if(!
    DeviceIoControl(hDisk, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskGeometry, sizeof(DISK_GEOMETRY), &(DWORD)br, NULL)
  )
  {
    iErrorCode=HEX_MEDIA_ERR_ACCESSING_DISK_GEOMETRY;
    return;
  }
  lSize=
    diskGeometry.Cylinders.QuadPart*
    diskGeometry.TracksPerCylinder*
    diskGeometry.SectorsPerTrack*
    diskGeometry.BytesPerSector;
  bInsert=false;
}

unsigned char&cDiskAccess::operator[](__int64 laddr)
{
  unsigned char a;
  return a;
}


bool cDiskAccess::Read(unsigned char*pBuff, unsigned int iCount, __int64 lOffset)
{
  // Rownaj wstecz do wielokrotnosci rozmiaru sektora
  while(lOffset%diskGeometry.BytesPerSector)lOffset--;
  // Zwiekszaj do wielokrotnosci rozmiaru sektora
  while(iCount%diskGeometry.BytesPerSector)iCount++;

  SetFilePointer(hDisk, lOffset, ((LONG*)(&lOffset))+1, FILE_BEGIN);
  return ReadFile(hDisk, pBuff, iCount, (DWORD*)&br, NULL);
}


bool cDiskAccess::Write(unsigned char*pBuff, unsigned int iCount, __int64 lOffset)
{
  // Rownaj wstecz do wielokrotnosci rozmiaru sektora
  while(lOffset%diskGeometry.BytesPerSector)lOffset--;
  // Zwiekszaj do wielokrotnosci rozmiaru sektora
  while(iCount%diskGeometry.BytesPerSector)iCount++;

  SetFilePointer(hDisk, lOffset, ((LONG*)(&lOffset))+1, FILE_BEGIN);
  return WriteFile(hDisk, pBuff, iCount, (DWORD*)&br, NULL);
}

