#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include "ros/ros.h"
#include <ros/package.h>
#include <geometry_msgs/TwistStamped.h>
#include "vcs_agent/vcs.h"
#include "vcs_agent/umsg.h"
#include <regex.h>
#include <math.h>
#include <iostream>
#include <cstring>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "vcs_agent/vcs.h"
#include "vcs_agent/mon.h"
#include "vcs_agent/Message1.h"
#include <geometry_msgs/TwistStamped.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_broadcaster.h>

#define MAX_EVENTS 10 
#define BUF_SIZE 255 
#define IP_ADDR "127.0.0.1"
//#define IP_ADDR "192.168.0.8"
#define STARTUP_DIR "/home/rubicom/catkin_ws/src/vcs_agent/start_up.txt"
#define FINISH_DIR "/home/rubicom/catkin_ws/src/vcs_agent/finish.txt"
using namespace std;
using namespace ros;

namespace vcsAgent
{
    class vAgent
    {
        protected:
            int vcsmd_sd;
            int vcsd_sd;
            ros::Publisher odom_pub;
            ros::Publisher agent_pub;
            ros::Publisher graph_pub;
            ros::Subscriber client_sub; 
            ros::Subscriber vcs_msg_sub;
            ros::NodeHandle nh;

            bool becat = 0;
            double buffer[2];
            double x = 0; 
            double y = 0;
            double th = 0;
            ros::Time stamp;
            double tmp_v = 0;
            double tmp_a = 0;


            void regcomp_all();
            void regfree_all();
            int regexec_with_args(regex_t *regex, char *msg, int ngroups, regmatch_t *groups, char *arg1, char *arg2);
            void chomp(char *msg, int len);
            void _reg_process(char *msg);
            int reg_process(char *msg);
            message parse_handler(char* file_buf);
            void print_string(char *p);
            double convert_mps_to_kmh(double linear_x);
            void VCSstartupCallback(const vcs_agent::Message1::ConstPtr& msg);
            void twistCallback(const geometry_msgs::TwistStampedConstPtr &input_msg);
            void sendtovcs(message *msg);
            void updateOdometry(const double vx, const double ang, const ros::Time &cur_time);
            void processRecvmsg(message *msg);
            double convertSteeringAngleToAngularVelocity(const double cvel,const double cang);
        public:
            int MainLoop();
            vAgent();
            ~vAgent();
    };
}
