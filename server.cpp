#pragma hdrstop

#include "server.h"
#include "code.h" 
//winsock инициализирована?
char inited=0;

/*

        инициализация winsock

*/
void init_ws()
{
        if(inited)
                return;
        WSAData d;
        WSAStartup(0x0202,&d);
        inited=1;
}
/*

        выгрузить winsock

*/
void clean_ws()
{
        if(!inited)
                return;
        WSACleanup();
        inited=0;
}
//вывести ошибку
extern void error();

stream_server::stream_server(unsigned max)
{
        h=INVALID_SOCKET;
        started=0;
        max_usr=max;
        port=0;
        if(!inited)
                init_ws();
}
stream_server::~stream_server()
{
        if(started)
                close();
        h=INVALID_SOCKET;
        started=0;
        port=0;
}

void stream_server::close()
{
        if(!started)
                return;
        closesocket(h);
}
/*

        запустить сервер

*/
char stream_server::start()
{
        if(started==1)
        {
                return 0;
        }
        if(!port)
                return 0;

        h=socket(AF_INET,SOCK_STREAM,0);
        if(h==INVALID_SOCKET)
        {
                error();
                return 0;
        }
        sockaddr_in addr;
        addr.sin_family=AF_INET;
        addr.sin_port=htons(port);
        addr.sin_addr.s_addr=0;
        if(bind(h,(sockaddr*)&addr,sizeof(addr))==SOCKET_ERROR)
        {
                error();
                closesocket(h);
                return 0;
        }
        if(listen(h,max_usr)==SOCKET_ERROR)
        {
                error();
                closesocket(h);
                return 0;
        }
        started=1;
        return 1;
}
/*

        отослать пакет
*/
char stream_server::send(SOCKET h,packet*p)
{
        #ifndef _DEBUG
                code((char*)p,sizeof(packet));
        #endif
        int ret=::send(h,(char*)p,sizeof(packet),0);
        #ifndef _DEBUG
                code((char*)p,sizeof(packet));
        #endif
        if(ret==SOCKET_ERROR)
                return 0;
        return 1;
}
/*

        принять подключение

*/
char stream_server::_accept(sockaddr_in*addr_out,SOCKET*h_out)
{
        int sz=sizeof(sockaddr_in);
        *h_out=accept(h,(sockaddr*)addr_out,&sz);
        if(*h_out==INVALID_SOCKET)
        {
                return 0;
        }
        return 1;
}

/*

        принять пакет

*/
char stream_server::_recv(SOCKET client,packet*p)
{
        int bytes=0;
        //p->flash();
        bytes=recv(client,(char*)p,sizeof(packet),0);
        if(bytes!=sizeof(packet))
                return 0;
        #ifndef _DEBUG
                code((char*)p,sizeof(packet));
        #endif
        if(!p->check())
                return 0;
        return 1;
}

dgram_server::dgram_server(unsigned max)
{
        max_usr=max;
        started=0;
        port=0;
        h=INVALID_SOCKET;
        if(!inited)
                init_ws();
}
dgram_server::~dgram_server()
{
        if(started)
                close();
        port=0;
        h=INVALID_SOCKET;
}
//закрыть сервер
void dgram_server::close()
{
        if(!started)
                return;
        closesocket(h);
}
//запустить сервер
char dgram_server::start()
{
        if(started)
        {
                return 0;
        }
        if(!port)
        {
                return 0;
        }
        h=socket(AF_INET,SOCK_DGRAM,0);
        if(h==INVALID_SOCKET)
        {
                error();
                return 0;
        }
        sockaddr_in addr;
        addr.sin_family=AF_INET;
        addr.sin_port=htons(port);
        addr.sin_addr.s_addr=0;
        if(bind(h,(sockaddr*)&addr,sizeof(addr))==SOCKET_ERROR)
        {
                error();
                closesocket(h);
                return 0;
        }
        started=1;
        return 1;
}
//получить строку
char dgram_server::_recv(char*buf,unsigned sz,sockaddr_in*adr,int *l)
{
        int s=sizeof(sockaddr_in);
        int bytes=recvfrom(h,buf,sz,0,(sockaddr*)adr,&s);
        if(bytes==SOCKET_ERROR)
                return 0;
        #ifndef _DEBUG
                code((char*)buf,bytes);
        #endif
        *l=bytes;
        buf[bytes]=0;
        return 1;
}
//отправить строку
char dgram_server::send(char*buf,int len,sockaddr_in*addr)
{
        #ifndef _DEBUG
                code(buf,len);
        #endif
        int ret=sendto(h,buf,len,0,(sockaddr*)addr,sizeof(sockaddr_in));
        #ifndef _DEBUG
                code(buf,len);
        #endif
        if(ret==SOCKET_ERROR)
                return 0;
        return 1;
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
