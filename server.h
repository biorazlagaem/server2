#ifndef serverH
#define serverH

#include "windows.h"
#include "winsock.h"
#include "packet.h"

void init_ws();
void clean_ws();

struct stream_server
{
        stream_server(unsigned max);
        ~stream_server();
        //запущен?
        char started;
        //сокет
        SOCKET h;
        //порт
        int port;
        //макс. количество пользователей
        unsigned max_usr;
        //запустить сервер
        char start();
        //получить пакет
        char _recv(SOCKET,packet*);
        //отправить пакет
        char send(SOCKET h, packet*p);
        //принять подключение
        char _accept(sockaddr_in*,SOCKET*);
        //закрыть сервер
        void close();
};

struct dgram_server
{
        dgram_server(unsigned max);
        ~dgram_server();
        //запущен?
        char started;
        //сокет
        SOCKET h;
        //порт
        int port;
        //макс. кол-во пользователей
        unsigned max_usr;
        //запустить сервер
        char start();
        //получить строку
        char _recv(char*,unsigned,sockaddr_in*,int*);
        //отправить строку
        char send(char*buf,int len,sockaddr_in*addr);
        //закрыть сервер
        void close();
};
//---------------------------------------------------------------------------
#endif
