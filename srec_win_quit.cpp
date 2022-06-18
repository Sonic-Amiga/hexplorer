#include"srec_win_quit.h"

srec_win_quit::srec_win_quit()
{
  strcpy(caption, "Srecord message");
  hwnd=NULL;
  quit_msg=0;
}

srec_win_quit::srec_win_quit(HWND hw, const char*t)
{
  if(strlen(t)>255)
  {
    CopyMemory(caption,t,255);
    caption[255]=0;
  }
  else
    strcpy(caption, t);
  hwnd=hw;
  quit_msg=0;
}

void srec_win_quit::message_v(const char*fmt, va_list ap)
{
  char buf[1024];
  vsnprintf(buf, sizeof(buf), fmt, ap);
  MessageBox(hwnd, buf, caption, MB_OK);
}

void srec_win_quit::exit(int msg) { quit_msg=msg; }





