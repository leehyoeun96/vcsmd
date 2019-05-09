#include <stdio.h>
#include <string>
#include <map>
#include <cstring>
#include <iostream>
#include "hybridautomata.h"

using namespace std;

extern void initHybridAutomata_ecat();
extern void operateHybridAutomata_ecat();
extern void add_rtParam(pair<string, string> p, char *name, char *nickname,
                   map<string, double> m, int value, int timestamp, int version, int property);
extern int set_rtParam(char *name, char *field, double val);
extern int get_rtParam_int(char *name, char *field);
extern void set_slave_physical_target_pos(unsigned int slave_no, int value);
extern int get_cfParam(char *name);
extern double get_rtParam_double(char *name, char *field);
extern int get_slave_physical_act_pos(unsigned int slave_no);
extern void initHA_estop();
extern void initHA_selftest();
extern void initHA_pullover();
extern void initHA_homingpedals();
extern void initHA_tuneCC();
extern void initHA_selfdriving();
extern void initHA_avc();
extern void steerControl();

extern void sendDebug(char* ret_msg);
double steer_start_pos=0;
enum
{
    P,
    R,
    N,
    D
};
enum
{
    HOMING,
    OUSTER,
    VELODYNE,
    DRIVING,
    PARKING,
    MAPPING
};


void nameToken(char *result, char *name, char ch)
{
    int i;

    for (i = 0; i < strlen(name); i++)
    {
        if (name[i] == ch)
            break;
    }
    strncpy(result, name, i);
}

int set_slave_virtual_target_pos(unsigned int slave_no)
{
    int pos;
    if ((slave_no < 0) || (slave_no >= get_cfParam("ECAT.num_motors")))
    {
        printf("EC, Wrong slave_no %d\n", slave_no);
        return;
    }
    if (slave_no == get_cfParam("ECAT.motor_id_throttle"))
    {
        pos = (int)(get_rtParam_double("Throttle.target_position", "value") * (double)get_cfParam("Throttle.convert_position"));
        if (pos < get_cfParam("Throttle.min_position") || pos > get_cfParam("Throttle.max_position"))
        {
            printf("EC, Throttle pos : Out of range %d\n",pos);
            return -1;
        }
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_brake"))
    {
        pos = (int)(get_rtParam_double("Brake.target_position", "value") * (double)get_cfParam("Brake.convert_position"));
        if (pos < get_cfParam("Brake.min_position") || pos > get_cfParam("Brake.max_position"))
        {
            printf("EC, Brake pos : Out of range %d\n",pos);
            return -1;
        }
    }
    
    else if (slave_no == get_cfParam("ECAT.motor_id_lidar_center"))
    {
        
        pos = (int)(get_rtParam_double("Lidar.center_target_position", "value") * (double)get_cfParam("Lidar.position_per_degree"));
        if (get_rtParam_double("Lidar.center_target_position", "value") < get_cfParam("Lidar.center_min_degree") || get_rtParam_double("Lidar.center_target_position", "value") > get_cfParam("Lidar.center_max_degree"))
        {
            printf("EC, CLidar pos : Out of range %d\n",pos);
            return -1;
        }
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_lidar_left"))
    {
        pos = (int)(get_rtParam_double("Lidar.left_target_position", "value") * (double)get_cfParam("Lidar.position_per_degree"));
        if (get_rtParam_double("Lidar.left_target_position", "value") < get_cfParam("Lidar.left_min_degree") || get_rtParam_double("Lidar.left_target_position", "value") > get_cfParam("Lidar.left_max_degree"))
        {
            printf("EC, LLidar pos : Out of range %d\n",pos);
            return -1;
        }
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_lidar_right"))
    {
        pos = (int)(get_rtParam_double("Lidar.right_target_position", "value") * (double)get_cfParam("Lidar.position_per_degree"));
        if (get_rtParam_double("Lidar.right_target_position", "value") < get_cfParam("Lidar.right_min_degree") || get_rtParam_double("Lidar.right_target_position", "value") > get_cfParam("Lidar.right_max_degree"))
        {
            printf("EC, RLidar pos : Out of range %d\n",pos);
            return -1;
        }
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_steer"))
    {
        int phy_steer_start_pos = steer_start_pos * (double)get_cfParam("Steerwheel.convert_position");
        pos = (int)(get_rtParam_double("Steerwheel.target_position", "value") * (double)get_cfParam("Steerwheel.convert_position"));
        if (pos < phy_steer_start_pos + get_cfParam("Steerwheel.min_position") || pos > phy_steer_start_pos + get_cfParam("Steerwheel.max_position"))
        {
            printf("EC, Steer pos : Out of range %d\n",pos);
            return -1;
        }
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_gearstick"))
    {
        int mode = get_rtParam_int("Gearstick.target_position", "value");
        switch (mode)
        {
        case P:
            pos = get_cfParam("Gearstick.park_position");
            break;
        case R:
            pos = get_cfParam("Gearstick.reverse_position");
            break;
        case N:
            pos = get_cfParam("Gearstick.neutral_position");
            break;
        case D:
            pos = get_cfParam("Gearstick.drive_position");
            break;
        default:
        {
            printf("EC, Gearstick pos : Out of range %d\n",pos);
            return -1;
            break;
        }
        }
    }
    set_slave_physical_target_pos(slave_no, pos);
    return 1;
}
float get_slave_virtual_target_pos(unsigned int slave_no)
{
    if ((slave_no < 0) || (slave_no >= get_cfParam("ECAT.num_motors")))
    {
        printf("EC, Wrong slave_no %d\n", slave_no);
        return -1;
    }
    if (slave_no == get_cfParam("ECAT.motor_id_throttle"))
    {
        return get_rtParam_double("Throttle.target_position", "value") / (double)get_cfParam("Throttle.convert_position");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_brake"))
    {
        return get_rtParam_double("Brake.target_position", "value") / (double)get_cfParam("Brake.convert_position");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_lidar_center"))
    {
        return get_rtParam_double("Lidar.center_target_position", "value") / (double)get_cfParam("Lidar.position_per_degree");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_lidar_left"))
    {
        return get_rtParam_double("Lidar.left_target_position", "value") / (double)get_cfParam("Lidar.position_per_degree");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_lidar_right"))
    {
        return get_rtParam_double("Lidar.right_target_position", "value") / (double)get_cfParam("Lidar.position_per_degree");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_steer"))
    {
        return get_rtParam_double("Steerwheel.target_position", "value") / (double)get_cfParam("Steerwheel.convert_position");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_gearstick"))
    {
        int pos = get_rtParam_int("Gearstick.target_position", "value");
        if(pos == get_cfParam("Gearstick.park_position"))
        {
            return P;
        }
        else if(pos == get_cfParam("Gearstick.reverse_position"))
        {
            return R;
        }
        else if(pos == get_cfParam("Gearstick.neutral_position"))
        {
            return N;
        }
        else if(pos == get_cfParam("Gearstick.drive_position"))
        {
            return D;
        }
    }
}
float get_slave_virtual_current_pos(unsigned int slave_no)
{
    if ((slave_no < 0) || (slave_no >= get_cfParam("ECAT.num_motors")))
    {
        printf("EC, Wrong slave_no %d\n", slave_no);
        return -1;
    }
    if (slave_no == get_cfParam("ECAT.motor_id_throttle"))
    {
        return (double)get_slave_physical_act_pos(slave_no) / (double)get_cfParam("Throttle.convert_position");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_brake"))
    {
        return (double)get_slave_physical_act_pos(slave_no) / (double)get_cfParam("Brake.convert_position");
    }
    
    else if (slave_no == get_cfParam("ECAT.motor_id_lidar_center"))
    {
        return (double)get_slave_physical_act_pos(slave_no) / (double)get_cfParam("Lidar.position_per_degree");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_lidar_left"))
    {
        return (double)get_slave_physical_act_pos(slave_no) / (double)get_cfParam("Lidar.position_per_degree");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_lidar_right"))
    {
        return (double)get_slave_physical_act_pos(slave_no) / (double)get_cfParam("Lidar.position_per_degree");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_steer"))
    {
        return (double)get_slave_physical_act_pos(slave_no) / (double)get_cfParam("Steerwheel.convert_position");
    }
    else if (slave_no == get_cfParam("ECAT.motor_id_gearstick"))
    {
        int pos = get_slave_physical_act_pos(slave_no);
        if(pos > get_cfParam("Gearstick.park_position") && pos < get_cfParam("Gearstick.reverse_position"))
        {
            return R;
        }
        else if(pos > get_cfParam("Gearstick.reverse_position") && pos < get_cfParam("Gearstick.neutral_position"))
        {
            return N;
        }
        else if(pos > get_cfParam("Gearstick.neutral_position") && pos < get_cfParam("Gearstick.drive_position"))
        {
            return D;
        }
    }
}
 
int throttleModule(char *arg1, double arg2)
{
    double prev_value = 0;
    unsigned int slave_no;
    int result = 1;

    prev_value = get_rtParam_double(arg1, "value");
    slave_no = get_cfParam("ECAT.motor_id_throttle");
    if (set_rtParam(arg1, "value", arg2) == -1)
        return -1;
    if ((result = set_slave_virtual_target_pos(slave_no)) == -1)
    {
        set_rtParam(arg1, "value", prev_value);
        return -1;
    }
    return 1;
}

int brakeModule(char *arg1, double arg2)
{
    double prev_value = 0;
    unsigned int slave_no;
    int result = 1;
    prev_value = get_rtParam_double(arg1, "value");
    slave_no = get_cfParam("ECAT.motor_id_brake");
    if (set_rtParam(arg1, "value", arg2) == -1)
        return -1;
    if ((result = set_slave_virtual_target_pos(slave_no)) == -1)
    {
        set_rtParam(arg1, "value", prev_value);
        return -1;
    }
    return 1;
}
int steerModule(char *arg1, double arg2)
{
    double prev_value = 0;
    unsigned int slave_no;
    int result = 1;
    prev_value = get_rtParam_double(arg1, "value");
    slave_no = get_cfParam("ECAT.motor_id_steer");
    if (get_cfParam("Main.use_can") == 1)
    {
        if (set_rtParam(arg1, "value", arg2+steer_start_pos) == -1)
            return -1;
    }
    else
    {
        if (set_rtParam(arg1, "value", arg2) == -1)
            return -1;
    }
    if ((result = set_slave_virtual_target_pos(slave_no)) == -1)
    {
        set_rtParam(arg1, "value", prev_value);
        return -1;
    }
    return 1;
}
int lidarModule(char *arg1, double arg2)
{
    double prev_value = 0;
    unsigned int slave_no;
    int result = 1;
    if((strcasestr(arg1, "center") != NULL) ||
    (strcasestr(arg1, "ctpos") != NULL))
    {
        prev_value = get_rtParam_double(arg1, "value");
        slave_no = get_cfParam("ECAT.motor_id_lidar_center");
    }
    else if((strcasestr(arg1, "left") != NULL) ||
    (strcasestr(arg1, "ltpos") != NULL))
    {
        prev_value = get_rtParam_double(arg1, "value");
        slave_no = get_cfParam("ECAT.motor_id_lidar_left");
    }
    else if((strcasestr(arg1, "right") != NULL) ||
    (strcasestr(arg1, "rtpos") != NULL))
    {
        prev_value = get_rtParam_double(arg1, "value");
        slave_no = get_cfParam("ECAT.motor_id_lidar_right");
    }
    else
        return -1;

    if (set_rtParam(arg1, "value", arg2) == -1)
        return -1;
    if ((result = set_slave_virtual_target_pos(slave_no)) == -1)
    {
        set_rtParam(arg1, "value", prev_value);
        return -1;
    }
    return 1;
}
int gearstickModule(char *arg1, double arg2)
{
    double prev_value = 0;
    unsigned int slave_no;
    int result = 1;
    prev_value = get_rtParam_double(arg1, "value");
    slave_no = get_cfParam("ECAT.motor_id_gearstick");
    if (set_rtParam(arg1, "value", arg2) == -1)
        return -1;
    if ((result = set_slave_virtual_target_pos(slave_no)) == -1)
    {
        set_rtParam(arg1, "value", prev_value);
        return -1;
    }
}
int poselidarModule(char *arg1, double arg2)
{
    int prev_value = 0;
    int mode = (int)arg2;
    int result = 1;
    prev_value = get_rtParam_int(arg1, "value");
    if (set_rtParam(arg1, "value", arg2) == -1)
        return -1;
    switch (mode)
    {
     
    case HOMING:
    {
        lidarModule("Lidar.center_target_position", get_cfParam("PoseLidar.HomeMode.center_degree"));
        lidarModule("Lidar.left_target_position", get_cfParam("PoseLidar.HomeMode.left_degree"));
        lidarModule("Lidar.right_target_position", get_cfParam("PoseLidar.HomeMode.right_degree"));
        return 1;
    }
    case OUSTER:
    {
        lidarModule("Lidar.center_target_position", (double)get_cfParam("PoseLidar.Ouster.center_degree")/ 1000.0);
        lidarModule("Lidar.left_target_position", (double)get_cfParam("PoseLidar.Ouster.left_degree")/ 1000.0);
        lidarModule("Lidar.right_target_position", (double)get_cfParam("PoseLidar.Ouster.right_degree")/ 1000.0);
        return 1;
    }
    case VELODYNE:
    {
        lidarModule("Lidar.center_target_position", (double)get_cfParam("PoseLidar.Velodyne.center_degree")/ 1000.0);
        lidarModule("Lidar.left_target_position", (double)get_cfParam("PoseLidar.Velodyne.left_degree")/ 1000.0);
        lidarModule("Lidar.right_target_position", (double)get_cfParam("PoseLidar.Velodyne.right_degree")/ 1000.0);
        return 1;
    }
    case MAPPING:
    {
        lidarModule("Lidar.center_target_position", get_cfParam("PoseLidar.MappingMode.center_degree"));
        lidarModule("Lidar.left_target_position", get_cfParam("PoseLidar.MappingMode.left_degree"));
        lidarModule("Lidar.right_target_position", get_cfParam("PoseLidar.MappingMode.right_degree"));
        return 1;
    }
    case DRIVING:
    {
        lidarModule("Lidar.center_target_position", get_cfParam("PoseLidar.DriveMode.center_degree"));
        lidarModule("Lidar.left_target_position", get_cfParam("PoseLidar.DriveMode.left_degree"));
        lidarModule("Lidar.right_target_position", get_cfParam("PoseLidar.DriveMode.right_degree"));
        return 1;
    }
    
    case PARKING:
    {
        lidarModule("Lidar.center_target_position", get_cfParam("PoseLidar.ParkMode.center_degree"));
        lidarModule("Lidar.left_target_position", get_cfParam("PoseLidar.ParkMode.left_degree"));
        lidarModule("Lidar.right_target_position", get_cfParam("PoseLidar.ParkMode.right_degree"));
        return 1;
    }
    
    default:
    {
        printf("poselidar mode error(%d)\n", mode);
        return -1;
    }
    }
}
int CCModule(char *arg1, double arg2)
{
    double prev_value = 0;

    prev_value = get_rtParam_double(arg1, "value");
    if (set_rtParam(arg1, "value", arg2) == -1)
        return -1;
    if(arg2<get_cfParam("CruiseControl.min_target_velocity") || 
    arg2>get_cfParam("CruiseControl.max_target_velocity"))
    {
        set_rtParam(arg1, "value", prev_value);
        return -1;
    }
    return 1;
}
int SCModule(char *arg1, double arg2)
{
    double prev_value = 0;
    int result = 1;

    prev_value = get_rtParam_double(arg1, "value");
    if (set_rtParam(arg1, "value", arg2) == -1)
        return -1;

    //steerControl();
    return 1;
}
int paramHandler(char *arg1, double arg2)
{
    int timestamp = 0;
    char param_name[40];

    memset(param_name, 0, sizeof(param_name));
    nameToken(param_name, arg1, '.');
    if (((strcmp(param_name, "Throttle")) && (strcmp(param_name, "t"))) == 0)
    {
        if (throttleModule(arg1, arg2) == -1)
            return -1;
    }
    else if (((strcmp(param_name, "Brake")) && (strcmp(param_name, "b"))) == 0)
    {
        if (brakeModule(arg1, arg2) == -1)
            return -1;
    }
    else if (((strcmp(param_name, "Steerwheel")) && (strcmp(param_name, "s"))) == 0)
    {
        if (steerModule(arg1, arg2) == -1)
            return -1;
    }
    else if (((strcmp(param_name, "Lidar")) && (strcmp(param_name, "l"))) == 0)
    {
        if (lidarModule(arg1, arg2) == -1)
            return -1;
    }
    else if (((strcmp(param_name, "Gearstick")) && (strcmp(param_name, "g"))) == 0)
    {
        if (gearstickModule(arg1, arg2) == -1)
            return -1;
    }
    else if (((strcmp(param_name, "PoseLidar")) && (strcmp(param_name, "pl"))) == 0)
    {
        if (poselidarModule(arg1, arg2) == -1)
            return -1;
    }
    else if (((strcmp(param_name, "CruiseControl")) && (strcmp(param_name, "cc"))) == 0)
    {
        if (CCModule(arg1,arg2) == -1)
            return -1;
    }
    else if (((strcmp(param_name, "SteerControl")) && (strcmp(param_name, "sc"))) == 0)
    {
        if (SCModule(arg1, arg2) == -1)
            return -1;
    }
    else
    {
        printf("Param name error\n");
        return -1;
    }
    return 1;
}

extern int operateHA_estop();
extern int operateHA_homingpedals();
extern int operateHA_pullover();
extern int operateHA_selftest();
extern int operateHA_tuneCC();
extern int operateHA_selfdriving();
extern int operateHA_avc();



HybridAutomata *cyclic_HA;

enum
{
    EC_START,
    EC_INIT,
    STANDBY,
    FIXSTEER,
    PULLOVER,
    HOMINGPEDALS,
    ESTOP,
    SELFTEST,
    CC,
    SELFDRIVING,
    AVC,
    EC_FINISH
};


extern void reset_pid_values_for_cc();

int standby()
{
    reset_pid_values_for_cc();
  return 1;
}
class STANDBY2STANDBY : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if( (get_rtParam_int("Motion.state", "value") != PULLOVER) &&
        (get_rtParam_int("Motion.state", "value") != FIXSTEER) &&
        (get_rtParam_int("Motion.state", "value") != HOMINGPEDALS) &&
        (get_rtParam_int("Motion.state", "value") != ESTOP) &&
        (get_rtParam_int("Motion.state", "value") != SELFTEST) &&
        (get_rtParam_int("Motion.state", "value") != CC) &&
        (get_rtParam_int("Motion.state", "value") != SELFDRIVING) &&
        (get_rtParam_int("Motion.state", "value") != AVC))
        {
            //cout << "condition : CC_S2S" << endl;
            return true;
        }

        return false;
    }
};


class STANDBY2FIXSTEER : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == FIXSTEER &&
        get_cfParam("ECAT.motor_id_steer") < get_cfParam("ECAT.num_motors"))
        {
            cout << "condition : STANDBY2FIXSTEER" << endl;
            sendDebug("condition : STANDBY2FIXSTEER\n");
            return true;
        }

        return false;
    }
};
class STANDBY2PULLOVER : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == PULLOVER &&
        get_cfParam("ECAT.motor_id_brake") < get_cfParam("ECAT.num_motors"))
        {
            cout << "condition : STANDBY2PULLOVER" << endl;
            sendDebug("condition : STANDBY2PULLOVER\n");
            return true;
        }

        return false;
    }
};
class STANDBY2HOMING : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == HOMINGPEDALS &&
        get_cfParam("ECAT.motor_id_brake") < get_cfParam("ECAT.num_motors"))
        {
            cout << "condition : STANDBY2HOMING" << endl;
            sendDebug("condition : STANDBY2HOMING\n");
            return true;
        }

        return false;
    }
};
class STANDBY2ESTOP : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == ESTOP &&
        get_cfParam("ECAT.motor_id_brake") < get_cfParam("ECAT.num_motors"))
        {
            cout << "condition : STANDBY2ESTOP" << endl;
            sendDebug("condition : STANDBY2ESTOP\n");
            return true;
        }

        return false;
    }
};
class STANDBY2SELFTEST : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == SELFTEST)
        {
            cout << "condition : STANDBY2SELFTEST" << endl;
            sendDebug("condition : STANDBY2SELFTEST\n");
            return true;
        }

        return false;
    }
};
class STANDBY2CC : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == CC &&
        get_cfParam("ECAT.motor_id_brake") < get_cfParam("ECAT.num_motors"))
        {
            cout << "condition : STANDBY2CC" << endl;
            sendDebug("condition : STANDBY2CC\n");
            return true;
        }

        return false;
    }
};
class STANDBY2SELFDRIVING : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == SELFDRIVING &&
        get_cfParam("ECAT.motor_id_steer") < get_cfParam("ECAT.num_motors"))
        {
            cout << "condition : STANDBY2SELFDRIVING" << endl;
            sendDebug("condition : STANDBY2SELFDRIVING\n");
            return true;
        }

        return false;
    }
};
class STANDBY2AVC : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == AVC &&
        get_cfParam("ECAT.motor_id_steer") < get_cfParam("ECAT.num_motors"))
        {
            cout << "condition : STANDBY2AVC" << endl;
            sendDebug("condition : STANDBY2AVC\n");
            return true;
        }

        return false;
    }
};
class FIXSTEER2STANDBY : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if((get_rtParam_double("CAN.steering_angle", "value") < 10) &&
        (get_rtParam_double("CAN.steering_angle", "value") > -10))
        {
            set_rtParam("Motion.state", "value", STANDBY) ;
            cout << "condition : FIXSTEER2STANDBY" << endl;
            sendDebug("condition : FIXSTEER2STANDBY\n");
            return true;
        }

        return false;
    }
};
class PULLOVER2STANDBY : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") != PULLOVER)
        {
            cout << "condition : PULLOVER2STANDBY" << endl;
            sendDebug("condition : PULLOVER2STANDBY\n");
            return true;
        }

        return false;
    }
};
class HOMING2STANDBY : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") != HOMINGPEDALS)
        {
            cout << "condition : HOMING2STANDBY" << endl;
            sendDebug("condition : HOMING2STANDBY\n");
            return true;
        }

        return false;
    }
};
class ESTOP2STANDBY : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") != ESTOP)
        {
            cout << "condition : ESTOP2STANDBY" << endl;
            sendDebug("condition : ESTOP2STANDBY\n");
            return true;
        }

        return false;
    }
};
class SELFTEST2STANDBY : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") != SELFTEST)
        {
            cout << "condition : SELFTEST2STANDBY" << endl;
            sendDebug("condition : SELFTEST2STANDBY\n");
            return true;
        }

        return false;
    }
};
class CC2STANDBY : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") != CC)
        {
            cout << "condition : CC2STANDBY" << endl;
            sendDebug("condition : CC2STANDBY\n");
            return true;
        }

        return false;
    }
};
class SELFDRIVING2STANDBY : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") != SELFDRIVING)
        {
            cout << "condition : SELFDRIVING2STANDBY" << endl;
            sendDebug("condition : SELFDRIVING2STANDBY\n");
            return true;
        }

        return false;
    }
};
class AVC2STANDBY : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") != AVC)
        {
            cout << "condition : AVC2STANDBY" << endl;
            sendDebug("condition : AVC2STANDBY\n");
            return true;
        }

        return false;
    }
};
class FIXSTEER2FIXSTEER : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if((get_rtParam_double("CAN.steering_angle", "value") > 10) ||
        (get_rtParam_double("CAN.steering_angle", "value") < -10))
        {
            //cout << "condition : CC_S2S" << endl;
            return true;
        }

        return false;
    }
};
class PULLOVER2PULLOVER : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == PULLOVER)
        {
            //cout << "condition : CC_S2S" << endl;
            return true;
        }

        return false;
    }
};
class HOMING2HOMING : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == HOMINGPEDALS)
        {
            //cout << "condition : CC_S2S" << endl;
            return true;
        }

        return false;
    }
};
class ESTOP2ESTOP : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == ESTOP)
        {
            //cout << "condition : CC_S2S" << endl;
            return true;
        }

        return false;
    }
};
class SELFTEST2SELFTEST : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == SELFTEST)
        {
            //cout << "condition : CC_S2S" << endl;
            return true;
        }

        return false;
    }
};
class CC2CC : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == CC)
        {
            //cout << "condition : CC_S2S" << endl;
            return true;
        }

        return false;
    }
};
class SELFDRIVING2SELFDRIVING : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == SELFDRIVING)
        {
            //cout << "condition : CC_S2S" << endl;
            return true;
        }

        return false;
    }
};
class AVC2AVC : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if(get_rtParam_int("Motion.state", "value") == AVC)
        {
            //cout << "condition : AVC2AVC" << endl;
            return true;
        }

        return false;
    }
};
int fix_steer()
{

    if (get_cfParam("Main.use_can") == 1)
    {
        double cur_angle = get_rtParam_double("CAN.steering_angle", "value");
        double max_phy_angle = 4680;
        double max_vir_angle = get_cfParam("Steerwheel.virtual_max_position");
        steer_start_pos = -(cur_angle * max_vir_angle / max_phy_angle) + get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_steer"));
	    printf("cur_angle : %lf, max_vir_angle : %lf, max_phy_angle : %lf, current_motor_pos : %lf\n", cur_angle, max_vir_angle,max_phy_angle,get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_steer")));
        steerModule("Steerwheel.target_position", get_cfParam("Steerwheel.start_position"));

        return 1;
    }
    else return -1;
}
int ec_initial()
{
    if (get_cfParam("ECAT.num_motors") > 0)
    {
        throttleModule("Throttle.target_position", get_cfParam("Throttle.virtual_min_position"));
    }
    if (get_cfParam("ECAT.num_motors") > 1)
    {
        brakeModule("Brake.target_position", get_cfParam("Brake.virtual_min_position"));
    }
    if (get_cfParam("ECAT.num_motors") > 2)
    {
        lidarModule("Lidar.center_target_position", get_cfParam("Lidar.center_min_degree"));
    }
    if (get_cfParam("ECAT.num_motors") > 3)
    {
        lidarModule("Lidar.left_target_position", get_cfParam("Lidar.left_min_degree"));
    }
    if (get_cfParam("ECAT.num_motors") > 4)
    {
        lidarModule("Lidar.right_target_position", get_cfParam("Lidar.right_max_degree"));
    }
    if (get_cfParam("ECAT.num_motors") > 5)
    {
        steerModule("Steerwheel.target_position", get_cfParam("Steerwheel.start_position"));
    }
    if (get_cfParam("ECAT.num_motors") > 6)
    {
        gearstickModule("Gearstick.target_position", get_cfParam("Gearstick.park_position"));
    }
    printf("In ec_initial\n");
    return 1;
}

void initHA_controller()
{
  cyclic_HA = new HybridAutomata(EC_START, EC_FINISH);

  cyclic_HA->setState(EC_INIT, ec_initial);
  cyclic_HA->setState(STANDBY, standby);
  cyclic_HA->setState(FIXSTEER, fix_steer);
  cyclic_HA->setState(PULLOVER, operateHA_pullover);
  cyclic_HA->setState(HOMINGPEDALS, operateHA_homingpedals);
  cyclic_HA->setState(ESTOP, operateHA_estop);
  cyclic_HA->setState(SELFTEST, operateHA_selftest);
  cyclic_HA->setState(CC, operateHA_tuneCC);
  cyclic_HA->setState(SELFDRIVING, operateHA_selfdriving);
  cyclic_HA->setState(AVC, operateHA_avc);

  STANDBY2STANDBY *s2s = new STANDBY2STANDBY();
  
  STANDBY2FIXSTEER *s2fix = new STANDBY2FIXSTEER();
  STANDBY2PULLOVER *s2pull = new STANDBY2PULLOVER();
  STANDBY2HOMING *s2home = new STANDBY2HOMING();
  STANDBY2ESTOP *s2estop = new STANDBY2ESTOP();
  STANDBY2SELFTEST *s2self = new STANDBY2SELFTEST();
  STANDBY2CC *s2cc = new STANDBY2CC();
  STANDBY2SELFDRIVING *s2drive = new STANDBY2SELFDRIVING();
  STANDBY2AVC *s2avc = new STANDBY2AVC();

  FIXSTEER2STANDBY *fix2s = new FIXSTEER2STANDBY();
  PULLOVER2STANDBY *pull2s = new PULLOVER2STANDBY();
  HOMING2STANDBY *home2s = new HOMING2STANDBY();
  ESTOP2STANDBY *stop2s = new ESTOP2STANDBY();
  SELFTEST2STANDBY *self2s = new SELFTEST2STANDBY();
  CC2STANDBY *cc2s = new CC2STANDBY();
  SELFDRIVING2STANDBY *drive2s = new SELFDRIVING2STANDBY();
  AVC2STANDBY *avc2s = new AVC2STANDBY();

  FIXSTEER2FIXSTEER *fix2fix = new FIXSTEER2FIXSTEER();
  PULLOVER2PULLOVER *pull2pull = new PULLOVER2PULLOVER();
  HOMING2HOMING *home2home = new HOMING2HOMING();
  ESTOP2ESTOP*stop2stop = new ESTOP2ESTOP();
  SELFTEST2SELFTEST *self2self = new SELFTEST2SELFTEST();
  CC2CC *cc2cc = new CC2CC();
  SELFDRIVING2SELFDRIVING *drive2drive = new SELFDRIVING2SELFDRIVING();
  AVC2AVC *avc2avc = new AVC2AVC();
  

  cyclic_HA->setCondition(EC_START, NULL, EC_INIT);
  cyclic_HA->setCondition(EC_INIT, NULL, STANDBY);
  cyclic_HA->setCondition(STANDBY, s2s, STANDBY);
  cyclic_HA->setCondition(STANDBY, s2fix, FIXSTEER);
  cyclic_HA->setCondition(STANDBY, s2pull, PULLOVER);
  cyclic_HA->setCondition(STANDBY, s2home, HOMINGPEDALS);
  cyclic_HA->setCondition(STANDBY, s2estop, ESTOP);
  cyclic_HA->setCondition(STANDBY, s2self, SELFTEST);
  cyclic_HA->setCondition(STANDBY, s2cc, CC);
  cyclic_HA->setCondition(STANDBY, s2drive, SELFDRIVING);
  cyclic_HA->setCondition(STANDBY, s2avc, AVC);

  cyclic_HA->setCondition(FIXSTEER, fix2s, STANDBY);
  cyclic_HA->setCondition(PULLOVER, pull2s, STANDBY);
  cyclic_HA->setCondition(HOMINGPEDALS, home2s, STANDBY);
  cyclic_HA->setCondition(ESTOP, stop2s, STANDBY);
  cyclic_HA->setCondition(SELFTEST, self2s, STANDBY);
  cyclic_HA->setCondition(CC, cc2s, STANDBY);
  cyclic_HA->setCondition(SELFDRIVING, drive2s, STANDBY);
  cyclic_HA->setCondition(AVC, avc2s, STANDBY);

  cyclic_HA->setCondition(FIXSTEER, fix2fix, FIXSTEER);
  cyclic_HA->setCondition(PULLOVER, pull2pull, PULLOVER);
  cyclic_HA->setCondition(HOMINGPEDALS, home2home, HOMINGPEDALS);
  cyclic_HA->setCondition(ESTOP, stop2stop, ESTOP);
  cyclic_HA->setCondition(SELFTEST, self2self, SELFTEST);
  cyclic_HA->setCondition(CC, cc2cc, CC);
  cyclic_HA->setCondition(SELFDRIVING, drive2drive, SELFDRIVING);
  cyclic_HA->setCondition(AVC, avc2avc, AVC);
  
}

int operateHA_controller()
{
    return cyclic_HA -> operate();
}

int EthercatCyclicHandler(char *arg1, double arg2)
{
    if (strcasestr(arg1, "Pullover") != NULL)
    {
        set_rtParam("Motion.state", "value", PULLOVER);
        return 1;
    }
    else if (strcasestr(arg1, "FixSteer") != NULL)
    {
        set_rtParam("Motion.state", "value", FIXSTEER);
        return 1;
    }
    else if (strcasestr(arg1, "HomingPedals") != NULL)
    {
        set_rtParam("Motion.state", "value", HOMINGPEDALS);
        return 1;
    }
    else if (strcasestr(arg1, "Estop") != NULL)
    {
        set_rtParam("Motion.state", "value", ESTOP);
        return 1;
    }
    else if (strcasestr(arg1, "Selftest") != NULL)
    {
        set_rtParam("Motion.state", "value", SELFTEST);
        return 1;
    }
    else if (strcasestr(arg1, "TuneCruiseControl") != NULL)
    {
        set_rtParam("Motion.state", "value", CC);
        return 1;
    }
    else if (strcasestr(arg1, "SelfDriving") != NULL)
    {
        set_rtParam("Motion.state", "value", SELFDRIVING);
        return 1;
    }
    else if (strcasestr(arg1, "AVC") != NULL)
    {
	    printf("good luck!\n");
        set_rtParam("Motion.state", "value", AVC);
        return 1;
    }
    return -1;
}
map<string, double> Throttle_tpos;
pair<string, string> Throttle_tpos_name;
map<string, double> Brake_tpos;
pair<string, string> Brake_tpos_name;
map<string, double> Steer_tpos;
pair<string, string> Steer_tpos_name;
map<string, double> CLidar_tpos;
pair<string, string> CLidar_tpos_name;
map<string, double> LLidar_tpos;
pair<string, string> LLidar_tpos_name;
map<string, double> RLidar_tpos;
pair<string, string> RLidar_tpos_name;
map<string, double> Gearstick_tpos;
pair<string, string> Gearstick_tpos_name;
map<string, double> Poselidar_mode;
pair<string, string> Poselidar_mode_name;
map<string, double> Motion_state;
pair<string, string> Motion_state_name;
map<string, double> CC_tvel;
pair<string, string> CC_tvel_name;
map<string, double> SC_tanvelo;
pair<string, string> SC_tanvelo_name;
map<string, double> Steer_apos;
pair<string, string> Steer_apos_name;

void initEcatCyclicModules()
{
    add_rtParam(Throttle_tpos_name, "Throttle.target_position","t.tpos", Throttle_tpos, 0, 0, 0, 2);
    add_rtParam(Brake_tpos_name, "Brake.target_position", "b.tpos",Brake_tpos ,0, 0, 0, 2);
    add_rtParam(CLidar_tpos_name, "Lidar.center_target_position", "l.ctpos",CLidar_tpos ,0, 0, 0, 2);
    add_rtParam(LLidar_tpos_name, "Lidar.left_target_position","l.ltpos",LLidar_tpos ,0, 0, 0, 2);
    add_rtParam(RLidar_tpos_name, "Lidar.right_target_position","l.rtpos",RLidar_tpos ,0, 0, 0, 2);
    add_rtParam(Steer_tpos_name, "Steerwheel.target_position", "s.tpos",Steer_tpos ,0, 0, 0, 2);
    add_rtParam(Gearstick_tpos_name, "Gearstick.target_position", "g.tpos",Gearstick_tpos ,0, 0, 0, 2);
    add_rtParam(Poselidar_mode_name, "PoseLidar.mode","pl.mode", Poselidar_mode,0, 0, 0, 2);
    add_rtParam(Motion_state_name, "Motion.state", "Motion.state",Motion_state ,0, 0, 0, 2);
    add_rtParam(CC_tvel_name, "CruiseControl.target_velocity", "cc.tvelo",CC_tvel ,0, 0, 0, 2);
    add_rtParam(SC_tanvelo_name, "SteerControl.target_angular_velocity", "sc.tanvelo", SC_tanvelo,0, 0, 0, 2);
    add_rtParam(Steer_apos_name, "Steerwheel.actual_position","s.apos",Steer_apos ,0, 0, 0, 2);
    initHA_estop();
    initHA_pullover();
    initHA_selftest();
    initHA_homingpedals();
    initHA_tuneCC();
    initHA_selfdriving();
    initHA_controller();
    initHA_avc();
}
