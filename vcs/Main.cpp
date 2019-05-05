#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
//#include "gpionum.h" /////gpio
#include <stdio.h>
#include <iostream>
#include <syslog.h>//added by hyo
#include <fcntl.h>//added by hyo
#include "vthread.h"
#include "socketprogram.h"
#include "hybridautomata.h"
#define LOCKFILE "/var/run/vcsd.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

using namespace std;

extern int readVehicleConfigs(char *xml_file); // from xml.cpp
extern int get_cfParam(char *name);            // from GlobalParam.cpp
extern int set_rtParam(char *name, char *field, double val);
extern int get_rtParam_int(char *name, char *field);
extern double get_rtParam_double(char *name, char *field);


extern void read_thread_handler(void *arg);
extern void ecatOff();
extern void ecatDown();
extern void obdOff();
extern void obdDown();
extern void parseMessage(UserMsg *msg);

pthread_t recv_thread[MAX_CLIENTS];
pthread_t read_thread;
VThread *send_thread[MAX_CLIENTS]={NULL};
//VThread *EM_thread;

bool is_first_connection = true;
int num_of_clients = 0;
int client_idx;
void exitHandler(VThread *t)
{
    cout << t->get_thread_name() << " terminated" << endl;
}
void myFlush()
{
    while (getchar() != '\n')
        ;
}

void countDown(int sec)
{
    for (int i = 0; i < sec; i++)
    {
        sleep(1);
        cout << "countDown to shutDown: " << sec - i << endl;
    }
}
void shutDown()
{
    cout << "shutDown Function" << endl;

    void* retval = 0;
    int idx = 0;

    is_first_connection = true;

    pthread_cancel(read_thread);
    pthread_join(read_thread, &retval);
    if ((int)retval == PTHREAD_CANCELED)
        cout << "read_thread canceled" << endl;
    else
        cout << "read_thread cancellation failed" << endl;

    while (idx < num_of_clients)
    {
        if (recv_thread[idx] != 0)
        {
            pthread_cancel(recv_thread[idx]);
            pthread_join(recv_thread[idx], &retval);
            if ((int)retval == PTHREAD_CANCELED)
                cout << "recv_thread canceled" << endl;
            else
                cout << "recv_thread cancellation failed" << endl;
            recv_thread[idx] = 0;
        }
        ++idx;
    }
    idx = 0;

    while (idx < num_of_clients)
    {
        delete send_thread[idx]; send_thread[idx]=NULL;
        idx++;
    }
   
    //delete parsing_thread;
    //myFlush();
    exit(0);
}

int lockfile(int fd)//added by hyo//
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return(fcntl(fd, F_SETLK, &fl));
}

int already_running(void)//added by hyo//
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


void signalHandler(int signo)
{
    if ((signo = !SIGINT) || (signo = !SIGTERM))
    {
        cout << "unexpected signal = " << signo << " '" << strerror(signo) << "'" << endl;
        exit(0);
    }
    shutDown();
    exit(0);
}
void SendHandler(VThread *t, ThreadMsg *msg)
{
    const UserMsg *packet = static_cast<const UserMsg*>(msg->msg);
    for(int i=0; i<num_of_clients; ++i)
    {
        if(write(clients[i].sockfd, packet, sizeof(*packet)) < 0)
        {
            cout << t->get_thread_name() << ": " << strerror(errno) << endl;
            kill(getpid(), SIGINT);
        }
    }
}
void *RecvHandler(void *arg)
{
    int client_idx = *(int *)arg;
    int sockfd = get_sockfd(client_idx);
    int bytecount = 0;
    double buffer[10];//modified by hyo
    UserMsg *packet = new UserMsg;
//    UserMsg *packet = static_cast<UserMsg*>(&packet);

    printf("sockfd %d\n", sockfd);
    while (1)
    {  
        if ((bytecount = recv(sockfd, packet , sizeof(*packet), MSG_WAITALL)) == -1)//modified by hyo
        {
            fprintf(stderr, "Error receiving data %d\n", errno);
            syslog(LOG_ERR, "Error receiving data %d\n", errno);
        }
        else if (bytecount == 0)
        {
            printf("connection fail\n");
            break;
        }
        syslog(LOG_ERR,"packet->cmd_code :%d", packet->cmd_code);//added by hyo
        printf("packet->cmd_code :%s\n", packet->result_msg);//added by hyo
    
        if(packet->cmd_code)
        {
            parseMessage(packet);
            send_thread[client_idx]->PostMsg((const UserMsg*)packet);
       /*     switch(packet->param_id)
            {
                case 0:
                    set_rtParam("SteerControl.target_angle", "value", packet->param_val);
                    syslog(LOG_ERR,"SteerControl.target_angle");
                    printf("client_idx: %d\n",client_idx);
                    send_thread[client_idx]->PostMsg((const UserMsg*)packet);
                    break;
                case 1:
                    set_rtParam("CruiseControl.target_velocity", "value", packet->param_val);
                    syslog(LOG_ERR,"CruiseControl.target_velocity");
                    break;
                default:
                    syslog(LOG_ERR,"can't set param");
                    break;
            }
        */
        }
    }
    pthread_exit((void *)2);
}
void createThreads(int client_idx)
{
	printf("createThread Func\n");
    if (get_cfParam("Main.use_socket"))
    {
        if (pthread_create(&recv_thread[num_of_clients], NULL, &RecvHandler, (void *)&client_idx) < 0)
        {
            printf("error: pthread_create_recv(): %s\n", strerror(errno));
            shutDown();
        }
            pthread_detach(&recv_thread[num_of_clients]);
    }
    if (is_first_connection)
    {
        if (pthread_create(&read_thread, NULL, &read_thread_handler, (void *)&client_idx) < 0)
        {
            printf("error: pthread_create_read(): %s\n", strerror(errno));
            shutDown();
        }
        if (get_cfParam("Main.use_ecat"))
            //initEcatModule();
        if (get_cfParam("Main.use_obd"))
            //initObdModule();
        if (get_cfParam("Main.use_can"))
            //initCanModule();

        is_first_connection = false;
    }
    ++num_of_clients;
}

int main(int argc, char *argv[])
{
    sigset_t waitmask;
    int c, flag_help = 0;
    struct in_addr server_ip;
    struct sockaddr_in sockaddr;
    char *ipaddr;
    int portno;
    int halfsd, fullsd; // socket descriptors
    int retval;
    //char buffer[4];
    ///added by hyo///
    char* cmd;
    if((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else cmd++;
    openlog(cmd, LOG_CONS, LOG_DAEMON);

    if(already_running()){
        syslog(LOG_ERR, "daemon already running");
        exit(1);
    }

    //------------//
    while ((c = getopt(argc, argv, "hi:p:")) != -1)
    {
        switch (c)
        {
        case 'h':
            flag_help = 1;
            break;
        case 'i':
            //memcpy(ipaddr, optarg, strlen(optarg));
            break;
        case 'c':
            portno = atoi(optarg);
            break;
        default:
            printf("unknown option : %c\n", optopt);
            break;
        }
    }

    if (flag_help == 1)
    {
        printf("usage: %s [-h] [-i ipaddr] [-p portno] \n", argv[0]);
        exit(1);
    }

    readVehicleConfigs("/home/rubicom/vcsd/vcs/vehicle/i30.xml");
    syslog(LOG_ERR,"Connect client? : %d\n", get_cfParam("Main.use_socket"));
    if (signal(SIGINT, signalHandler) == SIG_ERR)
    {
        printf("error: signal(): %s\n", strerror(errno));
        exit(1);
    }
    if (signal(SIGTERM, signalHandler) == SIG_ERR)
    {
        printf("error: signal(): %s\n", strerror(errno));
        exit(1);
    }
    sigemptyset(&waitmask);
    sigaddset(&waitmask, SIGUSR1);
   
    if (get_cfParam("Main.use_socket"))
    {
        server_ip.s_addr = get_cfParam("Main.ip_addr");
        ipaddr = inet_ntoa(server_ip);
        portno = get_cfParam("Main.ip_port");

        printf("server address = %s:%d\n", ipaddr, portno);
        syslog(LOG_ERR, "server address = %s:%d\n", ipaddr, portno);//added by hyo

        if ((halfsd = startupServer(ipaddr, portno)) < 0)
        {
            syslog(LOG_ERR, "startup server() error = %s\n", strerror(errno));//added by hyo
            shutDown();
            exit(1);
        }

        initClients();

        while (1)
        {
            socklen_t len = sizeof(sockaddr);
            fullsd = accept(halfsd, (struct sockaddr *)&sockaddr, &len);
            if (fullsd < 0)
            {
                syslog(LOG_ERR, "error : %s\n", strerror(errno));//added by hyo
                shutDown();
                break;
            }
            syslog(LOG_ERR, "Connected\n");//added by hyo
            printf("Connected\n");

            if (num_of_clients == MAX_CLIENTS)
            {
                printf("error: max clients reached\n");
                close(fullsd);
                sleep(60); //wait for a thread to exit for 1minute
                continue;
            }

            addClient(num_of_clients, fullsd, sockaddr);
            client_idx = findClientByID(num_of_clients);

            createThreads(client_idx);
            printf("main client_idx : %d\n", client_idx);
            send_thread[client_idx] = new VThread("send_thread", client_idx, SendHandler, exitHandler);
            send_thread[client_idx]->CreateThread();
            /*send_thread[num_of_clients] = new VThread("send_thread", num_of_clients, SendHandler, exitHandler);
            send_thread[num_of_clients]->CreateThread();
*/
            //parsing_thread->PostOperationalMsg((const Operational_msg *)umsg);
            //
            /*
            if (pthread_create(&send_thread_id[num_of_clients], NULL, &SendHandler, (void *)&client_idx) < 0)
            {
                printf("error: pthread_create(): %s\n", strerror(errno));
                shutDown();
                break;
            }
            */

            // create the following threads only once --> move the following code outside the while loop

            /*if (pthread_create(&gpio_thread_id, NULL, &GPIO_Input_Handler, (void *)&client_idx) < 0)
                {
                    printf("error: pthread_create(): %s\n", strerror(errno));
                    shutDown();
                    break;
                }
                */
        }
        
        if (findEmptyClient() == -1)
        {
            sigsuspend(&waitmask);
        }
    }

    else
    {
        printf("not connecting Autoware\n");

        //EM_thread = new VThread("EM_thread", num_of_clients, EthercatManageHandler, exitHandler);
        //EM_thread->CreateThread();
        createThreads(client_idx);

        if (findEmptyClient() == -1)
        {
            sigsuspend(&waitmask);
        }
        /*
        char ch;
        while (1)
        {
            ch = getchar();
            ecatUp();
            ch = getchar();
            if (pthread_create(&ecat_cyclic_thread_id, NULL, &cyclicTask, (void *)NULL))
                pthread_detach(ecat_cyclic_thread_id);
            ch = getchar();
            slave_info[0].target_pos = 10000;
      edMsg* threadMsg = new ThreadMsg(MSG_POST_USER_MSG,0,data);
      157     cout<<"threadMsg created"<<endl;
      while (1)
            {
            //
            // printf("4->");
            // printStatus(0);
            // //setagain(0);
            // ch = getchar();
            // ch = getchar();
            // ch = getchar();
        }
        */
    }
    return 0;
}

