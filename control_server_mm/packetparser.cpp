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
int parseMessage(UserMsg *msg)
{ // NOT COMPLETE!!!
    char arg1[128];
    char arg2[128];
    if (msg->param_id == 17)
    {
	    if (get_cfParam("Main.use_ecat"))
	    {
		    if(EthercatManageHandler("Controller", "HomingPedals", NULL) == 1)
			    return 1;
		    else
		    {
			    cout << "HomingPedals error\n";
			    return -1;
		    }

	    }
	    cout << "check and enable(1) use_ecat in .xml" << endl;
	    //cout << ": homingpedals " << endl;                                     
	    return -1;

    }
    else if (msg->param_id == 18)
    {
	    if (get_cfParam("Main.use_ecat"))
	    {
		    if(EthercatManageHandler("Ecat", "off",NULL) == 1)
			    return 1;
		    else
		    {
			    cout << "EcatOff error\n";
			    return -1;
		    }
	    }
	    cout << "check and enable(1) use_ecat in .xml" << endl;
	    return -1;
    }

   /* else if (regexec_with_args(&regex_get, msg, 2, groups, arg1, NULL))
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
*/
    else if (msg->param_id == 0)
    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"up");
                break;
            case 1:
                strcpy(arg1,"on");
                break;
            case 2:
                strcpy(arg1,"off");
                break;
            case 3:
                strcpy(arg1,"down");
                break;
        }
            syslog(LOG_ERR,"set ecat\n");
            
        if (get_cfParam("Main.use_ecat"))
        {
            syslog(LOG_ERR,"set ecat\n");
            if (EthercatManageHandler("Ecat",arg1,NULL) == 1)
                return 1;
            else
            {
                printf("EM Handler,arg1 error : state name error\n");
                return -1;
            }
        }
        cout << "check and enable(1) use_ecat in .xml" << endl;
        //cout << ": set : ecat " << arg1 << " " << arg2 << endl;
        return -1;
    }
    else if (msg->param_id == 1)    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"up");
                break;
            case 1:
                strcpy(arg1,"on");
                break;
            case 2:
                strcpy(arg1,"off");
                break;
            case 3:
                strcpy(arg1,"down");
                break;
        }   
        if (get_cfParam("Main.use_obd"))
        {
            syslog(LOG_ERR,"set obd\n");
            if (ObdManageHandler(arg1))
                return 1;
            else
            {
                printf("OM Handler,arg1 error : state name error\n");
                return -1;
            }
        }
        cout << "check and enable(1) use_obd in .xml" << endl;
        //cout << ": set : obd " << arg1 << " " << arg2 << endl;
        return -1;
    }
    else if (msg->param_id == 2)    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"up");
                break;
            case 1:
                strcpy(arg1,"on");
                break;
            case 2:
                strcpy(arg1,"off");
                break;
            case 3:
                strcpy(arg1,"down");
                break;
        }
        if (get_cfParam("Main.use_can"))
        {

            syslog(LOG_ERR,"set can\n");
            if (CanManageHandler(arg1)) {
                return 1;
            }


            else
            {
                printf("CM Handler, arg1 error : state name error\n");
                return -1;
            }

        }
        cout << "check and enable(1) use_can in .xml" << endl;
        //cout << ": set : can " << arg1 << " " << arg2 << endl;
        return -1;
    }
    else if (msg->param_id == 3)    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"up");
                break;
            case 1:
                strcpy(arg1,"on");
                break;
            case 2:
                strcpy(arg1,"off");
                break;
            case 3:
                strcpy(arg1,"down");
                break;
        }
        if (get_cfParam("Main.use_hvi"))
        {
            syslog(LOG_ERR,"set hvi\n");

            return 1;
        }
        cout << "check and enable(1) use_hvi in .xml" << endl;
        //cout << ": set : hvi " << arg1 << " " << arg2 << endl;
        return -1;
    }

    else if (msg->param_id == 4)    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"Pullover");
                break;
            case 1:
                strcpy(arg1,"Homingpedals");
                break;
            case 2:
                strcpy(arg1,"Estop");
                break;
            case 3:
                strcpy(arg1,"Selftest");
                break;
	    case 4:
		strcpy(arg1,"fixsteer");
		break;
        }
        if (get_cfParam("Main.use_ecat"))
        {
            syslog(LOG_ERR,"set motion\n");
            if (EthercatManageHandler("Motion", arg1, atof(arg2)))
                return 1;
            else
            {
                printf("EC Handler, arg1 error : Motion name error\n");
                return -1;
            }
        }
        cout << "check and enable(1) use_ecat in .xml" << endl;
        //cout << ": set : controller " << arg1 << " " << arg2 << endl;
        return -1;
    }
    else if (msg->param_id == 5)    {
        switch((int)msg->param_val)
        {
            case 0:
                strcpy(arg1,"TuneCruiseControl");
                break;
            case 1:
                strcpy(arg1,"SelfDriving");
                break;
        }      
        if (get_cfParam("Main.use_ecat"))
        {
            syslog(LOG_ERR,"set contorller\n");
            if (EthercatManageHandler("Controller", arg1, atof(arg2)))
                return 1;
            else
            {
                printf("EC Handler, arg1 error : Controller name error\n");
                return -1;
            }
        }
        cout << "check and enable(1) use_ecat in .xml" << endl;
        //cout << ": set : controller " << arg1 << " " << arg2 << endl;
        return -1;
    }
    else{
         switch((int)msg->param_id)
        {
            case 6:
                strcpy(arg1,"t.tpos");
                break;
            case 7:
                strcpy(arg1,"b.tpos");
                break;
            case 8:
                strcpy(arg1,"l.ctpos");
                break;
            case 9:
                strcpy(arg1,"l.ltpos");
                break;
            case 10:
                strcpy(arg1,"l.rtpos");
                break;
            case 11:
                strcpy(arg1,"pl.mode");
                break;
            case 12:
                strcpy(arg1,"s.tpos");
                break;
            case 13:
                strcpy(arg1,"g.tpos");
                break;
            case 14:
                strcpy(arg1,"cc.tvelo");
                break;
            case 15:
                strcpy(arg1,"sc.tanvelo");
                break;
            case 16:
                strcpy(arg1,"hvi.mode");
                break;
        }      
        //sprintf(arg2, "%lf", msg->param_val);
        if (get_cfParam("Main.use_ecat"))
        {
            printf("arg1 : %s\n",arg1);
            printf("param->val : %lf\n",msg->param_val);
            syslog(LOG_ERR,"set param\n");
            if (EthercatManageHandler("Param", arg1, msg->param_val) == 1)
                return 1;
            else
            {
                printf("EC Handler, arg1 error : Param name error\n");
                return -1;
            }
        }
        cout << "check and enable(1) use_ecat in .xml" << endl;
        //cout << ": set : param " << arg1 << " " << arg2 << endl;
        return -1;

    }
    printf("Unknown command\n", msg);
    return -1;
}

