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
#include "vcs_client/Message1.h"
#include "vcs_client/Message2.h"
using namespace std;
#define BUF_SIZE 255 
ros::Subscriber client_sub;
ros::Subscriber vcs_msg_sub; 
ros::Subscriber client_sub1;
ros::Publisher vcs_msg_pub;

vcs_client::Message1 vcs_pub_msg;
vcs_client::Message2 vcs_sub_msg;
int vcsmd_sd;
int vcsd_sd;
int seq_no=0;
double buffer[2];
typedef struct message
{
    int seq_no;
    int ack_no;
    int cmd_code;//0 means get / 1 means set
    int param_id;//0 means target velocity/ 1 means target omega//temporally
    double param_val;//used as reply for get
    int result_code;//result of cmd(0 means success)
    string result_msg;//infor/warning/error msg
} msg;

double convert_mps_to_kmh(double linear_x)
{
    return linear_x * 3.6;
}

void sendmsg()
{
    int bytecount;
    int i=0;
    msg packet;

    printf("target vel : %lf\n",buffer[0]);
    printf("target omega : %lf\n",buffer[1]);
    
    packet.seq_no = seq_no++;
    packet.cmd_code = 1;
    packet.param_id = 0;
    packet.param_val = buffer[0];

    if ((bytecount = send(vcsd_sd, &packet, sizeof(packet), 0)) == -1)
    {   
        fprintf(stderr, "Error sending data %d\n", errno);
    }
    

    if ((bytecount = send(vcsd_sd, buffer, sizeof(buffer), 0)) == -1)
    {   
        fprintf(stderr, "Error sending data %d\n", errno);
    }
    
}
void VCSstartupCallback(const vcs_client::Message2::ConstPtr& msg)
{
    std::string str1 ("start vcs");
    std::string str2 ("pose lidar");
    if((msg->command.compare(str1)) == 0)
    {
        FILE *fp = fopen("/home/rubicom/catkin_ws/src/vcs_client/start_up.txt","r");

        char file_buf[50]={0,};
        if(fp != NULL)
        {
            while(!feof(fp))
            {
                fgets(file_buf, sizeof(file_buf),fp);
                vcs_pub_msg.data = file_buf;
                printf("%s",file_buf);
                //parse_msg(file_buf);
                vcs_msg_pub.publish(vcs_pub_msg);
            }
            fclose(fp);
        }
        else cout << "can't read file" <<endl;
    }
    else if((msg->command.compare(str2)) == 0)
    {
        cout<< msg->value <<endl;
    }
    else 
        cout << "can't parse vcs_startup msg"<<endl;
}
void curVelCallback(const geometry_msgs::TwistStampedConstPtr &msg)
{
    //buffer[2]= convert_mps_to_kmh(msg->twist.linear.x);
}
void twistCallback(const geometry_msgs::TwistStampedConstPtr &input_msg)
{
    buffer[0] = convert_mps_to_kmh(input_msg->twist.linear.x);
    buffer[1] = input_msg->twist.angular.z;
    cout<<buffer[0]<<endl;
    //sendmsg();
}

msg parse_msg(char* buffer)
{
    //NOT COMPLETE
    char command[50]={0,};
    char* ptr;
    msg instr;
    
    strcpy(command,buffer);
    ptr = strtok(buffer, " ");
    if(strcmp(ptr,"get") == 0) instr.cmd_code = 0;
    else if(strcmp(ptr,"set") == 0) instr.cmd_code = 1;

    ptr = strtok(buffer, " ");
    if(strcmp(ptr,"ecat") == 0) instr.param_id = 0;
    else if(strcmp(ptr, "can") == 0) instr.param_id = 1;
    else if(strcmp(ptr, "PoseLidar.mode") == 0) instr.param_id = 2;
    else if(strcmp(ptr, "CruiseControl.target_velocity") == 0) instr.param_id = 3;
    else if(strcmp(ptr, "controller") == 0) instr.param_id = 4;

    ptr = strtok(buffer, " ");
    if(strcmp(ptr,"ecat") == 0) instr.param_id = 0;
    

}

int main(int argc, char**argv)
{
    ros::init(argc, argv,"vcs_client");
    ros::NodeHandle nh;

    client_sub = nh.subscribe("/twist_cmd",1,twistCallback);
    vcs_msg_sub = nh.subscribe("/vcs_msg",1,VCSstartupCallback);
    client_sub1 = nh.subscribe("/current_velocity",1,curVelCallback);
    vcs_msg_pub = nh.advertise<vcs_client::Message1>("/startup_msg",100);
    
   while(ros::ok())
    {
        ros::spinOnce();
    }
    return 0;
}

