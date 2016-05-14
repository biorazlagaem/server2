
#include <windows.h>
#pragma hdrstop
//серверы
#include "server.h"
//пользователи
#include "usr.h"
//баны
#include "ban.h"
//регистрация
#include "regist.h"
//почта
#include "mail.h"

//id горячей кнопки для закрытия 
#define KEYSTOP 0
//TCP и UDP серверы
stream_server *TCPs=0;
dgram_server *UDPs=0;

//имя сервера
char*s_name= "ChanServ\0";
//длина имени сервера
#define s_sz 8

//пользователи
Users*users=0;
//баны
Bans*ban=0;
//регистрация
Regged*reg=0;
//почта
mail*Mail=0;

//отслеживать атаки
int trace=0;
//уровень жесткости отслеживания
int tstat=0;
//число пользоватей
int ucnt=0;
//время для отслеживания
int ttime=0;
//закрыть?
char close=0;
//тема
packet topic;
//most of the day
packet MOTD;

//---------------------------------------------------------------------------
//загрузить настройки
void load(char*file);
//запустить сервер UDP
void StartUDP();
//выполнить действие
void DoEvent(SOCKET h,packet*);
/*

        проверка на состояние отладки

*/
DWORD __stdcall check(void*)
{
        if(!GetModuleHandle("kernel32.dll"))
        {
                ExitProcess(0);
        }
        while(!close)
        {
                if(IsDebuggerPresent())
                        ExitProcess(0);
                Sleep(10);
        }
        return 0;
}
/*

        зарегистрировать горячие клавиши

*/
void RegisterKeys()
{
        if(!RegisterHotKey(0,KEYSTOP,MOD_ALT,VK_F12))
                ExitProcess(0);
}
/*

        ввод пароля для идентификации

*/
DWORD __stdcall regs(void*)
{
        Usr*t;
        packet p;
        p.type=SERVER;
        p.SetFrom(s_name,s_sz);

        while(!close)
        {
                t=users->user;
                for(unsigned a=0;a<users->count;a++)
                {
                        if(t->needreg)
                        {
                                t->sec++;
                                if(t->sec%20==0)
                                {
                                        char*s=new char[256];
                                        sprintf(s,"У вас осталось %d секунд для ввода пароля",60-t->sec);
                                        p.SetData(s,strlen(s));
                                        users->Send(a,&p);
                                        delete [] s;
                                }
                                if(t->sec>60)
                                {
                                        users->close(a);
                                }
                        }
                        t=t->next;
                }
                Sleep(1000);
        }
        return 0;
}

void StartRegs()
{
        DWORD id;
        CreateThread(0,0,regs,0,0,&id);
}

void StartCheck()
{
        #ifdef _DEBUG
                return;
        #else
                DWORD id=0;
                CreateThread(0,0,check,0,0,&id);
        #endif
}
/*

        отслеживание атак.

*/
DWORD __stdcall Trace(void*)
{
        packet p;
        p.type=SERVER;
        p.SetFrom(s_name,s_sz);
        while(!close)
        {
                Usr*t;
                for(unsigned a=0;a<users->count;a++)
                {
                        t=users->get(a);
                        if(!strcmp(t->Nick,DEF_NICK))
                        {
                                t->ms++;
                                if(t->ms>100)
                                {
                                        t->ms=0;
                                        char*s=new char[256];
                                        sprintf(s,"Сервер закрыл соединение с IP %s т.к. принял его как атакующее соединение...",t->IP);
                                        p.SetData(s,strlen(s));
                                        users->SendOps(&p);
                                        if(tstat)
                                        {
                                                ban->add_ban(t->IP);
                                                sprintf(s,"Сервер забанил данный IP");
                                                p.SetData(s,strlen(s));
                                                users->SendOps(&p);
                                        }
                                        users->close(a);
                                }
                        }
                }
                Sleep(10);
        }
        return 0;
}

void StartTrace()
{
        if(!trace)
                return;
        DWORD id=0;
        CreateThread(0,0,Trace,0,0,&id);
}
/*

        вывести сообщение об ошибке

*/
void error()
{
        DWORD e=GetLastError();
        char*mes=0;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,e,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &mes,0,NULL);
        MessageBox(0,(LPCSTR)mes,"LastError",0);
}


#pragma argsused
WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
        StartCheck();
        TCPs=new stream_server(ucnt);
        UDPs=new dgram_server(ucnt);
        Mail=new mail();
        users=new Users();
        ban=new Bans();
        reg=new Regged();

        load("etc\\config.txt");
        if(trace)
        {
                StartTrace();
        }
        Mail->setfile("etc\\mail.txt");
        reg->SetFile("etc\\reg.txt");
        ban->SetFile("etc\\ban.txt");

        if(!Mail->load())
        {
                MessageBox(0,"Ошибка загрузки файла \"etc\\mail.txt\"\nзавершение программы...","",MB_ICONERROR+MB_OK);
                close=1;
                ExitProcess(1);
                return 0;
        }
        if(!ban->Load())
        {
                MessageBox(0,"Ошибка загрузки файла \"etc\\ban.txt\"\nзавершение программы...","",MB_ICONERROR+MB_OK);
                close=1;
                ExitProcess(1);
                return 0;
        }
        if(!reg->Load())
        {
                MessageBox(0,"Ошибка загрузки файла \"etc\\reg.txt\"\nзавершение программы...","",MB_ICONERROR+MB_OK);
                close=1;
                ExitProcess(1);
                return 0;
        }
        init_ws();
        if(!TCPs->start())
        {
                close=1;
                Sleep(1000);
                delete Mail;
                delete ban;
                delete reg;
                delete TCPs;
                clean_ws();
                return 0;
                goto fin;
        }
        if(!UDPs->start())
        {
                goto fin;
        }
        RegisterKeys();
        StartUDP();
        StartRegs();

        MSG msg;
        while(!close)
        {
                if(GetMessage(&msg,NULL,0,0))
                {
                        if((msg.message==WM_HOTKEY)&&((int)msg.wParam==KEYSTOP))
                        {
                                if(MessageBox(HWND_DESKTOP,"закрыть сервер?","",MB_YESNO+MB_ICONQUESTION)==IDYES)
                                {
                                        close=1;
                                }
                        }
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                }
        }


        TCPs->close();
        UDPs->close();
        fin:
        /*чистим все за собой*/
        delete TCPs;
        delete UDPs;
        clean_ws();
        Mail->save();
        delete Mail;
        ban->Save();
        delete ban;
        reg->Save();
        delete reg;
        return 0;
}
//---------------------------------------------------------------------------
//чтение строки из файла
void read(char*buffer,FILE*f)
{
        do
        {
                fgets(buffer,255,f);
        }while(*buffer=='\n' || *buffer=='/');
        if(buffer[strlen(buffer)-1]=='\n')
                buffer[strlen(buffer)-1]=0;
}
/*

        загрузить настройки

*/
void load(char*file)
{
        FILE*f=fopen(file,"rt");
        if(!f)
        {
                error();
                ExitProcess(0);
        }
        char*str=new char[257];
        str[256]=0;
        read(str,f);
        sscanf(str,"USERS %d",&ucnt);
        read(str,f);
        sscanf(str,"STREAM_PORT %d",&TCPs->port);
        read(str,f);
        sscanf(str,"DGRAM_PORT %d",&UDPs->port);
        read(str,f);
        MOTD.SetData(str,strlen(str));
        MOTD.SetFrom(s_name,s_sz);
        MOTD.type=_MOTD;
        read(str,f);
        topic.SetData(str,strlen(str));
        topic.SetFrom(s_name,s_sz);
        topic.type=TOPIC;
        read(str,f);
        sscanf(str,"trace %d",&trace);
        read(str,f);
        sscanf(str,"trace_stat %d",&tstat);
        read(str,f);
        sscanf(str,"time %d",&ttime);
        delete [] str;
        fclose(f);
}
/*

        поток для одного клиента

*/
DWORD __stdcall client(void*point)
{
        SOCKET h=*((SOCKET*)point);
        delete point;
        packet*p=new packet();
        int id=users->get_h(h);
        //получаем пакет с именем
        if(!TCPs->_recv(h,p))
        {
                closesocket(h);
                delete p;
                return 0;
        }
        if(p->type!=NAME)
        {
                delete p;
                return 0;
        }else
        {/*устанавливаем имя пользователю*/
                char*str=new char[256];
                if(id>=0)
                {
                        char*s=new char[256];
                        sprintf(s,"@%s",p->data);
                        int i=users->get_id(p->data);
                        if(i>=0 || users->get_id(s)>=0)
                        {
                                sprintf(s,"Пользователь с данным ником уже существует - измените ник!");
                                p->flash();
                                p->type=SERVER;
                                p->SetFrom(s_name,s_sz);
                                p->SetData(s,strlen(s));
                                users->Send(id,p);
                                delete []s;
                                Usr*t=users->get(id);
                                char*str=new char[256];
                                sprintf(str,"%s покинул канал", t->Nick);
                                users->close(id);
                                p->flash();
                                p->type=SERVER;
                                p->SetFrom(s_name,s_sz);
                                p->SetData(str,strlen(str));
                                users->SendAll(p);
                                delete p;
                                delete [] str;
                                users->RefreshAll();

                                return 0;
                        }
                        delete [] s;
                        sprintf(str,"%s изменил имя на %s",users->get(id)->Nick,p->data);
                        users->get(id)->SetNick(p->data);
                        packet*_p=new packet();
                        _p->type=_ID;
                        _p->SetData(p->data,strlen(p->data));
                        users->Send(id,_p);
                        _p->type=SERVER;
                        _p->SetFrom(s_name,s_sz);
                        _p->SetData(str,strlen(str));
                        users->SendAll(_p);
                        if(users->get(id)->oper)
                        {
                                users->setop(id);
                        }else
                        {
                                int ops=users->opn();
                                if(ops<1)
                                {
                                        sprintf(str,"Сервер присвоил %s статус оператора",users->get(id)->Nick);
                                        users->setop(id);
                                        _p->SetData(str,strlen(str));
                                        users->SendAll(_p);
                                }
                        }
                        int tmp=reg->IsReg(checksum(p->data,strlen(p->data)));
                        if(tmp)
                        {
                                sprintf(str,"%s, данный ник зарегистрирован!!! вам необходимо ввести пароль!!!", p->data);
                                _p->SetData(str,strlen(str));
                                users->Send(id,_p);
                                Usr*u=users->get(id);
                                u->needreg=1;
                                u->reg=0;
                                u->sec=0;
                                //sd
                        }
                        delete _p;
                        users->RefreshAll();
                }
                delete [] str;
        }
        //шлем Most Of The Day
        //и топик
        users->Send(id,&MOTD);
        users->Send(id,&topic);
        //обновляе список пользователей
        users->Refresh(id);
        char ok=0;
        do
        {
                ok=TCPs->_recv(h,p);
                if(ok)
                {
                        DoEvent(h,p);
                }
        }while(ok&&!close);
        id=users->get_h(h);
        if(id>=0)
        {
                Usr*t=users->get(id);
                char*str=new char[256];
                sprintf(str,"%s покинул канал", t->Nick);
                users->close(id);
                p->flash();
                p->type=SERVER;
                p->SetFrom(s_name,s_sz);
                p->SetData(str,strlen(str));
                users->SendAll(p);
                delete p;
                delete [] str;
        }
        users->RefreshAll();
        return 0;
}

void StartRecv(SOCKET p)
{
        DWORD id;
        SOCKET*h=new SOCKET;
        *h=p;
        CreateThread(0,0,client,h,0,&id);
}
/*

        принять подключение

*/
DWORD __stdcall accpt(void*)
{
        sockaddr_in addr;
        SOCKET h=0;
        if(TCPs->_accept(&addr,&h))
        {
                users->addusr(inet_ntoa(addr.sin_addr),h);
                StartRecv(h);
        }
        return 0;
}

void StartAccept()
{
        DWORD id=0;
        CreateThread(0,0,accpt,0,0,&id);
}
/*

        принятие запроса на подключение

*/
DWORD __stdcall u_recv(void*)
{
        char*buf=new char[10];
        memset(buf,0,10);
        sockaddr_in addr;
        int len=0;
        while(!close)
        {
                if(UDPs->_recv(buf,10,&addr,&len))
                {
                        if(ban->IsBanned_(inet_ntoa(addr.sin_addr)))
                        {
                                UDPs->send("ban",3,&addr);
                                continue;
                        }
                        if(!memcmp(buf,"accpt",5))
                        {
                                StartAccept();
                                UDPs->send("OK",2,&addr);
                        }
                }
        }

        return 0;
}

void StartUDP()
{
        DWORD id=0;
        CreateThread(0,0,u_recv,0,0,&id);
}
/*

        выполнить действие в зависимости от типа пакета

*/
void DoEvent(SOCKET h,packet*p)
{
        int id=users->get_h(h);
        Usr*t=users->get(id);
        //установим ник "от кого"
        p->SetFrom(t->Nick,strlen(t->Nick));
        /*пинг - просто пересылаем ответ*/
        if(p->type==PING)
        {
                users->Send(id,p);
                return;
        }
        //"общее" сообщение - отослать всем
        if(p->type==PUB)
        {
                users->SendAll(p);
                return;
        }
        //сообщение в третьем лице
        if(p->type==SELF)
        {
                users->Send(id,p);
                return;
        }
        //смена имени
        if(p->type==NAME)
        {
                char*str=new char[256];
                if(id>=0)
                {
                        char*s=new char[256];
                        sprintf(s,"@%s",p->data);
                        int i=users->get_id(p->data);
                        if(i>=0 || users->get_id(s)>=0)
                        {
                                sprintf(s,"Пользователь с данным ником уже существует - измените ник!");
                                p->flash();
                                p->type=SERVER;
                                p->SetFrom(s_name,s_sz);
                                p->SetData(s,strlen(s));
                                users->Send(id,p);
                                delete []s;
                                return;
                        }
                        delete [] s;
                        sprintf(str,"%s изменил имя на %s",users->get(id)->Nick,p->data);
                        users->get(id)->SetNick(p->data);
                        packet*_p=new packet();
                        _p->type=_ID;
                        _p->SetData(p->data,strlen(p->data));
                        users->Send(id,_p);
                        _p->type=SERVER;
                        _p->SetFrom(s_name,s_sz);
                        _p->SetData(str,strlen(str));
                        users->SendAll(_p);
                        if(t->oper)
                        {
                                users->setop(id);
                        }else
                        {
                                int ops=users->opn();
                                if(ops<1)
                                {
                                        sprintf(str,"Сервер присвоил %s статус оператора",users->get(id)->Nick);
                                        users->setop(id);
                                        _p->SetData(str,strlen(str));
                                        users->SendAll(_p);
                                }
                        }
                        int tmp=reg->IsReg(checksum(p->data,strlen(p->data)));
                        if(tmp)
                        {
                                sprintf(str,"%s, данный ник зарегистрирован!!! вам необходимо ввести пароль!!!", p->data);
                                _p->SetData(str,strlen(str));
                                users->Send(id,_p);
                                Usr*u=users->get(id);
                                u->needreg=1;
                                u->reg=0;
                                u->sec=0;
                        }
                        delete _p;
                        users->RefreshAll();
                }
                delete [] str;
                return;
        }
        //отправка списка пользователей
        if(p->type==LIST)
        {
                users->Refresh(id);
                return;
        }
        //смена темы
        if(p->type==TOPIC)
        {
                topic.SetFrom(p->from,strlen(p->from));
                topic.SetData(p->data,strlen(p->data));
                users->SendAll(&topic);
                p->flash();
                p->type=SERVER;
                p->SetFrom(s_name,s_sz);
                p->SetData("тема изменена...",16);
                users->SendAll(p);
                return;
        }
        //отправить инфо о пользователе
        if(p->type==INFO)
        {
                int i=users->get_id(p->data);
                if(i>=0)
                {
                        p->flash();
                        p->SetFrom(s_name,s_sz);
                        p->type=SERVER;
                        char*str=new char[256];
                        Usr*t=users->get(i);
                        sprintf(str,"Ник: %s, IP:%s, сокет:%d, Оператор?:%s",t->Nick,t->IP,t->h,t->oper?"да":"нет");
                        p->SetData(str,strlen(str));
                        users->Send(id,p);
                }
                return;
        }
        //опнуть пользователя
        if(p->type==OP)
        {
                if(t->oper)
                {
                        int i=users->get_id(p->data);
                        if(i>=0)
                        {
                               users->setop(i);
                               Usr*t=users->get(i);
                               char*s=new char[256];
                               sprintf(s,"%s присвоил %s статус оператора.",p->from,t->Nick);
                               p->flash();
                               p->SetFrom(s_name,s_sz);
                               p->SetData(s,strlen(s));
                               p->type=SERVER;
                               users->SendAll(p);
                               users->RefreshAll();
                               delete [] s;
                        }
                }else
                {
                        p->flash();
                        p->type=SERVER;
                        p->SetFrom(s_name,s_sz);
                        char*s="вы не оператор канала!!! Вам данная функция не доступна!";
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                }
                return;
        }
        //убрать привилегии
        if(p->type==UNOP)
        {
                if(t->oper)
                {
                        int i=users->get_id(p->data);
                        if(i>=0)
                        {
                               users->unop(i);
                               Usr*t=users->get(i);
                               char*s=new char[256];
                               sprintf(s,"%s убрал статус оператора у %s.",p->from,t->Nick);
                               p->flash();
                               p->SetFrom(s_name,s_sz);
                               p->SetData(s,strlen(s));
                               p->type=SERVER;
                               users->SendAll(p);
                               users->RefreshAll();
                               delete [] s;
                        }
                }else
                {
                        p->flash();
                        p->type=SERVER;
                        p->SetFrom(s_name,s_sz);
                        char*s="вы не оператор канала!!! Вам данная функция не доступна!";
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                }
                return;
        }
        //кикнуть пользователя
        if(p->type==KICK)
        {
                if(t->oper)
                {
                        int i=users->get_id(p->data);
                        if(i>=0)
                        {
                               Usr*t=users->get(i);
                               char*s=new char[256];
                               sprintf(s,"%s кикнул %s",p->from,t->Nick);
                               p->flash();
                               p->SetFrom(s_name,s_sz);
                               p->SetData(s,strlen(s));
                               p->type=SERVER;
                               users->SendAll(p);
                               users->close(i);
                               users->RefreshAll();
                               delete [] s;
                        }
                }else
                {
                        p->flash();
                        p->type=SERVER;
                        p->SetFrom(s_name,s_sz);
                        char*s="вы не оператор канала!!! Вам данная функция не доступна!";
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                }
                return;
        }
        //забанить пользователя
        if(p->type==BAN)
        {
                if(t->oper)
                {
                        int i=users->get_id(p->data);
                        if(i>=0)
                        {
                               Usr*t=users->get(i);
                               char*s=new char[256];
                               sprintf(s,"%s забанил %s (IP:%s)",p->from,t->Nick,t->IP);
                               p->flash();
                               p->SetFrom(s_name,s_sz);
                               p->SetData(s,strlen(s));
                               p->type=SERVER;
                               users->SendAll(p);
                               ban->add_ban(t->IP);
                               delete [] s;
                        }
                }else
                {
                        p->flash();
                        p->type=SERVER;
                        p->SetFrom(s_name,s_sz);
                        char*s="вы не оператор канала!!! Вам данная функция не доступна!";
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                }
                return;
        }
        //забанить IP
        if(p->type==BANIP)
        {
                if(t->oper)
                {
                        char*s=new char[256];
                        sprintf(s,"%s забанил IP %s",p->from,p->data);
                        ban->add_ban(p->data);
                        p->flash();
                        p->SetFrom(s_name,s_sz);
                        p->SetData(s,strlen(s));
                        p->type=SERVER;
                        users->SendAll(p);
                        delete [] s;
                }else
                {
                        p->flash();
                        p->type=SERVER;
                        p->SetFrom(s_name,s_sz);
                        char*s="вы не оператор канала!!! Вам данная функция не доступна!";
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                }
                return;
        }
        //список банов
        if(p->type==BANLIST)
        {
                if(t->oper)
                {
                        char*s=new char[256];
                        sprintf(s,"всего %d банов",ban->count);
                        p->flash();
                        p->SetFrom(s_name,s_sz);
                        p->SetData(s,strlen(s));
                        p->type=SERVER;
                        users->Send(id,p);
                        for(unsigned a=0;a<ban->count;a++)
                        {
                                Ban*b=ban->get(a);
                                in_addr tmp;
                                tmp.S_un.S_addr=b->ip;
                                sprintf(s,"Бан №%d - %s",a+1,inet_ntoa(tmp));
                                p->SetData(s,strlen(s));
                                users->Send(id,p);
                        }
                        delete [] s;
                }else
                {
                        p->flash();
                        p->type=SERVER;
                        p->SetFrom(s_name,s_sz);
                        char*s="вы не оператор канала!!! Вам данная функция не доступна!";
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                }
                return;
        }
        //разбанить IP
        if(p->type==UNBAN)
        {
                if(t->oper)
                {
                        int i=ban->get_id(inet_addr(p->data));
                        if(i>=0)
                        {
                               Usr*t=users->get(i);
                               char*s=new char[256];
                               sprintf(s,"%s разбанил IP %s",p->from,p->data);
                               p->flash();
                               p->SetFrom(s_name,s_sz);
                               p->SetData(s,strlen(s));
                               p->type=SERVER;
                               users->SendAll(p);
                               ban->unban(t->IP);
                               delete [] s;
                        }
                }else
                {
                        p->flash();
                        p->type=SERVER;
                        p->SetFrom(s_name,s_sz);
                        char*s="вы не оператор канала!!! Вам данная функция не доступна!";
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                }
                return;
        }
        //добавить новое почтовое сообщение
        if(p->type==NEWMAIL)
        {
                Mail->add();
                p->type=NEWMAIL;
                Mail->last->SetMess(p);
                if(*p->from=='@')
                {
                        Mail->last->message->SetFrom(p->from+1,strlen(p->from+1));
                }
                char*str=new char[256];
                sprintf(str,"Сообщение для %s добавлено в список сообщений",p->to);
                p->flash();
                p->type=SERVER;
                p->SetFrom(s_name,s_sz);
                p->SetData(str,strlen(str));
                users->Send(id,p);
                return;
        }
        //отправить почту пользователю
        if(p->type==GETMAIL)
        {
                int cnt;
                char*nick=p->from;
                if(t->oper)
                {
                        nick++;
                }
                cnt=Mail->getmcount(nick);
                char*str=new char[256];
                sprintf(str,"Для вас имеется %d сообщений",cnt);
                packet pack;
                pack.type=SERVER;
                pack.SetFrom(s_name,s_sz);
                pack.SetData(str,strlen(str));
                users->Send(id,&pack);
                int tmp=1;
                for(unsigned a=0;a<Mail->count;a++)
                {
                        Mess*mess=Mail->get(a);
                        if(!strcmp(mess->message->to,nick))
                        {
                                pack.type=SERVER;
                                sprintf(str,"сообщение №%d:",tmp);
                                pack.SetFrom(s_name,s_sz);
                                pack.SetData(str,strlen(str));
                                users->Send(id,&pack);
                                pack.type=NEWMAIL;
                                pack.SetFrom(mess->message->from,strlen(mess->message->from));
                                pack.SetData(mess->message->data,mess->message->size);
                                users->Send(id,&pack);
                                tmp++;
                                Mail->free(a);
                                a--;
                        }
                }
                delete [] str;
                return;
        }
        //приватное сообщение
        if(p->type==PRIV)
        {
                int i=users->get_id(p->to);
                if(i>=0)
                {
                        users->Send(i,p);
                }
                return;
        }
        //отклонить
        if(p->type==DEN)
        {
                int i=users->get_id(p->to);
                if(i>=0)
                {
                        users->Send(i,p);
                }
                return;
        }
        //принять
        if(p->type==ACCEPT)
        {
                int i=users->get_id(p->to);
                if(i>=0)
                {
                        users->Send(i,p);
                }
                return;
        }
        //начаол файла
        if(p->type==FOPEN)
        {
                int i=users->get_id(p->to);
                if(i>=0)
                {
                        users->Send(i,p);
                }
                return;
        }
        //данные файла
        if(p->type==FDATA)
        {
                int i=users->get_id(p->to);
                if(i>=0)
                {
                        users->Send(i,p);
                }
                return;
        }
        //конец файла
        if(p->type==FEND)
        {
                int i=users->get_id(p->to);
                if(i>=0)
                {
                        users->Send(i,p);
                }
                return;
        }
        //запрос
        if(p->type==REQ)
        {
                int i=users->get_id(p->to);
                if(i>=0)
                {
                        users->Send(i,p);
                }
                return;
        }
        //ответ OK
        if(p->type==ROK)
        {
                int i=users->get_id(p->to);
                if(i>=0)
                {
                        users->Send(i,p);
                }
                return;
        }
        //ответ NO
        if(p->type==RNO)
        {
                int i=users->get_id(p->to);
                if(i>=0)
                {
                        users->Send(i,p);
                }
                return;
        }
        //регистрация ника
        if(p->type==REGISTER)
        {
                char*nick=t->Nick;
                if(t->oper)
                {
                        nick++;
                }
                if(reg->IsReg(checksum(nick,strlen(nick))))
                {
                        p->type=SERVER;
                        p->SetFrom(s_name,s_sz);
                        char*s=new char[256];
                        sprintf(s,"ваше имя уже зарегистрированно!!! Сначала нужно снять регистрацию!");
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                        delete []s;
                        return;
                }
                reg->addreg(checksum(nick,strlen(nick)),checksum(p->data,strlen(p->data)));
                p->type=SERVER;
                p->SetFrom(s_name,s_sz);
                char*s=new char[256];
                sprintf(s,"Теперь ваш ник зарегистрирован, при каждом входе вам нужно будет ввести пароль...");
                p->SetData(s,strlen(s));
                users->Send(id,p);
                return;
        }
        //идентификация ника
        if(p->type==IDENTIFY)
        {
                char*nick=t->Nick;
                if(t->oper)
                        nick++;
                p->type=SERVER;
                p->SetFrom(s_name,s_sz);
                char *s=new char[256];
                if(reg->IsOk(checksum(nick,strlen(nick)),checksum(p->data,strlen(p->data))))
                {
                        sprintf(s,"Пароль для вашего ника принят");
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                        t->needreg=0;
                        t->reg=0;
                        t->sec=0;
                }else
                {
                        sprintf(s,"Введен не верный пароль");
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                }
                delete [] s;
                return;
        }
        //снять регистрацию с ника
        if(p->type==UNREGISTER)
        {
                char*nick=t->Nick;
                if(t->oper)
                        nick++;
                p->type=SERVER;
                p->SetFrom(s_name,s_sz);
                char *s=new char[256];
                if(reg->IsOk(checksum(nick,strlen(nick)),checksum(p->data,strlen(p->data))))
                {
                        reg->unreg(checksum(nick,strlen(nick)));
                        sprintf(s,"Регистрация с вашего ника снята");
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                }else
                {
                        sprintf(s,"Введен не верный пароль");
                        p->SetData(s,strlen(s));
                        users->Send(id,p);
                }
                delete [] s;
                return;
        }

}
