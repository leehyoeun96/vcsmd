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
#include <geometry_msgs/TwistStamped.h>
#include "vcs_client/autoware.h"
#include "vcs_client/agent.h"
#include "vcs_client/mm.h"
#include <regex.h>
#include <iostream>
#include <cstring>

using namespace std;
void regcomp_all();
void regfree_all();
int regexec_with_args(regex_t *regex, char *msg, int ngroups, regmatch_t *groups, char *arg1, char *arg2);
void chomp(char *msg, int len);
void _reg_process(char *msg);
int reg_process(char *msg);
vcs_client::agent parse_handler(char* file_buf);
void print_string(char *p);
