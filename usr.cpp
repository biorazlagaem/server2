#pragma hdrstop

#include "usr.h"

#include "mem.h"
#include "code.h"
Usr::Usr()
{
        next=0;
        memset(IP,0,16);
        memset(Nick,0,MAX_NICK_SZ);
        h=INVALID_SOCKET;
        memcpy(Nick,DEF_NICK,strlen(DEF_NICK));
        oper=0;
        needreg=reg=0;
        sec=0;
        ms=0;
        status=U_A;
}
Usr::~Usr()
{
        h=INVALID_SOCKET;
        next=0;
}

Users::Users()
{
        user=last=0;
        count=0;
}

Users::~Users()
{
        Usr*t;
        while(user)
        {
                t=user->next;
                delete user;
                user=t;
        }
        count=0;
}

void Users::add()
{
        Usr*t=new Usr();
        if(!count)
        {
                user=t;
        }else
        {
                last->next=t;
        }
        last =t;
        count++;
}

Usr* Users::get(unsigned id)
{
        if(id>=count)
                return 0;
        if(!id)
        {
                return user;
        }
        Usr*t=user->next;
        for(unsigned a=1;a<id;a++)
                t=t->next;
        return t;
}

Usr* Users::get_n(char*str)
{
        Usr*t=user;
        for(unsigned a=0;a<count;a++)
        {
                if(strcmp(t->Nick,str)==0)
                        return t;
                t=t->next;
        }
        return 0;
}
Usr* Users::get_IP(char*ip)
{
        Usr*t=user;
        for(unsigned a=0;a<count;a++)
        {
                if(strcmp(t->IP,ip)==0)
                        return t;
                t=t->next;
        }
        return 0;
}
int Users::get_id(char*ip)
{
        Usr*t=user;
        for(unsigned a=0;a<count;a++)
        {
                if(strcmp(t->Nick,ip)==0)
                        return a;
                t=t->next;
        }
        return -1;
}

void Users::free(unsigned id)
{
        if(id>=count)
                return;
        Usr*t;
        if(!id)
        {
                t=user->next;
                delete user;
                user=t;
        }else
        {
                t=get(id-1);
                Usr*tmp=t->next;
                t->next=tmp->next;
                if(last==tmp)
                {
                        last=t;
                }
                delete tmp;
        }
        count--;
}

void Users::Send(unsigned id, packet*p )
{
        if(id>=count)
                return;
        Usr*t=get(id);
        #ifdef _DEBUG
                send(t->h,(char*)p,sizeof(packet),0);
        #else
                code((char*)p,sizeof(packet));
                send(t->h,(char*)p,sizeof(packet),0);
                code((char*)p,sizeof(packet));
        #endif
}

void Users::close(unsigned id)
{
        Usr*t=get(id);
        if(t)
        {
                closesocket(t->h);
                free(id);
        }
}

void Users::addusr(char*IP,SOCKET h)
{
        add();
        last->h=h;
        last->SetIP(IP);
        last->SetNick(DEF_NICK);
}
void Users::SendAll(packet*p)
{
        for(unsigned a=0;a<count;a++)
        {
                Send(a,p);
        }
}
int Users::get_h(SOCKET h)
{
        Usr*t=user;
        for(unsigned a=0;a<count;a++)
        {
                if(h==t->h)
                        return a;
                t=t->next;
        }
        return -1;
}

void Usr::SetIP(char*ip)
{
        if(!IP)
                return;
        int sz=strlen(ip);
        if(sz>15)
                sz=15;

        memcpy(IP,ip,sz);
        IP[sz]=0;
}
void Usr::SetNick(char*name)
{
        int sz=strlen(name);
        if(sz>=MAX_NICK_SZ)
                sz=MAX_NICK_SZ-1;
        memset(Nick,0,MAX_NICK_SZ);
        memcpy(Nick,name,sz);
}

unsigned Users::opn()
{
        int ops=0;
        Usr*t=user;
        for(unsigned a=0;a<count;a++)
        {
                if(t->oper)
                {
                        ops++;
                }
                t=t->next;
        }
        return ops;
}
void Users::setop(unsigned id)
{
        Usr*t=get(id);
        if(!t)
                return;
        if(t->oper&&*t->Nick=='@')
                return;
        char*nname=new char[256];
        memset(nname,0,256);
        *nname='@';
        memcpy(nname+1,t->Nick,strlen(t->Nick));
        t->SetNick(nname);
        delete [] nname;
        t->oper=1;
}
void Users::unop(unsigned id)
{
        Usr*t=get(id);
        if(!t)
                return;
        if(!t->oper && *t->Nick!='@')
                return;
        char*nname=new char[256];
        memset(nname,0,256);
        memcpy(nname,t->Nick+1,strlen(t->Nick)-1);
        t->SetNick(nname);
        delete [] nname;
        t->oper=0;
}

void Users::Refresh(unsigned id)
{
        if(id>=count)
                return;
        Usr*t=user;
        packet*p=new packet();
        p->type=CLEAR;
        Send(id,p);
        p->type=LIST;
        for(unsigned a=0;a<count;a++)
        {
                p->SetData(t->Nick,strlen(t->Nick));
                Send(id,p);
                t=t->next;
        }
}
void Users::RefreshAll()
{
        for(unsigned a=0;a<count;a++)
        {
                Refresh(a);
        }
}

void Users::SendOps(packet*p)
{
        Usr*t=user;
        for(unsigned a=0;a<count;a++)
        {
                if(t->oper)
                        SEND(t->h,p);
                t=t->next;
        }
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
