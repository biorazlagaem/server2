#ifndef banH
#define banH

#include "stdio.h"


struct Ban
{
        /*ip*/
        unsigned long ip;
        void SetBan(char*IP);
        Ban*next;
        Ban();
        ~Ban();
};

struct Bans
{
        Ban*ban;
        Ban*last;
        unsigned count;
        void add();
        Ban*get(unsigned);
        Ban*get_ip(unsigned long ip);
        int get_id(unsigned long ip);
        void free(unsigned);
        char Load();
        void Save();
        void SetFile(char*);
        char*file;
        Bans();
        /*добавить бан*/
        void add_ban(char*IP);
        void release();
        /*IP забанен?*/
        char IsBanned(unsigned long IP);
        char IsBanned_(char* IP);
        //снять бан
        void unban(char*ip);
        ~Bans();
};

//---------------------------------------------------------------------------
#endif
