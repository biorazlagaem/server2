
#pragma hdrstop

#include "code.h"

//строка используется для шифрования строки
char src[]="sadjkgh124df";

//размер строки
#define SZ 12

//функция шифрования строк
void code(char*buffer,unsigned sz)
{
        char*p=buffer;
        //элемент строки
        unsigned a=0;
        for(;p<buffer+sz;p++)
        {
                //шифрование строки
                *p=(*p)^src[a]+(p-buffer)*2;
                a++;
                if(a>SZ)
                        a=0;
        }
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
