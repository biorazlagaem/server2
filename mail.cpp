#pragma hdrstop

#include "mail.h"
#include "stdio.h"
/*заголовок*/
char _hdr[]={13,43,123,124,2};
//длина заголовка 
#define _sz 5

Mess::Mess()
{
        next=0;
        message=new packet();
}
Mess::~Mess()
{
        next=0;
        if(message)
                delete message;
        message=0;
}

void Mess::SetMess(packet*p)
{
        message->SetData(p->data,p->size);
        message->type=p->type;
        message->SetTo(p->to,strlen(p->to));
        message->SetFrom(p->from,strlen(p->from));
}
mail::mail()
{
        file=new char[256];
        memset(file,0,256);
        m=last=0;
        count=0;
}
void mail::release()
{
        Mess*t;
        while(m)
        {
                t=m->next;
                delete m;
                m=t;
        }
        count=0;
}
mail::~mail()
{
        delete [] file;
        release();
}
void mail::setfile(char*n)
{
        if(strlen(n)>255)
                return;
        memcpy(file,n,strlen(n));
}

char mail::load()
{
        if(!file)
        {
                return 0;
        }
        FILE*f=fopen(file,"rb");
        if(!f)
        {
                return 0;
        }
        release();
        char*hdr=new char[_sz];
        fread(hdr,_sz,1,f);
        if(memcmp(hdr,_hdr,_sz))
        {
                delete [] hdr;
                fclose(f);
                return 0;
        }
        unsigned cnt;
        fread(&cnt,sizeof(cnt),1,f);
        for(unsigned a=0;a<cnt;a++)
        {
                add();
                fread(last->message,sizeof(packet),1,f);
        }
        fclose(f);
        delete [] hdr;
        return 1;
}
void mail::save()
{
        if(!file)
                return;
        FILE*f=fopen(file,"wb");
        if(!f)
                return;
        fwrite(_hdr,_sz,1,f);
        fwrite(&count,sizeof(count),1,f);
        for(unsigned a=0;a<count;a++)
        {
                Mess*t=get(a);
                fwrite(t->message,sizeof(packet),1,f);
        }
        fclose(f);
}
void mail::add()
{
        Mess*t=new Mess();
        if(!count)
        {
                m=t;
        }
        else
        {
                last->next=t;
        }
        last =t;
        count++;
}

Mess*mail::get(unsigned id)
{
        if(id>=count)
                return 0;
        if(!id)
                return m;
        Mess*t=m->next;
        for(unsigned a=1;a<id;a++)
        {
                t=t->next;
        }
        return t;
}

void mail::free(unsigned id)
{
        if(id>=count)
                return;
        if(!id)
        {
                Mess*t=m->next;
                delete m;
                m=t;
        }else
        {
                Mess*t=get(id-1),*t1=t->next;
                if(last==t1)
                {
                        last=t;
                }
                t->next=t1->next;
                delete t1;
        }
        count--;
}
unsigned mail::getmcount(char*nick)
{
        if(!nick)
                return 0;
        Mess*t=m;
        unsigned cnt=0;
        for(unsigned a=0;a<count;a++)
        {
                if(!strcmp(t->message->to,nick))
                {
                        cnt++;
                }
                t=t->next;
        }
        return cnt;
}

void mail::getmail(SOCKET h,char*nick)
{
        if((!nick)||(h==INVALID_SOCKET))
                return;
        Mess*t=m;
        for(unsigned a=0;a<count;a++)
        {
                if(!strcmp(t->message->to,nick))
                {
                        SEND(h,t->message);
                }
        }
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
