#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


#define BACKLOG 10

char *writefile = "/var/tmp/aesdsocketdata";
char *writefile_2 = "/var/tmp/aesdsocketdata_2";
volatile bool caught_sig = false;

static void signal_handler ( int signal_number )
{
    /**
    * Save a copy of errno so we can restore it later.  See https://pubs.opengroup.org/onlinepubs/9699919799/
    * "Operations which obtain the value of errno and operations which assign a value to errno shall be
    *  async-signal-safe, provided that the signal-catching function saves the value of errno upon entry and
    *  restores it before it returns."
    */

    syslog(LOG_INFO,"Caught signal, exiting");
    printf("Caught signal, exiting\n");

    // if(access(writefile,F_OK) == 0 ){
    //     printf("File exists, about to remove it\n");
    //     if(remove(writefile)){
    //         printf("successfully remove writefile\n");
    //     }
    //     else {
    //         printf("unable remove writefile\n");
    //     }
    // }
    // else {
    //     printf("File is not exist yet\n");
    // }

    caught_sig = true;
}

bool setup_signal(){

    struct sigaction new_action;
    bool success = true;
    memset(&new_action,0,sizeof(struct sigaction));
    new_action.sa_handler=signal_handler;
    if( sigaction(SIGTERM, &new_action, NULL) != 0 ) {
        printf("Error %d (%s) registering for SIGTERM",errno,strerror(errno));
        return false;
    }
    if( sigaction(SIGINT, &new_action, NULL) ) {
        printf("Error %d (%s) registering for SIGINT",errno,strerror(errno));
        return false;
    }
    return true;
}

struct sockaddr_storage their_addr;
socklen_t addr_size;
struct addrinfo hints, *res;
int sockfd, new_fd;
char *s = NULL;

int fd2;

char  *data_recv = NULL;

void close_and_free(){
	
	if(sockfd > 0){
		close(sockfd);
	}
	if(new_fd > 0){
		close(new_fd);
	}
	if(fd2 > 0){
		close(fd2);
	}
	if(s){
		free(s);
	}
	if(data_recv){
		free(data_recv);
	}

}

int main(int argc, char* argv[]){

    openlog(NULL,0,LOG_USER);
    setup_signal();

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if(getaddrinfo(NULL, "9000", &hints, &res) != 0){
        printf("getaddinfor failed");
        return -1;
    }
    // make a socket:

    sockfd = socket(res->ai_family, res->ai_socktype, 0);
    if(sockfd < 0){
        freeaddrinfo(res);
        close(sockfd);
        printf("socket failed");
        return -1;
    }
 
    // bind it to the port we passed in to getaddrinfo():
   
    if(bind(sockfd, res->ai_addr, res->ai_addrlen) < 0){
        printf("bind failed\n");
        perror("ERROR in bind: ");
        freeaddrinfo(res);
        close(sockfd);
        return -1 ;
    }
    freeaddrinfo(res);

    if(listen(sockfd, BACKLOG) < 0){
        printf("listen failed");
        close(sockfd);
        return -1;
    }
    // fd = open(writefile,O_WRONLY|O_CREAT|O_TRUNC,0777);
    // fd2 = open(writefile_2,O_WRONLY|O_CREAT|O_TRUNC,0777);
    fd2 = open(writefile,O_WRONLY|O_CREAT|O_TRUNC,0777);
    ssize_t write_re;

    ssize_t d_read_sz, f_len = 0, prev_len = 0;
    while( !caught_sig ){
        printf("In the first while\n");
        addr_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if(new_fd < 0){
            printf("accept failed\n");
	    free(data_recv);
	    close(fd2);
	    close(new_fd);
            close(sockfd);
            return -1;
        }
        s = malloc(addr_size);

        struct sockaddr_in *sin = (struct sockaddr_in *)&their_addr;
        inet_ntop(AF_INET, &(sin->sin_addr), s, addr_size);
        syslog(LOG_INFO,"Accepted connection from %s\n",s);
        printf("Accepted connection from %s\n",s);
    

        
        
        bool is_nl_char = false;
        bool is_close_socket = false;

        char buf[128];

        printf("caught sis = %d\n",caught_sig);

        while( !is_close_socket && !caught_sig ){
            printf("In the second while\n");
            d_read_sz = recv(new_fd, buf, 128, 0); 
            
            if(d_read_sz > 0){
                printf("sucessfully recv \n");
                f_len += d_read_sz;
                printf("f_len =  %ld \n",f_len);
                data_recv = realloc(data_recv, f_len + 1);

                if(!data_recv){
                
                    syslog(LOG_ERR,"Failed to realloc data_recv");
                    printf("Failed to realloc data_recv\n");
                    free(data_recv);
                    free(s);
                    close(new_fd);
                    close(fd2);
                    close(sockfd);
                    return -1;
                }
                printf("buf recv is: %s",buf);
                for(int i = prev_len, j = 0; j < d_read_sz; i++, j++){
                
                    data_recv[i] = buf[j];
                    is_nl_char = (buf[j] == '\n');
                }
                prev_len = f_len;

                // right now the data received from client did not have a null terminated char
                data_recv[f_len] = 0;
                printf("sz of data_recv: %ld\n", strlen(data_recv));

                printf("data_recv is: %s\n",data_recv);

                write_re = write(fd2, buf, d_read_sz);

                printf("write return %ld\n",write_re);
                perror("Error: \n");

                if(is_nl_char){
                
                    //Done receving packet from client, now send it back to the client.
                    printf("about to send %s\n",data_recv);
                    if(send(new_fd,data_recv,f_len,0) != f_len){
                        printf("fail to send \n");
                        free(data_recv);
                        free(s);
                        close(new_fd);
                        close(fd2);
                        close(sockfd);
                        return -1;
                    }
                    perror("Error of send: \n");


                }
                memset(buf,0,128);


            }
            else {
                printf("about to close socket after send full len back to clint \n");
                syslog(LOG_DEBUG, "Closed connection from %s \n",s);
                is_close_socket = true; 
                // free(data_recv);
                free(s);
                close(new_fd);
                //close(fd);
            }

        }
    
    }
    printf("about to close socket\n");
    close(fd2);
    close(sockfd);

    return 0;
}
