#ifndef registH
#define registH

#include "chksum.h"

struct Reg
{
        /*имя*/
        unsigned name;
        //пароль
        unsigned pass;
        Reg*next;
        Reg();
        ~Reg();
};

struct Regged
{
        Reg*reg;
        Reg*last;
        unsigned count;
        void add();
        void free(unsigned);
        Reg*get(unsigned);
        //проверить имя и пароль на подлинность
        char IsOk(unsigned long name,unsigned long pass);
        //зарегистрирован?
        char IsReg(unsigned long name);
        char*file;
        char Load();
        void Save();
        Regged();
        ~Regged();
        void release();
        void SetFile(char*);
        //добавить регистрацию на ник и пароль
        void addreg(unsigned long,unsigned long);
        //снять регистрацию
        void unreg(unsigned long);
};

//---------------------------------------------------------------------------
#endif
