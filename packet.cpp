#pragma hdrstop

#include "packet.h"
#include "mem.h"
//контрольная сумма
#include "chksum.h"
//шифрование
#include "code.h"
/*заголовок пакета*/
char _hdr[]="\0\0\4\.\0\+";
/*

        конструктор класса
        ставим все по умолчанию

*/
packet::packet()
{
        memset(to,0,MAX_NICK_SZ);
        memset(from,0,MAX_NICK_SZ);
        memcpy(hdr,_hdr,HDR_SZ);
        type=0;
        size=0;
        chksum=0;
}

packet::~packet()
{
}
/*

        очистить пакет

*/
void packet::flash()
{
        memset(to,0,MAX_NICK_SZ);
        memset(from,0,MAX_NICK_SZ);
        memset(hdr,0,HDR_SZ);
        memcpy(hdr,_hdr,HDR_SZ);
        type=0;
        size=0;
        chksum=0;
}
/*

        проверить подлинность и целостность пакета

*/
char packet::check()
{
        if(memcmp(hdr,_hdr,HDR_SZ)!=0)
                return 0;
        if(chksum==checksum(data,size))
                return 1;
        return 0;
}
/*

        установить ник "кому"

*/
void packet::SetTo(char*str,unsigned sz)
{
        if(sz>=MAX_NICK_SZ)
                sz=MAX_NICK_SZ-1;
        memset(to,0,MAX_NICK_SZ);
        memcpy(to,str,sz);
        to[sz]=0;
}
/*

        "от кого"

*/
void packet::SetFrom(char*str,unsigned sz)
{
        if(sz>=MAX_NICK_SZ)
                sz=MAX_NICK_SZ-1;
        memset(from,0,MAX_NICK_SZ);
        memcpy(from,str,sz);
        from[sz]=0;
}
/*

        установить данные

*/
void packet::SetData(char*str,unsigned sz)
{
        if(sz>MAX_PACKET_SZ)
                sz=MAX_PACKET_SZ;
        memset(data,0,MAX_PACKET_SZ);
        memcpy(data,str,sz);
        data[sz]=0;
        size=sz;
        chksum=checksum(data,sz);
}

/*

        отправить пакет

*/
char SEND(SOCKET h,packet*p)
{
        int ret;
        #ifdef _DEBUG
                ret=send(h,(char*)p,sizeof(packet),0);
        #else
                code((char*)p,sizeof(packet));
                ret=send(h,(char*)p,sizeof(packet),0);
                code((char*)p,sizeof(packet));
        #endif
        if(ret==SOCKET_ERROR)
                return 0;
        return 1;
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
