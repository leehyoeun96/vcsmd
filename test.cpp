#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <syslog.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

using namespace std;
int main()
{
/*    pthread_t pid;
    pid = fork();
    {
        if( pid == -1)
            cout<< "fork fail" << endl;
        else if(pid ==0)
        {
            execl("/bin/ls", "ls", "-l", (char*) 0);
            perror("execl failed to run ls");
        }
        else
        {
        cout<<"hello"<<endl;
        }
        }
        exit(1);
 */
    struct sockaddr_in server_addr, client_addr;
    socklen_t clientlen = sizeof(client_addr);
    char buf[256];
    int halfsd, fullsd;

    memset((char *)&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(9000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if((halfsd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        syslog(LOG_ERR, "can't create socket\n");
        exit(1);
    }
    if(bind(halfsd , (struct sockaddr *)&server_addr, sizeof(server_addr))==-1){
        //syslog(LOG_ERR, "can't bind socket\n");
        syslog(LOG_ERR, "bind error: %s\n", strerror(errno));
        exit(1);
    }
    if(listen(halfsd,5)){
        syslog(LOG_ERR, "can't listen\n");
        exit(1);
    }
    //////network programming end/////
    while(1){
        if((fullsd = accept(halfsd, (struct sockaddr *)&client_addr, (socklen_t *)&clientlen)) == -1){
            syslog(LOG_ERR, "accept fail\n");
            exit(1);
        }
        else 
            syslog(LOG_ERR, "accept success\n");
    }
    close(fullsd);
    close(halfsd);
}
