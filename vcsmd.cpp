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

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
sigset_t mask;

/*
   static void err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
    int errno_save, n;
    char buf[MAXLINE];

    errno_save = errno;
#ifdef HAVE_VSNPRINTF
    vsnprintf(buf, sizeof(buf), fmt, ap);
#else
    vsprintf(buf, fmt, ap);
#endif
    n = strlen(buf);
    if(errnoflag)
        snprintf(buf+n, sizeof(buf)-n, ": %s", sterror(errno_save));
    strcat(buf, "\n");
    if(daemon_proc){
    syslog(level, buf);
    }
    else{
        fflush(stdout);
        fputs(buf, stderr);
        fflush(stderr);
    }
    return;
}
*/


int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return(fcntl(fd, F_SETLK, &fl));
}

int already_running(void)
{
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
    if(fd < 0)
    {
        syslog(LOG_ERR, "can't open %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    if(lockfile(fd)<0)
    {
        if(errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return(1);
        }
        syslog(LOG_ERR, "can't lock %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf)+1);
    return(0);
}

void err_quit(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
//    err_doit(0, fmt, ap);
    va_end(ap);
    exit(1);
}

void reread(void) {}

void* thr_fn(void *arg)
{
    int err, signo;
    for(;;)
    {
        err = sigwait(&mask, &signo);
        if( err != 0)
        {
            syslog(LOG_ERR, "sigwait failed");
            exit(1);
        }

        switch(signo){
            case SIGHUP:
                syslog(LOG_INFO, "Re-reading configuration file");
                reread();
                break;
            case SIGTERM:
                syslog(LOG_INFO, "got SIGTERM; exiting");
                exit(0);
            default:
                syslog(LOG_INFO,"unexpected signal %d\n", signo);
        }
    }
    return(0);
}

void daemonize(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    umask(0);

    if(getrlimit(RLIMIT_NOFILE,&rl) < 0)
    {
        printf("%s: can't get file limit", cmd);
        exit(0);
    }

    if((pid = fork()) < 0)
    {
        printf("%s: cant' fork", cmd);
        exit(0);
    }
    else if (pid != 0)
        exit(0);
    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGHUP, &sa, NULL) < 0)
    {
        err_quit("%s: can't ignore SIGHUP", cmd);
        exit(0);
    }
    if((pid = fork()) < 0)
    {
        err_quit("%s: can't fork", cmd);
        exit(0);
    }
    else if(pid != 0)
        exit(0);

    if(chdir("/") < 0)
    {
        err_quit("%s: can't change directory to /", cmd);
        exit(0);
    }

    if(rl.rlim_max = RLIM_INFINITY)
        rl.rlim_max = 1024;
    for(i = 0; i < rl.rlim_max; i++)
        close(i);

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if( fd0 != 0 || fd1 != 1 || fd2 != 2){
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
                fd0, fd1, fd2);
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    int err;
    pthread_t tid;
    char* cmd;
    struct sigaction sa;
    
    if((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else cmd++;

    printf("hello\n");
    daemonize(cmd);

    if(already_running()){
        syslog(LOG_ERR, "daemon already running");
        exit(1);
    }

    sa.sa_handler = SIG_DFL;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGHUP, &sa, NULL) < 0)
        syslog(LOG_ERR, "%s: can't restore SIGHUP default",cmd);
    sigfillset(&mask);
    if((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
        syslog(LOG_ERR, "SIG_BLOCK error");

    err = pthread_create(&tid, NULL, thr_fn, 0);
    if(err != 0)
        syslog(LOG_ERR, "can't create thread");
    
    //////network programming////////
    struct sockaddr_in server_addr, client_addr;
    socklen_t clientlen = sizeof(client_addr);
    char buf[256];
    int sd, ns;

    memset((char *)&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(9000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        syslog(LOG_ERR, "can't create socket");
        exit(1);
    }
    if(bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr))){
        syslog(LOG_ERR, "can't bind socket");
        exit(1);
    }
    if(listen(sd,5)){
        syslog(LOG_ERR, "can't listen");
        exit(1);
    }

    if((ns = accept(sd, (struct sockaddr *)&client_addr, &clientlen)) == -1){
        syslog(LOG_ERR, "accept");
        exit(1);
    }

    //////////////main///////////////
    while(1){
        if(recv(ns, buf, sizeof(buf), 0) > 0){
            syslog(LOG_ERR, "recv str: %s\n", buf);
            exit(1);
        }
    }
    exit(0);
}
