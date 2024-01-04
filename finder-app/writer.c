#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

int main(int argc, char* argv[]){
    openlog(NULL,0,LOG_USER);
    if(argc == 3){
        char *writefile = argv[1];
        char *writestr = argv[2];
        FILE *fptr = fopen(writefile,"w");

        if(fptr == NULL){
            syslog(LOG_ERR,"File could not be created");
            fclose(fptr);
            return 1;
        } else {
            fwrite(writestr,sizeof(char),sizeof(writestr),fptr);
            syslog(LOG_DEBUG,"Writing %s to %s",writestr,writefile);
            fclose(fptr);
        }

    } else {
        syslog(LOG_ERR,"Invalid arguments");
        return 1;
    }
    return 0;
}
