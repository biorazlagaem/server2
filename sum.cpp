/*

        вобще, этот модуль присутствует в программе только в целях
        безопасности. по контрольной сумме проверяется целостность
        и подлинность пакета

*/
#pragma hdrstop

#include "chksum.h"

/*эту строку мы будем спользовать при вычислении контрольной суммы*/
char src[]="wdeas123d3";

//это размер строки
#define SZ 10
//собственно, сама функция
unsigned long checksum(char*buffer,unsigned sz)
{
        /*контрольная сумма*/
        unsigned long ret=0;
        /*номер сивола в строке src*/
        unsigned a=0;
        char*p=buffer;
        for(;p<buffer+sz;p++)
        {
                /*вычисление контрольной суммы*/
                ret+=*p^src[a]+(p-buffer)*2;
                a++;
                if(a>SZ)
                        a=0;
        }
        return ret;

}

//---------------------------------------------------------------------------
#pragma package(smart_init)
