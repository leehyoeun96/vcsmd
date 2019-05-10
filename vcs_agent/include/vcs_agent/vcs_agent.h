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
#include <iostream>
#include <cstring>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "vcs_agent/vcs.h"
#include "vcs_agent/Message1.h"
using namespace std;
using namespace ros;

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
