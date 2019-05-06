#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "ros/ros.h"
#include <geometry_msgs/TwistStamped.h>
#include "vcs_agent/vcs.h"
using namespace std;
#define BUF_SIZE 255 
#define MAX_EVENTS 10 

int vcsmd_sd;
int vcsd_sd;
typedef struct _message
{
    int seq_no;
    int ack_no;
    int cmd_code;//0 means get / 1 means set
    int param_id;//0 means target velocity/ 1 means target omega//temporally
    double param_val;//used as reply for get
    int result_code;//result of cmd(0 means success)
    char result_msg[100];//infor/warning/error msg
}message;

double convert_mps_to_kmh(double linear_x)
{
    return linear_x * 3.6;
}
void sendtovcs(const vcs_agent::vcs::ConstPtr &input)
{
    int bytecount;
//    message *packet = new message;
    message packet;
    char buffer[1000];
     packet.seq_no = input->seq_no;
    packet.ack_no = input->ack_no;
    packet.cmd_code = input->cmd_code;
    packet.param_id = input->param_id;
    packet.param_val = input->param_val;
    packet.result_code = input->result_code;
    strcpy(packet.result_msg, input->result_msg.c_str());
cout << "sendtovcs: seq_no "<< packet.seq_no<<endl;
cout << "sendtovcs: ack_no "<< packet.ack_no<<endl;
cout << "sendtovcs: cmd_code "<< packet.cmd_code<<endl;
cout << "sendtovcs: param_id "<< packet.param_id<<endl;
cout << "sendtovcs: param_val "<< packet.param_val<<endl;
cout << "sendtovcs: result_code "<< packet.result_code<<endl;
cout << "sendtovcs: result_msg " << packet.result_msg<<endl;

    if ((bytecount = send(vcsd_sd,(char*)&packet, sizeof(message), 0)) == -1)
    {   
        fprintf(stderr, "Error sending data : %s\n", strerror(errno));
    }
//    delete packet;
}

int main(int argc, char**argv)
{
    ros::init(argc, argv,"vcs_agent");
    ros::NodeHandle nh;

    ros::Subscriber agent_sub = nh.subscribe("/target_val",100,sendtovcs);
    ros::Subscriber agent_sub1 = nh.subscribe("/startup_cmd",100,sendtovcs);
    ros::Publisher agent_pub = nh.advertise<vcs_agent::vcs>("/vcs_ack", 10);
    vcs_agent::vcs vcs_ack;

    message read_buffer;

    struct sockaddr_in vcsmd_addr;//sock addr for vcsmd
    struct sockaddr_in vcsd_addr;//sock addr for vcs

    //connect to vcsmd//
    vcsmd_sd = socket(PF_INET, SOCK_STREAM, 0);

    memset(&vcsmd_addr, 0, sizeof(vcsmd_addr));
    vcsmd_addr.sin_family = PF_INET;
    vcsmd_addr.sin_port = htons(9002);
    inet_aton("192.168.0.8", &vcsmd_addr.sin_addr);

    if(connect(vcsmd_sd, (struct sockaddr *)&vcsmd_addr, sizeof(vcsmd_addr)) ==-1)
    {
        printf("connect error: %s\n", strerror(errno));
        return 0;
    }
    close(vcsmd_sd);

    //connect to vcsd//
    sleep(1);
    vcsd_sd = socket(PF_INET, SOCK_STREAM, 0);

    memset(&vcsd_addr, 0, sizeof(vcsd_addr));
    vcsd_addr.sin_family = AF_INET;
    vcsd_addr.sin_port = htons(9000);
    inet_aton("192.168.0.8", &vcsd_addr.sin_addr);
    //inet_aton("127.0.0.1", &vcsd_addr.sin_addr);

    if(connect(vcsd_sd, (struct sockaddr *)&vcsd_addr, sizeof(struct sockaddr_in)) ==-1)
    {
        printf("connect error with vcsd\n");
        return 0;
    }

    printf("connected...\n");

    //epoll setting//
    int i;
    int str_len = 0;
    int event_cnt;
    int epfd;
    struct epoll_event ev, evs[MAX_EVENTS];

    epfd=epoll_create1(0);
    ev.events = EPOLLIN;
    ev.data.fd = vcsd_sd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, vcsd_sd, &ev);

    while(ros::ok())
    {
        ros::spinOnce();
        event_cnt= epoll_wait(epfd,evs,MAX_EVENTS,1);
        if(event_cnt ==-1){
            perror("epoll");
            break;
        }
        else if(event_cnt ==0){
            continue;
        }
        for(i=0;i<=event_cnt;i++){
            if(evs[i].events ==EPOLLIN){
                str_len = read(evs[i].data.fd,&read_buffer,sizeof(read_buffer));
                if(str_len==0){
                    epoll_ctl(epfd,EPOLL_CTL_DEL,evs[i].data.fd,NULL);
                    close(evs[i].data.fd);
                    printf("Closed client: %d\n",evs[i].data.fd);
                }
                else{
                    cout << "seq_no "<< read_buffer.seq_no<<endl;
                    cout << "ack_no "<< read_buffer.ack_no<<endl;
                    cout << "cmd_code "<<read_buffer.cmd_code<<endl;
                    cout << "param_id "<< read_buffer.param_id<<endl;
                    cout << "param_val "<< read_buffer.param_val<<endl;
                    cout << "result_code "<< read_buffer.result_code<<endl;
                    cout << "result_msg " << read_buffer.result_msg<<endl;
                    vcs_ack.seq_no = read_buffer.seq_no;
                    vcs_ack.ack_no = read_buffer.ack_no;
                    vcs_ack.cmd_code = read_buffer.cmd_code;
                    vcs_ack.param_id = read_buffer.param_id;
                    vcs_ack.param_val = read_buffer.param_val;
                    vcs_ack.result_code = read_buffer.result_code;
                    vcs_ack.result_msg = read_buffer.result_msg;
                    agent_pub.publish(vcs_ack);

                }
            }
        }
    }
    epoll_ctl(epfd,EPOLL_CTL_DEL,vcsd_sd,NULL);
    close(vcsd_sd);
}
