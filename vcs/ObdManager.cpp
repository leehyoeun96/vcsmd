#include <stdio.h>
#include <string>
#include <map>
#include <cstring>
#include <iostream>

using namespace std;
extern void initHA_obd();
extern void operateHA_obd();
extern void init_rtParam(map<string, double> v, char *listName, int value, int timestamp, int version, int property);
extern int set_rtParam(char *name, char* field, double val);

map<string, double> Obd_State;
map<string, double> Obd_cvel;

enum 
{
  START,
  UP,
  ON,
  OFF,
  DOWN,
  FINISH
};

void initObdModule()
{
    initHA_obd();
    init_rtParam(Obd_State, "OBD.state",0,0,0,2);
    init_rtParam(Obd_cvel, "OBD.current_velocity",0,0,0,2);
}

int ObdManageHandler(char* value)
{
    int post_state =0;
    static int timestamp=0;
    if(strcmp(value,"up") == 0)
    {
        post_state = UP;
    }
    else if(strcmp(value,"on") == 0)
    {
        post_state = ON;
    }
    else if(strcmp(value,"off") == 0)
    {
        post_state = OFF;
    }
    else if(strcmp(value,"down") == 0)
    {
        post_state = DOWN;
    }
    else 
    {
        
        return -1;
    }

    set_rtParam("OBD.state","value",post_state);
    operateHA_obd();
    return 1;
}