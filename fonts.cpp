#include"fonts.h"

extern int cxChar, cyChar;

int Font::num = 0;
int Font::actual = 0;

Font::Font(HFONT t)
{
    type = t;
    lp = num++;
}
void Font::Set(HDC hdc)
{
    TEXTMETRIC tm;
    SelectObject(hdc, type);
    GetTextMetrics(hdc, &tm);
    cxChar = tm.tmAveCharWidth * (tm.tmPitchAndFamily & 1 ? 3 : 2) / 2 + 2;
    cyChar = tm.tmHeight + tm.tmExternalLeading + 2;
    actual = lp;
}

hexColors* hexColors::actual;
char hexColors::scheme = 0;

hexColors::hexColors(int init) //  = 0
{
    if(init == 1)
    {
        for(int i = 0; i < 256; i++)
        {
            int kol = i;
            /*
            if(kol > 32 && kol < 127)
                kolor[i] = 0xff;
            else */if (!kol||kol==255)
               kolor[i]=0xff;
            else
            {
                /*  to zostawic w spokoju bo stary matrix tak byl definiowany
                if(kol >= 127)
                    kol -= 94;
                kol>>=1;
                kol += 87;
                kolor[i] = ((kol>>1)<<16)|kol<<8;
                */
                kolor[i]=0;
            }
        }
        kolor[256] = 0xffffff;
    }
    else if(init == 2)
    {
        for(int i = 0; i < 256; i++)
        {
            int kol = i;
            if(kol > 32 && kol < 127)
                kolor[i] = 0xa000;
            else
            {

                if(kol >= 127)
                    kol -= 94;
                kol += 93;
                kolor[i] = (kol<<8)|0xff;
            }
        }
        kolor[256] = 0;
    }
    else if(init == 3)
    {
        for(int i = 0; i < 256; i++)
            kolor[i] = 0xa000;
        kolor[256] = 0;
    }
    else if(init == 4)
    {
        for(int i = 0; i < 256; i++)
        {
            int ziel = i;
            ziel>>=1;
            ziel+=93;
            if(i<128)
              kolor[i] = ziel<<8;
            else
            {
              int brightness=i-127;
              kolor[i]=(ziel<<8)+brightness+(brightness<<16);
            }
        }
        kolor[256] = 0;
    }
    else if(init == 5)
    {
        for(unsigned int i=0;i<256;i++)
          kolor[i]=!i||!~(char)i?0xffffff:0xffff;
        kolor[256] = 0xff0000;
    }
    else     // = 0 (B&W)
    {
        for(int i = 0; i < 256; i++)
            kolor[i] = 0;
        kolor[256] = 0xffffff;
    }
    actual = this;
}
void hexColors::Set(){  actual = this; }

