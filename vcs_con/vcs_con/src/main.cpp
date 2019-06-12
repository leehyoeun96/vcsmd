#include "ros/ros.h"
#include "vcs_con/Message.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <strings.h>

using namespace std;
int main(int argc, char** argv)
{
    ros::init(argc,argv,"vcs_con");
    ros::NodeHandle nh;
    vcs_con::Message msg;
    ros::Publisher kbd_pub = nh.advertise<vcs_con::Message>("/vcs_msg",100);
    ros::Rate loop_rate(10);
    string cmd;
    cout<<"hello!"<<endl;
    while(ros::ok())
    {
        getline(cin,cmd);
        msg.command = cmd;
        msg.value = 0;
        kbd_pub.publish(msg);
    }
}
