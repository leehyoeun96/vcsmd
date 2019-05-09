#include <stdio.h>
#include <string>
#include <map>
#include <cstring>
#include <iostream>
//#include "vthread.h"
//#include "hybridautomata.h"

using namespace std;
extern void initEcatCyclicModules();
extern void initHA_ecat();
extern int operateHA_ecat();
extern void add_rtParam(pair<string, string> p, char *name, char *nickname,
                   map<string, double> m, int value, int timestamp, int version, int property);
extern int set_rtParam(char *name, char *field, double val);
extern int paramHandler(char *arg1, double arg2);
extern int EthercatCyclicHandler(char *arg1, double arg2);


map<string, double> Ecat_State;
pair<string, string> Ecat_State_name;

enum
{
    START,
    UP,
    ON,
    OFF,
    DOWN,
    FINISH
};

void initEcatModule()
{
    initHA_ecat();
    add_rtParam(Ecat_State_name, "ECAT.state", "ECAT.state", Ecat_State, 0, 0, 0, 2);
    initEcatCyclicModules();
}
/*void EthercatManageHandler(VThread *t, ThreadMsg *msg)
{
    // message == EC_Motion
    // then EC_motion -> operate();
}*/

int moduleEcatHandler(char *arg1, double arg2)
{
    int post_state = 0;
    if (strcmp(arg1, "up") == 0)
        post_state = UP;
    else if (strcmp(arg1, "on") == 0)
        post_state = ON;
    else if (strcmp(arg1, "off") == 0)
        post_state = OFF;
    else if (strcmp(arg1, "down") == 0)
        post_state = DOWN;
    else
        return -1;

    set_rtParam("ECAT.state", "value", post_state);
    return operateHA_ecat();
}

int EthercatManageHandler(char *module, char *arg1, double arg2)
{

    if (strcmp(module, "Ecat") == 0)
    {
        return moduleEcatHandler(arg1, arg2);
    }
    else if (strcmp(module, "Param") == 0)
    {
        return paramHandler(arg1,arg2);
    }
    else if (strcmp(module, "Motion") == 0)
    {
        return EthercatCyclicHandler(arg1,arg2);
    }
    else if (strcmp(module, "Controller") == 0)
    {
        return EthercatCyclicHandler(arg1,arg2);
    }

    return -1;
}