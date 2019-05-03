#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "ros/ros.h"
#include <geometry_msgs/TwistStamped.h>
#include "vcs_agent/Message1.h"
#include "vcs_agent/vcs.h"
using namespace std;
#define BUF_SIZE 255 

int vcsmd_sd;
int vcsd_sd;
int seq_no=0;
double buffer[2];
typedef struct _message
{
    int seq_no;
    int ack_no;
    int cmd_code;//0 means get / 1 means set
    int param_id;//0 means target velocity/ 1 means target omega//temporally
    double param_val;//used as reply for get
    int result_code;//result of cmd(0 means success)
    string result_msg;//infor/warning/error msg
}message;

double convert_mps_to_kmh(double linear_x)
{
    return linear_x * 3.6;
}
void sendtovcs(const vcs_agent::vcs::ConstPtr &input)
{
    int bytecount;
    message packet;
    packet.seq_no = input->seq_no;
    packet.ack_no = input->ack_no;
    packet.cmd_code = input->cmd_code;
    packet.param_id = input->param_id;
    packet.param_val = input->param_val;
    packet.result_code = input->result_code;
    packet.result_msg = input->result_msg;

    if ((bytecount = send(vcsd_sd, &packet, sizeof(packet), 0)) == -1)
    {   
        fprintf(stderr, "Error sending data : %s\n", strerror(errno));
    }
    printf("send!\n");
}

int main(int argc, char**argv)
{
    ros::init(argc, argv,"vcs_agent");
    ros::NodeHandle nh;

    ros::Subscriber agent_sub = nh.subscribe("/target_val",100,sendtovcs);
    ros::Subscriber agent_sub1 = nh.subscribe("/startup_cmd",100,sendtovcs);

    struct sockaddr_in vcsmd_addr;//sock addr for vcsmd
    struct sockaddr_in vcsd_addr;//sock addr for vcs
   
    //connect to vcsmd//
    vcsmd_sd = socket(PF_INET, SOCK_STREAM, 0);
   
    memset(&vcsmd_addr, 0, sizeof(vcsmd_addr));
    vcsmd_addr.sin_family = PF_INET;
    vcsmd_addr.sin_port = htons(9002);
    inet_aton("127.0.0.1", &vcsmd_addr.sin_addr);
    
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
    inet_aton("127.0.0.1", &vcsd_addr.sin_addr);

    if(connect(vcsd_sd, (struct sockaddr *)&vcsd_addr, sizeof(struct sockaddr_in)) ==-1)
    {
        printf("connect error with vcsd\n");
        return 0;
    }
    
    printf("connected...\n");

    while(ros::ok())
    {
        ros::spinOnce();
    }

    close(vcsd_sd);
}

