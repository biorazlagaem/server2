#ifndef mailH
#define mailH
#include "packet.h"
struct Mess
{
        packet*message;
        Mess*next;
        void SetMess(packet*);
        Mess();
        ~Mess();
};

struct mail
{
        Mess*m;
        Mess*last;
        //количество
        unsigned count;
        char*file;
        char load();
        void save();
        void add();
        Mess*get(unsigned);
        void free(unsigned);
        //количество сообщений для ника
        unsigned getmcount(char*nick);
        //отправить почту нику
        void getmail(SOCKET h,char*);
        //установить файл для сохранения/загрузки почты
        void setfile(char*);
        void release();
        mail();
        ~mail();
};

//---------------------------------------------------------------------------
#endif
