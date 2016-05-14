#include "windows.h"
#pragma hdrstop

#include "winsock.h"
#include "ban.h"
//заголовок бана
char ban_hdr[]="\12\32\43";

Ban::Ban()
{
        next=0;
        ip=0;
}
Ban::~Ban()
{
        next=0;
        ip=0;
}
/*

        установить бан для IP

*/
void Ban::SetBan(char*IP)
{
        ip=inet_addr(IP);
}

Bans::Bans()
{
        ban=last=0;
        count=0;
        file=0;
}
void Bans::release()
{
        Ban*t;
        while(ban)
        {
                t=ban->next;
                delete ban;
                ban=t;
        }
        count=0;

}
Bans::~Bans()
{
        Save();
        release();
        if(file)
                delete [] file;
}

void Bans::add()
{
        Ban*t=new Ban();
        if(!count)
        {
                ban=t;
        }else
        {
                last->next=t;
        }
        last =t;
        count++;
}

Ban*Bans::get(unsigned id)
{
        if(id>=count)
                return 0;
        if(!id)
        {
                return ban;
        }
        Ban*t=ban->next;
        for(unsigned a=1;a<id;a++)
                t=t->next;
        return t;
}

Ban*Bans::get_ip(unsigned long IP)
{
        Ban*t=ban;
        for(unsigned a=0;a<count;a++)
        {
                if(t->ip==IP)
                        return t;
                t=t->next;
        }
        return 0;
}

int Bans::get_id(unsigned long IP)
{
        Ban*t=ban;
        for(unsigned a=0;a<count;a++)
        {
                if(t->ip==IP)
                        return a;
                t=t->next;
        }
        return -1;
}
void Bans::free(unsigned id)
{
        if(id>=count)
                return;
        Ban*t;
        if(!id)
        {
                t=ban->next;
                delete ban;
                ban=t;
        }else
        {
                t=get(id-1);
                Ban*tmp=t->next;
                t->next=t->next->next;
                if(last==tmp)
                        last=t;
                delete tmp;
        }
        count--;
}

void Bans::SetFile(char*name)
{
        if(file)
                delete [] file;
        file=new char[MAX_PATH];
        memcpy(file,name,strlen(name));
        file[strlen(name)]=0;
}
void Bans::Save()
{
        if(!file)
                return;
        FILE*f=fopen(file,"wb");
        if(!f)
                return;
        fwrite(ban_hdr,3,1,f);
        fwrite(&count,sizeof(count),1,f);
        Ban*t=ban;
        for(unsigned a=0;a<count;a++)
        {
                fwrite(&t->ip,sizeof(unsigned long),1,f);
                t=t->next;
        }
        fclose(f);
}

char Bans::Load()
{
        if(!file)
                return 0;
        unsigned tmp=0;
        FILE*f=fopen(file,"rb");
        if(!f)
                return 0;
        release();
        char hdr[3]={0};
        fread(hdr,1,3,f);
        if(memcmp(hdr,ban_hdr,3))
        {
                fclose(f);
                return 0;
        }
        fread(&tmp,1,sizeof(tmp),f);
        Ban*t;
        for(unsigned a=0;a<tmp;a++)
        {
                add();
                t=last;
                fread(&t->ip,sizeof(unsigned long),1,f);
        }
        fclose(f);
        return 1;
}

void Bans::add_ban(char*IP)
{
        add();
        last->SetBan(IP);
}

char Bans::IsBanned(unsigned long IP)
{
        Ban*t=ban;
        for(unsigned a=0;a<count;a++)
        {
                if(t->ip==IP)
                        return 1;
                t=t->next;
        }
        return 0;
}

char Bans::IsBanned_(char* ip)
{
        Ban*t=ban;
        unsigned long IP=inet_addr(ip);
        for(unsigned a=0;a<count;a++)
        {
                if(t->ip==IP)
                        return 1;
                t=t->next;
        }
        return 0;
}

void Bans::unban(char*ip)
{
        int id=get_id(inet_addr(ip));
        if(id>=0)
        {
                free(id);
        }
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
