#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
    openlog(NULL,0,LOG_USER);
    if(argc == 3){
        char *writefile = argv[1];
        char *writestr = argv[2];
        int fd = open(writefile,O_WRONLY|O_CREAT,0777);

        if(fd == -1){
            printf("fail to open file");
            syslog(LOG_ERR,"File could not be created");
            close(fd);
            return 1;
        } else {
            printf("success to open file");
            write(fd,writestr,strlen(writestr));
            syslog(LOG_DEBUG,"Writing %s to %s",writestr,writefile);
            close(fd);
        }

    } else {
        syslog(LOG_ERR,"Invalid arguments");
        return 1;
    }
    return 0;
}
