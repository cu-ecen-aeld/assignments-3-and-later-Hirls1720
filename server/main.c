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
volatile bool caught_sig = false;

static void signal_handler ( int signal_number )
{
    // if(access(writefile,F_OK) == 0 ){
    //     printf("File exists, about to remove it\n");
    //     if(!remove(writefile)){
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
bool is_close_socket = true;

int fd2;

char  *data_recv = NULL;

void close_and_free(){
	
	if(sockfd > 0 ){
		close(sockfd);
	}
	if(new_fd > 0 ){
		close(new_fd);
	}
	if(fd2 > 0 ){
		close(fd2);
	}
	if(data_recv ){
		free(data_recv);
	}
    if(caught_sig){
        syslog(LOG_INFO,"Caught signal, exiting");
        printf("Caught signal, exiting\n");
        exit(0);
    }

}

void server_executer(){

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if(getaddrinfo(NULL, "9000", &hints, &res) != 0){
        syslog(LOG_ERR,"Failed to getaddrinfo");
        exit(-1);
    }
    // make a socket:

    sockfd = socket(res->ai_family, res->ai_socktype, 0);
    if(sockfd < 0){
        freeaddrinfo(res);
        syslog(LOG_ERR,"Failed to get socket");
        exit(-1);
    }
 
    // bind it to the port we passed in to getaddrinfo():
   
    if(bind(sockfd, res->ai_addr, res->ai_addrlen) < 0){
        perror("ERROR in bind: ");
        syslog(LOG_ERR,"Failed to bind");
        freeaddrinfo(res);
        close_and_free();
        exit(-1);
    }
    freeaddrinfo(res);

    if(listen(sockfd, BACKLOG) < 0){
        syslog(LOG_ERR,"Failed to listen");
        close_and_free();
        exit(-1);
    }

    fd2 = open(writefile,O_WRONLY|O_CREAT|O_TRUNC,0777);
    ssize_t write_re;
    ssize_t d_read_sz, f_len = 0, prev_len = 0;

    while( !caught_sig ){
        addr_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if(new_fd < 0){
            syslog(LOG_ERR,"Failed to accept");
            close_and_free();
            exit(-1);
        }
        
        char s[addr_size];
        struct sockaddr_in *sin = (struct sockaddr_in *)&their_addr;
        inet_ntop(AF_INET, &(sin->sin_addr), s, addr_size);
        syslog(LOG_INFO,"Accepted connection from %s\n",s);
        printf("Accepted connection from %s\n",s);
    
        bool is_nl_char = false;
        is_close_socket = false;
        char buf[128];

        while( !is_close_socket && !caught_sig ){
            d_read_sz = recv(new_fd, buf, 128, 0); 
            
            if(d_read_sz > 0){
                f_len += d_read_sz;
                data_recv = realloc(data_recv, f_len + 1);

                if(!data_recv){
                    syslog(LOG_ERR,"Failed to realloc data_recv");
                    close_and_free();
                    exit(-1);
                }
                for(int i = prev_len, j = 0; j < d_read_sz; i++, j++){
                
                    data_recv[i] = buf[j];
                    is_nl_char = (buf[j] == '\n');
                }
                prev_len = f_len;

                // right now the data received from client did not have a null terminated char
                data_recv[f_len] = 0;

                write_re = write(fd2, buf, d_read_sz);

                if(is_nl_char){
                
                    //Done receving packet from client, now send it back to the client.
                    if(send(new_fd,data_recv,f_len,0) != f_len){
                        syslog(LOG_ERR,"Failed to send");
                        close_and_free();
                        exit(-1);
                    }
                }
                memset(buf,0,128);
            }
            else {
                syslog(LOG_DEBUG, "Closed connection from %s \n",s);
                is_close_socket = true; 
                close(new_fd);

            }

        }
    
    }
    close_and_free();
}

int main(int argc, char* argv[]){


    bool daemon = false;
    int opt;

     while ((opt = getopt (argc, argv, "d")) != -1){
        switch(opt){
            case 'd':
                daemon = true;
                break;
            case '?':
                return 1;
            default:
                abort();
        }
     }
    openlog(NULL,0,LOG_USER);
    setup_signal();
    if(daemon){
        pid_t pid;

        /* Fork off the parent process */
        pid = fork();

        if(pid < 0){
            syslog(LOG_ERR, "Error while creating a child process");
        }
        else if(pid ==0){
            server_executer();
        }
        else{
            exit(0);
        }
    }
    else{
        server_executer();
    }
    printf("DONE\n");

    return 0;
}
