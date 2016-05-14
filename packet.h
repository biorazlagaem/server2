
#ifndef packetH
#define packetH

/*packet types*/
#define _UNDEF 0
#define PUB 1
#define PRIV 2
#define PING 3
#define TOPIC 4
#define GETMAIL 5
#define LIST 6
/*file routine*/
#define FOPEN 7
#define FDATA 8
#define FEND 9
/*file routine*/
#define NAME 10
#define SERVER 11
#define _MOTD 12
#define SELF 13
#define CLEAR 14
#define INFO 15
#define KICK 16
#define OP 17
#define UNOP 18
#define BAN 19
#define UNBAN 20
#define NEWMAIL 21
#define DEN 22
#define ACCEPT 23
#define BANLIST 24
#define ROK 25
#define RNO 26
#define REQ 27
#define IDENTIFY 28
#define REGISTER 29
#define UNREGISTER 30
#define BANIP 31
#define _ID 32

/*макс. размер пакета*/
#define MAX_PACKET_SZ 1024
#define MAX_NICK_SZ 31

/*размер заголовка*/
#define HDR_SZ 6
/*пакет*/
struct packet
{
        /*заголовок*/
        char hdr[HDR_SZ];
        /*от кого*/
        char from[MAX_NICK_SZ];
        /*кому*/
        char to[MAX_NICK_SZ];
        /*тип*/
        unsigned char type;
        /*размер данных*/
        unsigned short size;
        /*контрольная сумма*/
        unsigned long chksum;
        /*данные*/
        char data[MAX_PACKET_SZ];
        packet();
        ~packet();
        void SetTo(char*,unsigned);
        void SetFrom(char*,unsigned);
        void SetData(char*,unsigned);
        void flash();
        char check();
};

#include "windows.h"
#include "winsock.h"
//послать пакет
char SEND(SOCKET h,packet*);

//---------------------------------------------------------------------------
#endif
