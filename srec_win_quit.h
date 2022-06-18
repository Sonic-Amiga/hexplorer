#ifndef INCLUDE_SREC_WIN_QUIT
#define INCLUDE_SREC_WIN_QUIT
#include<cstdio>
#include<windows.h>
#include<quit.h>
/**
  * ENG:
  * The srec_win_quit class is used to represent a srecords quit handler which
  * uses standard windows messages to print errors
  * and oveloaded exit method, which actually doesn't end the program, but
  * saves an error code for further use.
  * PL:
  * Klasa srec_win_quit reprezentuje uchwyt quit biblioteki srecords ktory korzysta
  * ze standardowych okienek sytemu Windows do wypisywanie bledow
  * oraz zawiera przeladowana metode exit, ktora jednak nie konczy programu,
  * a jedynie zapamietuje numer bledu, ktory moze byc potem sprawdzony.
  */
class srec_win_quit : public quit
{
protected:
  HWND hwnd;
  char caption[256];
  int quit_msg;
public:
  srec_win_quit();
  srec_win_quit(HWND, const char*);
  srec_win_quit(const srec_win_quit&arg){}
  srec_win_quit &operator=(const srec_win_quit & arg){return *this;}
  virtual ~srec_win_quit() {}
  virtual void message_v(const char *, va_list);
  virtual void exit(int);
  int get_quit_msg(){ return quit_msg; }
};
#endif
