#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <signal.h>
#include <syslog.h>
#include "umsg.h"

using namespace std;

extern int EthercatManageHandler(char* module, char* arg1, double arg2);
extern int ObdManageHandler(char *value);
extern int CanManageHandler(char *value);
extern double get_rtParam_double(char *name, char *field);
extern int get_cfParam(char *name);
#define MAX_BUFFER 1024
void parseMessage(UserMsg *msg)
{ // NOT COMPLETE!!!
    char arg1[128];
    char arg2[128];
    /* if (regexec_with_args(&regex_ecatoff, msg, 0, NULL, NULL, NULL))
       {
       if (get_cfParam("Main.use_ecat"))
       {
    //EthercatManageHandler("Ecat", "off",NULL);
    goto done;
    }
    //cout << ": ecatoff " << endl;
    goto done;
    }
    else if (regexec_with_args(&regex_get, msg, 2, groups, arg1, NULL))
    {

    if (get_rtParam_double(arg1, "value") != -1)
    {
    cout << "rtParam Matched : " << arg1 << " = " << get_rtParam_double(arg1, "value") << endl;
    goto done;
    }
    if (get_cfParam(arg1) != -1)
    {
    cout << "cfParam Matched : " << arg1 << " = " << get_cfParam(arg1) << endl;
    //cout << ": get " << arg1 << endl;
    goto done;
    }

    goto done;
    }

    else*/ if (msg->param_id == 0)
    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"up");
            case 1:
                strcpy(arg1,"on");
            case 2:
                strcpy(arg1,"off");
            case 3:
                strcpy(arg1,"down");
        }
            syslog(LOG_ERR,"set ecat\n");
        if (get_cfParam("Main.use_ecat"))
        {
            syslog(LOG_ERR,"set ecat\n");
/*            if (EthercatManageHandler("Ecat",arg1,NULL))
                goto done;
            else
            {
                printf("EM Handler,arg1 error : state name error\n");
                goto done;
            }*/
        }
        cout << "check and enable(1) use_ecat in .xml" << endl;
        //cout << ": set : ecat " << arg1 << " " << arg2 << endl;
        goto done;
    }
    else if (msg->param_id == 1)    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"up");
            case 1:
                strcpy(arg1,"on");
            case 2:
                strcpy(arg1,"off");
            case 3:
                strcpy(arg1,"down");
        }   
        if (get_cfParam("Main.use_obd"))
        {
            syslog(LOG_ERR,"set obd\n");
          /*  if (ObdManageHandler(arg1))
                goto done;
            else
            {
                printf("OM Handler,arg1 error : state name error\n");
                goto done;
            }*/
        }
        cout << "check and enable(1) use_obd in .xml" << endl;
        //cout << ": set : obd " << arg1 << " " << arg2 << endl;
        goto done;
    }
    else if (msg->param_id == 2)    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"up");
            case 1:
                strcpy(arg1,"on");
            case 2:
                strcpy(arg1,"off");
            case 3:
                strcpy(arg1,"down");
        }
        if (get_cfParam("Main.use_can"))
        {

            syslog(LOG_ERR,"set can\n");
/*            if (CanManageHandler(arg1)) {
                goto done;
            }


            else
            {
                printf("CM Handler, arg1 error : state name error\n");
                goto done;
            }
*/
        }
        cout << "check and enable(1) use_can in .xml" << endl;
        //cout << ": set : can " << arg1 << " " << arg2 << endl;
        goto done;
    }
    else if (msg->param_id == 3)    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"up");
            case 1:
                strcpy(arg1,"on");
            case 2:
                strcpy(arg1,"off");
            case 3:
                strcpy(arg1,"down");
        }
        if (get_cfParam("Main.use_hvi"))
        {
            syslog(LOG_ERR,"set hvi\n");

            goto done;
        }
        cout << "check and enable(1) use_hvi in .xml" << endl;
        //cout << ": set : hvi " << arg1 << " " << arg2 << endl;
        goto done;
    }

    else if (msg->param_id == 4)    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"Pullover");
            case 1:
                strcpy(arg1,"Homingpedals");
            case 2:
                strcpy(arg1,"Estop");
            case 3:
                strcpy(arg1,"Selftest");
        }
        if (get_cfParam("Main.use_ecat"))
        {
            syslog(LOG_ERR,"set motion\n");
/*            if (EthercatManageHandler("Motion", arg1, atof(arg2)))
                goto done;
            else
            {
                printf("EC Handler, arg1 error : Motion name error\n");
                goto done;
            }*/
        }
        cout << "check and enable(1) use_ecat in .xml" << endl;
        //cout << ": set : controller " << arg1 << " " << arg2 << endl;
        goto done;
    }
    else if (msg->param_id == 5)    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"TuneCruiseControl");
            case 1:
                strcpy(arg1,"SelfDriving");
        }      
        if (get_cfParam("Main.use_ecat"))
        {
            syslog(LOG_ERR,"set contorller\n");
/*            if (EthercatManageHandler("Controller", arg1, atof(arg2)))
                goto done;
            else
            {
                printf("EC Handler, arg1 error : Controller name error\n");
                goto done;
            }*/
        }
        cout << "check and enable(1) use_ecat in .xml" << endl;
        //cout << ": set : controller " << arg1 << " " << arg2 << endl;
        goto done;
    }
    else{
        sprintf(arg1, "%lf", msg->param_val);
        if (get_cfParam("Main.use_ecat"))
        {
            syslog(LOG_ERR,"set param\n");
/*            if (EthercatManageHandler("Param", arg1, atof(arg2)))//?
                goto done;
            else
            {
                printf("EC Handler, arg1 error : Param name error\n");
                goto done;
            }*/
        }
        cout << "check and enable(1) use_ecat in .xml" << endl;
        //cout << ": set : param " << arg1 << " " << arg2 << endl;
        goto done;

    }
    printf("Unknown command\n", msg);

done:
    return;
}

