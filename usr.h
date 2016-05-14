#ifndef usrH
#define usrH

#include "packet.h"
#include "windows.h"
#include "winsock.h" 
//пока не поддерживается...
//пользователь доступен
#define U_A 0
//не доступен
#define U_NA 1

//ник по-умолчанию
#define DEF_NICK "default"

struct Usr
{
        Usr();
        ~Usr();
        //IP
        char IP[16];
        //ник
        char Nick[MAX_NICK_SZ];
        //сокет
        SOCKET h;
        //оператор?
        char oper;
        //статус
        char status;
        //нужно идентификация?
        char needreg;
        //идентифицирован?
        char reg;
        //секунды?
        char sec;
        //милисекунды?
        char ms;
        //задать IP
        void SetIP(char*);
        //задать ник
        void SetNick(char*);
        Usr*next;
};

struct Users
{
        Usr*user;
        Usr*last;
        unsigned count;
        void add();
        Usr*get(unsigned);
        Usr*get_n(char*);
        Usr*get_IP(char*);
        int get_id(char*);
        int get_h(SOCKET);
        void free(unsigned);
        void close(unsigned);
        //добавить пользователя
        void addusr(char*IP,SOCKET h);
        //сделать опом
        void setop(unsigned);
        //сделать обычным пользователем
        void unop(unsigned);
        //число опов
        unsigned opn();
        Users();
        ~Users();
        //послать пакет
        void Send(unsigned,packet*);
        //послать всем
        void SendAll(packet*);
        //обновить список пользователей
        void Refresh(unsigned);
        //обновить список пользователей у всех
        void RefreshAll();
        //послать операторам
        void SendOps(packet*);
};

//---------------------------------------------------------------------------
#endif
