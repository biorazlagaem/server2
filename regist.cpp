#pragma hdrstop

#include "regist.h"
#include "stdio.h"
#include "string.h"
#include "mem.h"

#define DWORD unsigned long
//заголовок
char r_hdr[]="\12\5\43\3";
#define H_SZ 4

Reg::Reg()
{
        name=pass=0;
        next=0;
}
Reg::~Reg()
{
        next=0;
}


Regged::Regged()
{
        reg=last=0;
        count=0;
        file=0;
}

void Regged::release()
{
        Reg*t;
        while(reg)
        {
                t=reg->next;
                delete t;
                reg=t;
        }
        count=0;
}

Regged::~Regged()
{
        Save();
        release();
}

void Regged::add()
{
        Reg*t=new Reg();
        if(!count)
        {
                reg=t;
        }else
        {
                last->next=t;
        }
        last=t;
        count++;
}

void Regged::free(unsigned id)
{
        if(id>=count)
                return;
        Reg*t;
        if(!id)
        {
                t=reg->next;
                delete reg;
                reg=t;
        }
        else
        {
                t=get(id-1);
                Reg*tmp=t->next;
                if(last==tmp)
                {
                        last=t;
                }
                t->next=tmp->next;
                delete tmp;
        }
        count--;
}

Reg* Regged::get(unsigned id)
{
        if(id>=count)
                return 0;
        if(!id)
                return reg;
        Reg*t=reg->next;
        for(unsigned a=1;a<id;a++)
                t=t->next;
        return t;
}

char Regged::IsOk(DWORD n,DWORD p)
{
        Reg*t=reg;
        for(unsigned a=0;a<count;a++)
        {
                if(t->name==n && t->pass==p)
                        return 1;
                t=t->next;
        }
        return 0;
}

void Regged::Save()
{
        if(!file)
                return;
        FILE*f=fopen(file,"wb");
        if(!f)
                return;
        fwrite(r_hdr,H_SZ,1,f);
        fwrite(&count,1,sizeof(count),f);
        Reg*t=reg;
        for(unsigned a=0;a<count;a++)
        {
                fwrite(&t->name,sizeof(t->name),1,f);
                fwrite(&t->pass,sizeof(t->pass),1,f);
                t=t->next;
        }
        fclose(f);
}

char Regged::Load()
{
        if(!file)
                return 0;
        FILE*f=fopen(file,"rb");
        if(!f)
                return 0;
        release();
        char hdr[H_SZ]={0};
        fread(hdr,H_SZ,1,f);
        if(memcmp(hdr,r_hdr,H_SZ))
        {
                fclose(f);
                return 0;
        }
        unsigned tmp;
        fread(&tmp,sizeof(tmp),1,f);
        for(unsigned a=0;a<tmp;a++)
        {
                add();
                fread(&last->name,sizeof(last->name),1,f);
                fread(&last->pass,sizeof(last->pass),1,f);        
        }
        fclose(f);
        return 1;

}
void Regged::SetFile(char*name)
{
        if(file)
                delete [] file;
        file=new char[256];
        memcpy(file,name,strlen(name));
        file[strlen(name)]=0;
}

void Regged::addreg(DWORD n,DWORD p)
{
        add();
        last->name=n;
        last->pass=p;
}

char Regged::IsReg(DWORD name)
{
        Reg*t=reg;
        for(unsigned a=0;a<count;a++)
        {
                if(t->name==name)
                        return 1;
                t=t->next;
        }
        return 0;
}
void Regged::unreg(DWORD name)
{
        Reg*t=reg;
        for(unsigned a=0;a<count;a++)
        {
                if(t->name==name)
                {
                        free(a);
                        return;
                }
                t=t->next;
        }
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
