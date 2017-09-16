#include <stdarg.h>
#include <string.h>
#include <stdio.h>


void plc_log(const char * msg, ...)
{
    va_list arg;
    char msgstr[256];
    memset(msgstr,0, 256);
    va_start(arg, msg);
    vsprintf(msgstr,msg,arg);
    va_end(arg);
    printf("%s\n",msgstr);
}
