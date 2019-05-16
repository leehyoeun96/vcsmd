#include "vcs_agent/vcs_agent.h"

// regex with one arg
regex_t regex_help;
regex_t regex_quit;
regex_t regex_show;
regex_t regex_homingpedals;
regex_t regex_ecatoff;

// regex with two args
regex_t regex_get;

// regex with three args
regex_t regex_set; //tvel tangle clidar llidar rlidar gearstick
//steer throttle brake signtower buzzer
regex_t regex_set_ecat;
regex_t regex_set_obd;
regex_t regex_set_can;
regex_t regex_set_hvi;

regex_t regex_set_param;
regex_t regex_set_motion;
regex_t regex_set_controller;

message umsg;
int seqno =0;
#define MAX_BUFFER 1024
using namespace vcsAgent;

void vAgent::regcomp_all()
{ // NOT COMPLETE!!!
    regcomp(&regex_help, "^help$", REG_EXTENDED);
    regcomp(&regex_quit, "^quit$", REG_EXTENDED);
    regcomp(&regex_show, "^show$", REG_EXTENDED);

    regcomp(&regex_homingpedals, "^h$", REG_EXTENDED);
    regcomp(&regex_ecatoff, "^f$", REG_EXTENDED);

    regcomp(&regex_get, "^get ([a-zA-Z0-9_.]{1,40})$", REG_EXTENDED);
    regcomp(&regex_set, "^set ([a-zA-Z0-9_.]{1,40}) ([a-zA-Z0-9_.-]{1,19})$", REG_EXTENDED);

    regcomp(&regex_set_ecat, "^set ecat ([a-zA-Z0-9_]{1,5})$", REG_EXTENDED);
    regcomp(&regex_set_obd, "^set obd ([a-zA-Z0-9_]{1,5})$", REG_EXTENDED);
    regcomp(&regex_set_can, "^set can ([a-zA-Z0-9_]{1,5})$", REG_EXTENDED);
    regcomp(&regex_set_hvi, "^set hvi ([a-zA-Z0-9_]{1,5})$", REG_EXTENDED);

    //regcomp(&regex_set_param, "^set param ([a-zA-Z0-9_.]{1,40}) ([a-zA-Z0-9_.-]{0,19})$", REG_EXTENDED);
    regcomp(&regex_set_motion, "^set motion ([a-zA-Z0-9_.]{1,19})$", REG_EXTENDED);
    regcomp(&regex_set_controller, "^set controller ([a-zA-Z0-9_.]{1,19})$", REG_EXTENDED);
}

void vAgent::regfree_all()
{ // NOT COMPLETE!!!
    regfree(&regex_help);
    regfree(&regex_quit);
    regfree(&regex_show);

    regfree(&regex_homingpedals);
    regfree(&regex_ecatoff);

    regfree(&regex_get);
    regfree(&regex_set);
    regfree(&regex_set_ecat);
    regfree(&regex_set_obd);
    regfree(&regex_set_can);
    regfree(&regex_set_hvi);

    //regfree(&regex_set_param);
    regfree(&regex_set_motion);
    regfree(&regex_set_controller);
}

int vAgent::regexec_with_args(regex_t *regex, char *msg, int ngroups, regmatch_t *groups,
        char *arg1, char *arg2)
{
    int ret = regexec(regex, msg, ngroups, groups, 0);
    if (ret == 0)
    {
        int len;
        if (ngroups > 1)
        {
            len = groups[1].rm_eo - groups[1].rm_so;
            memcpy(arg1, msg + groups[1].rm_so, len);
            arg1[len] = '\0';
        }
        if (ngroups > 2)
        {
            len = groups[2].rm_eo - groups[2].rm_so;
            memcpy(arg2, msg + groups[2].rm_so, len);
            arg2[len] = '\0';
        }
    }
    return !ret;
}

void vAgent::chomp(char *msg, int len)
{
    int nconsumed, nread = len;
    char *npos, *rpos, *cpos = msg;
    while ((npos = (char *)memchr(cpos, '\n', nread)) != NULL)
    {
        *npos = 0; // replace '\n' with NULL
        if ((rpos = (char *)memchr(cpos, '\r', nread)) != NULL)
            *rpos = 0; // replace '\r' with NULL
        nconsumed = npos - cpos + 1;
        nread -= nconsumed;
        cpos = npos + 1;
    }
}

void vAgent::_reg_process(char *msg)
{ // NOT COMPLETE!!!
    regmatch_t groups[3];
    char arg1[128];
    char arg2[128];
    chomp(msg, strlen(msg)); // remove \r\n from msg

    if (regexec_with_args(&regex_help, msg, 0, NULL, NULL, NULL))
    {
        //do_help();
        //cout << ": help" << endl;
        goto done;
    }
    else if (regexec_with_args(&regex_quit, msg, 0, NULL, NULL, NULL))
    {
        //do_quit();-zA-Z0-9_.]{1,19})$", REG_EXTENDED);
        //cout << ": quit" << endl;
        goto done;
    }
    else if (regexec_with_args(&regex_show, msg, 0, NULL, NULL, NULL))
    {
        //do_show();
        //cout << ": show " << endl;
        goto done;
    }
    else if (regexec_with_args(&regex_homingpedals, msg, 0, NULL, NULL, NULL))
    {
        umsg.seq_no = seqno++; 
        umsg.cmd_code = 1;
        umsg.param_id = 0;
        umsg.result_code = 0;
        strcpy(umsg.result_msg, "homingpedals");
        umsg.param_val = 0;
        goto done;
    }
    else if (regexec_with_args(&regex_ecatoff, msg, 0, NULL, NULL, NULL))
    {
        umsg.seq_no = seqno++; 
        umsg.cmd_code = 1;
        umsg.param_id = 1;
        umsg.result_code = 0;
        strcpy(umsg.result_msg, "ecatoff");
        umsg.param_val = 0;
        goto done;
    }
    else if (regexec_with_args(&regex_get, msg, 2, groups, arg1, NULL))
    {
        umsg.seq_no = seqno++; 
        umsg.cmd_code = 0;
        umsg.result_code = 0;
        strcpy(umsg.result_msg, arg1);
        umsg.param_val = 0;
        goto done;
    }

    else if (regexec_with_args(&regex_set_ecat, msg, 2, groups, arg1, NULL))
    {
        umsg.seq_no = seqno++; 
        umsg.cmd_code = 1;
        umsg.param_id = 2;
        umsg.result_code = 0;
        strcpy(umsg.result_msg, arg1);
        if( strcasecmp(arg1, "up") == 0) umsg.param_val = 0;
        else if( strcasecmp(arg1, "on") == 0) umsg.param_val = 1;
        else if( strcasecmp(arg1, "off") == 0) umsg.param_val = 2;
        else if( strcasecmp(arg1, "down") == 0) umsg.param_val = 3;
        else {
            umsg.result_code = -1;
            printf("Unknown command\n");
        }
        goto done;
    }
    else if (regexec_with_args(&regex_set_obd, msg, 2, groups, arg1, NULL))
    {
        umsg.seq_no = seqno++; 
        umsg.cmd_code = 1;
        umsg.param_id = 3;
        umsg.result_code = 0;
        strcpy(umsg.result_msg, arg1);
        if( strcasecmp(arg1, "up") == 0) umsg.param_val = 0;
        else if( strcasecmp(arg1, "on") == 0) umsg.param_val = 1;
        else if( strcasecmp(arg1, "off") == 0) umsg.param_val = 2;
        else if( strcasecmp(arg1, "down") == 0) umsg.param_val = 3;
        else {
            umsg.result_code = -1;
            printf("Unknown command\n");
        }
        goto done;
    }
    else if (regexec_with_args(&regex_set_can, msg, 2, groups, arg1, NULL))
    {
        umsg.seq_no = seqno++; 
        umsg.cmd_code = 1;
        umsg.param_id = 4;
        umsg.result_code = 0;
        strcpy(umsg.result_msg, arg1);
        if( strcasecmp(arg1, "up") == 0) umsg.param_val = 0;
        else if( strcasecmp(arg1, "on") == 0) umsg.param_val = 1;
        else if( strcasecmp(arg1, "off") == 0) umsg.param_val = 2;
        else if( strcasecmp(arg1, "down") == 0) umsg.param_val = 3;
        else {
            umsg.result_code = -1;
            printf("Unknown command\n");
        }
        goto done;
    }
    else if (regexec_with_args(&regex_set_hvi, msg, 2, groups, arg1, NULL))
    {
        umsg.seq_no = seqno++; 
        umsg.cmd_code = 1;
        umsg.param_id = 5;
        umsg.result_code = 0;
        strcpy(umsg.result_msg, arg1);
        if( strcasecmp(arg1, "up") == 0) umsg.param_val = 0;
        else if( strcasecmp(arg1, "on") == 0) umsg.param_val = 1;
        else if( strcasecmp(arg1, "off") == 0) umsg.param_val = 2;
        else if( strcasecmp(arg1, "down") == 0) umsg.param_val = 3;
        else {
            umsg.result_code = -1;
            printf("Unknown command\n");
        }
        goto done;
    }

    else if (regexec_with_args(&regex_set_motion, msg, 2, groups, arg1, NULL))
    {
        umsg.seq_no = seqno++; 
        umsg.cmd_code = 1;
        umsg.param_id = 6;
        umsg.result_code = 0;
        strcpy(umsg.result_msg, arg1);
        if( strcasecmp(arg1, "pullover") == 0) umsg.param_val = 0;
        else if( strcasecmp(arg1, "homingpedals") == 0) umsg.param_val = 1;
        else if( strcasecmp(arg1, "estop") == 0) umsg.param_val = 2;
        else if( strcasecmp(arg1, "selftest") == 0) umsg.param_val = 3;
        else if( strcasecmp(arg1, "fixsteer") == 0) umsg.param_val = 4;
        else if( strcasecmp(arg1, "ready2start") == 0) umsg.param_val = 5;
        else {
            umsg.result_code = -1;
            printf("Unknown command\n");
        }
        goto done;
    }
    else if (regexec_with_args(&regex_set_controller, msg, 2, groups, arg1, NULL))
    {
        umsg.seq_no = seqno++; 
        umsg.cmd_code = 1;
        umsg.param_id = 7;
        umsg.result_code = 0;
        strcpy(umsg.result_msg, arg1);
        if( strcasecmp(arg1, "tunecruisecontrol") == 0) umsg.param_val = 0;
        else if( strcasecmp(arg1, "selfdriving") == 0) umsg.param_val = 1;
        else if( strcasecmp(arg1, "avc") == 0) umsg.param_val = 2;
        else {
            umsg.result_code = -1;
            printf("Unknown command\n");
        }
        goto done;
    }
    else if (regexec_with_args(&regex_set, msg, 3, groups, arg1, arg2))
    {
        umsg.seq_no = seqno++; 
        umsg.cmd_code = 1;
        umsg.result_code = 0;
        strcpy(umsg.result_msg, arg1);
        umsg.param_val = atof(arg2);
        if( (strcasecmp(arg1, "throttle.target_position") == 0) |  (strcasecmp(arg1, "t.tpos") == 0) ) umsg.param_id = 8;
        else if( strcasecmp(arg1, "brake.target_position") == 0 | (strcasecmp(arg1, "b.tpos") == 0) ) umsg.param_id = 9;
        else if( strcasecmp(arg1, "lidar.center_target_position") == 0  | (strcasecmp(arg1, "l.ctpos") == 0)) umsg.param_id = 10;
        else if( strcasecmp(arg1, "lidar.left_target_position") == 0 | (strcasecmp(arg1, "l.ltpos") == 0) )umsg.param_id = 11;
        else if( strcasecmp(arg1, "lidar.right_target_position") == 0 | (strcasecmp(arg1, "l.rtpos") == 0) )umsg.param_id = 12;
        else if( strcasecmp(arg1, "poselidar.mode") == 0 | (strcasecmp(arg1, "pl.mode") == 0)) umsg.param_id = 13;
        else if( strcasecmp(arg1, "steerwheel.target_position") == 0 | (strcasecmp(arg1, "s.tpos") == 0)) umsg.param_id = 14;
        else if( strcasecmp(arg1, "gearstick.target_position") == 0 | (strcasecmp(arg1, "g.tpos") == 0)) umsg.param_id = 15;
        else if( strcasecmp(arg1, "cruisecontrol.target_velocity") == 0 | (strcasecmp(arg1, "cc.tvelo") == 0)) umsg.param_id = 16;
        else if( strcasecmp(arg1, "steercontrol.target_angular_velocity") == 0 | (strcasecmp(arg1, "sc.tanvelo") == 0)) umsg.param_id = 17;
        else if( strcasecmp(arg1, "hvi.mode") == 0) umsg.param_id = 18;
        else if( strcasecmp(arg1, "CAN.pub2agent") == 0) umsg.param_id = 19;
        else {
            umsg.result_code = -1;
            printf("Unknown command\n");
        } 
        goto done;
    }
    printf("Unknown command\n");
    umsg.result_code = -1;

done:
    return;
}

int vAgent::reg_process(char *msg)
{
    _reg_process(msg);
    return 0;
}

void vAgent::print_string(char *p)
{
    while (*p != 0)
        printf("%u_", (unsigned char)*p++);
    printf("\n");
}
message vAgent::parse_handler(char* file_buf)
{
    char buf[MAX_BUFFER], buffer[MAX_BUFFER];
    int nread, nconsumed, totread = 0;
    char *cpos, *npos, *rpos;

    memset(buffer, 0, sizeof(buffer));

    strcpy(buf, file_buf);
    if (sizeof(buffer) - strlen(buffer) <= strlen(buf))
    {
        printf("read_thread: too small buffer\n");
        kill(getpid(), SIGINT);
    }

    strcat(buffer, buf);
    totread += strlen(file_buf);
    cpos = buffer;
    while ((npos = (char *)memchr(cpos, '\n', totread)) != NULL)
    {
        *npos = 0; // replace '\n' with NULL
        if ((rpos = (char *)memchr(cpos, '\r', totread)) != NULL)
            *rpos = 0; // replace '\r' with NULL
        if (reg_process(cpos) != 0)
            printf("reg_process fail");
        printf("parse_handler: %s\n", cpos);
    }
    return umsg;
}
