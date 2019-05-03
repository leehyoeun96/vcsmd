#include "vcs_client/vcs_client.h"

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

vcs_client::agent message;
int seqno =0;
#define MAX_BUFFER 1024
void regcomp_all()
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

void regfree_all()
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

int regexec_with_args(regex_t *regex, char *msg, int ngroups, regmatch_t *groups,
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

void chomp(char *msg, int len)
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

void _reg_process(char *msg)
{ // NOT COMPLETE!!!
    regmatch_t groups[3];
    char arg1[128];
    char arg2[128];
    chomp(msg, strlen(msg)); // remove \r\n from message

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
            //EthercatManageHandler("Controller", "HomingPedals", NULL);
        cout << "check and enable(1) use_ecat in .xml" << endl;
        //cout << ": homingpedals " << endl;
        goto done;
    }
    else if (regexec_with_args(&regex_ecatoff, msg, 0, NULL, NULL, NULL))
    {
        //cout << ": ecatoff " << endl;
        goto done;
    }
    else if (regexec_with_args(&regex_get, msg, 2, groups, arg1, NULL))
    {
        goto done;
    }
    
    else if (regexec_with_args(&regex_set_ecat, msg, 2, groups, arg1, NULL))
    {
        message.seq_no = seqno++; 
        message.cmd_code = 1;
        message.param_id = 0;
        if( strcmp(arg1, "up") == 0) message.param_val = 0;
        else if( strcmp(arg1, "on") == 0) message.param_val = 1;
        else if( strcmp(arg1, "off") == 0) message.param_val = 2;
        else if( strcmp(arg1, "down") == 0) message.param_val = 3;
        message.result_msg = arg1;
        goto done;
    }
    else if (regexec_with_args(&regex_set_obd, msg, 2, groups, arg1, NULL))
    {
        message.seq_no = seqno++; 
        message.cmd_code = 1;
        message.param_id = 1;
        if( strcmp(arg1, "up") == 0) message.param_val = 0;
        else if( strcmp(arg1, "on") == 0) message.param_val = 1;
        else if( strcmp(arg1, "off") == 0) message.param_val = 2;
        else if( strcmp(arg1, "down") == 0) message.param_val = 3;
        message.result_msg = arg1;
        goto done;
    }
    else if (regexec_with_args(&regex_set_can, msg, 2, groups, arg1, NULL))
    {
         message.seq_no = seqno++; 
        message.cmd_code = 1;
        message.param_id = 2;
        if( strcmp(arg1, "up") == 0) message.param_val = 0;
        else if( strcmp(arg1, "on") == 0) message.param_val = 1;
        else if( strcmp(arg1, "off") == 0) message.param_val = 2;
        else if( strcmp(arg1, "down") == 0) message.param_val = 3;
        message.result_msg = arg1;
        goto done;
    }
    else if (regexec_with_args(&regex_set_hvi, msg, 2, groups, arg1, NULL))
    {
          message.seq_no = seqno++; 
        message.cmd_code = 1;
        message.param_id = 3;
        if( strcmp(arg1, "up") == 0) message.param_val = 0;
        else if( strcmp(arg1, "on") == 0) message.param_val = 1;
        else if( strcmp(arg1, "off") == 0) message.param_val = 2;
        else if( strcmp(arg1, "down") == 0) message.param_val = 3;
        message.result_msg = arg1;
        goto done;
    }
    
    else if (regexec_with_args(&regex_set_motion, msg, 2, groups, arg1, NULL))
    {
        message.seq_no = seqno++; 
        message.cmd_code = 1;
        message.param_id = 4;
        if( strcmp(arg1, "pullover") == 0) message.param_val = 0;
        else if( strcmp(arg1, "homingpedals") == 0) message.param_val = 1;
        else if( strcmp(arg1, "estop") == 0) message.param_val = 2;
        else if( strcmp(arg1, "selftest") == 0) message.param_val = 3;
        message.result_msg = arg1;
        goto done;
    }
    else if (regexec_with_args(&regex_set_controller, msg, 2, groups, arg1, NULL))
    {
        message.seq_no = seqno++; 
        message.cmd_code = 1;
        message.param_id = 5;
        if( strcmp(arg1, "tunecruisecontrol") == 0) message.param_val = 0;
        else if( strcmp(arg1, "selfdriving") == 0) message.param_val = 1;
        message.result_msg = arg1;
        goto done;
    }
    else if (regexec_with_args(&regex_set, msg, 3, groups, arg1, arg2))
    {
        message.seq_no = seqno++; 
        message.cmd_code = 1;
        if( strcmp(arg1, "throttle.target_position") == 0) message.param_id = 6;
        else if( strcmp(arg1, "brake.target_position") == 0) message.param_id = 7;
        else if( strcmp(arg1, "lidar.center_target_position") == 0) message.param_id = 8;
        else if( strcmp(arg1, "lidar.left_target_position") == 0) message.param_id = 9;
        else if( strcmp(arg1, "lidar.right_target_position") == 0) message.param_id = 10;
        else if( strcmp(arg1, "poselidar.mode") == 0) message.param_id = 11;
        else if( strcmp(arg1, "steerwheel.target_position") == 0) message.param_id = 12;
        else if( strcmp(arg1, "gearstick.target_position") == 0) message.param_id = 13;
        else if( strcmp(arg1, "cruisecontrol.target_velocity") == 0) message.param_id = 14;
        else if( strcmp(arg1, "steercontrol.target_angular_velocity") == 0) message.param_id = 15;
        message.param_val = atof(arg2);
        message.result_msg = arg1;
        goto done;
    }
    printf("Unknown command\n");

done:
    return;
}

int reg_process(char *msg)
{
    _reg_process(msg);
    return 0;
}

void print_string(char *p)
{
    while (*p != 0)
        printf("%u_", (unsigned char)*p++);
    printf("\n");
}
vcs_client::agent parse_handler(char* file_buf)
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
    return message;
}
