#include <stdio.h>
#include <string>
#include <map>
#include <iostream>
#include "hybridautomata.h"

HybridAutomata *estop_HA;
HybridAutomata *pullover_HA;
HybridAutomata *homingpedals_HA;
HybridAutomata *selftest_HA;

extern int get_cfParam(char *name);
extern float get_slave_virtual_current_pos(unsigned int slave_no);
extern int brakeModule(char *arg1, double arg2);
extern int throttleModule(char *arg1, double arg2);
extern int lidarModule(char *arg1, double arg2);
extern int steerModule(char *arg1, double arg2);
extern int gearstickModule(char *arg1, double arg2);
extern int get_slave_physical_act_pos(unsigned int slave_no);
extern int get_slave_physical_target_pos(unsigned int slave_no);
extern double get_rtParam_double(char *name, char *field);
extern int set_rtParam(char *name, char *field, double val);

enum
{
    EC_START,
    STANDBY,
    PULLOVER,
    HOMINGPEDALS,
    ESTOP,
    SELFTEST,
    CC,
    SELFDRIVING,
    EC_FINISH
};
////////////////////////////////////////////////////////////////////////
///////////////////////////HomingPedals Module//////////////////////////
////////////////////////////////////////////////////////////////////////
enum
{
    HP_START,
    HP_THROTTLE,
    HP_BRAKE,
    HP_FINISH
};

void homingThrottle()
{
   
    throttleModule("Throttle.target_position", get_cfParam("Throttle.virtual_min_position"));
}
void homingBrake()
{
    brakeModule("Brake.target_position", get_cfParam("Brake.virtual_min_position"));
}

void homingFinish()
{
    printf("HOMING!!!\n");
    set_rtParam("Motion.state", "value",STANDBY);
    return;
}
void initHA_homingpedals()
{
    homingpedals_HA = new HybridAutomata(HP_START, HP_FINISH);

    homingpedals_HA->setState(HP_THROTTLE, homingThrottle);
    homingpedals_HA->setState(HP_BRAKE, homingBrake);
    homingpedals_HA->setState(HP_FINISH, homingFinish);

    homingpedals_HA->setCondition(HP_START, NULL, HP_THROTTLE);
    homingpedals_HA->setCondition(HP_THROTTLE, NULL, HP_BRAKE);
    homingpedals_HA->setCondition(HP_BRAKE, NULL, HP_FINISH);
}
void operateHA_homingpedals()
{
    
    homingpedals_HA->operate();
    if(homingpedals_HA -> curState == HP_FINISH)
    {
       homingpedals_HA -> curState = HP_START;
    }
}
////////////////////////////////////////////////////////////////////////
///////////////////////////end of HomingPedals Module///////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///////////////////////////E-Stop Module////////////////////////////////
////////////////////////////////////////////////////////////////////////
enum
{
    E_START,
    E_STOP,
    E_FINISH
};

class ESTOP2ESTOP : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_brake")) < get_cfParam("Brake.virtual_max_position"))
            return true;
        else
            return false;
    }
};
class ESTOP2FINISH : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_brake")) >= get_cfParam("Brake.virtual_max_position"))
            return true;
        else
            return false;
    }
};
void estop()
{
    homingThrottle();
    brakeModule("Brake.target_position", get_cfParam("Brake.virtual_max_position"));
    // reset_pid_values_for_cc();
}
void efinish()
{
    set_rtParam("Motion.state", "value",STANDBY);
    return;
}
void initHA_estop()
{
    estop_HA = new HybridAutomata(E_START, E_FINISH);

    estop_HA->setState(E_STOP, estop);
    estop_HA->setState(E_FINISH, efinish);

    ESTOP2ESTOP *estop2stop = new ESTOP2ESTOP();
    ESTOP2FINISH *estop2finish = new ESTOP2FINISH();

    estop_HA->setCondition(E_START, NULL, E_STOP);
    estop_HA->setCondition(E_STOP, estop2stop, E_STOP);
    estop_HA->setCondition(E_STOP, estop2finish, E_FINISH);
}
void operateHA_estop()
{
    
    estop_HA->operate();
    if(estop_HA -> curState == E_FINISH)
    {
       estop_HA -> curState = E_START;
    }
}

////////////////////////////////////////////////////////////////////////
///////////////////////////end of E-Stop Module/////////////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///////////////////////////Pullover Module//////////////////////////////
////////////////////////////////////////////////////////////////////////
enum
{
    PO_START,
    PO_DECIDE,
    PO,
    PO_FINISH
};
float pullover_tick_count = 0;
clock_t start_time,finish_time;
clock_t cur_time;


class START2DECIDE : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        start_time = clock();
        return true;
    }
};
class PO2DECIDE : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_brake")) < get_cfParam("Brake.virtual_max_position"))
            return true;
        else
            return false;
    }
};
class PO2FINISH : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_brake")) >= get_cfParam("Brake.virtual_max_position"))
        {
            finish_time = clock();
            pullover_tick_count=0;
            return true;
        }
        else
            return false;
    }
};

void decidePullover()
{
    static double init_vel;
    double cur_vel;
    static double delta_time;
    static double opt_vel;
    
    int err_margin = get_cfParam("Pullover.error_margin");
    double jerk = get_cfParam("Pullover.jerk");
    double pos_per_tick = (double)get_cfParam("Pullover.position_per_tick") / (double)get_cfParam("Brake.convert_position");
    

    /* if(get_var("Current_Velocity", "value")==0)
  {
    set_virtual_brake_target_pos(v_BRAKE_MAX_POS);
  }*/
    homingThrottle();
    if (pullover_tick_count == 0)
    {
        init_vel = get_rtParam_double("OBD.current_velocity", "value") / 3.6;
    }

    cur_vel = get_rtParam_double("OBD.current_velocity", "value") / 3.6;
    delta_time = (double)(clock() - start_time) / 100000;
    opt_vel = (double)(jerk * delta_time) * delta_time + init_vel;

    printf("************************\n");
    printf("** init_vel = %.3lf\n", init_vel);
    printf("** cur_vel = %.3lf\n", cur_vel);
    printf("** delta_time = %.3lf\n", delta_time);
    printf("** opt_vel = %.3lf\n", opt_vel);
    printf("** opt_vel = %.3lf\n", opt_vel);
    printf("** cur_pos = %d\n", get_slave_physical_act_pos(get_cfParam("ECAT.motor_id_brake")));
    printf("** tar_pos = %d\n", get_slave_physical_target_pos(get_cfParam("ECAT.motor_id_brake")));
    printf("************************\n");

    if (opt_vel > cur_vel || opt_vel == cur_vel)
    {
        pullover_tick_count -= pos_per_tick;
    }
    else if (opt_vel < cur_vel)
    {
        pullover_tick_count += pos_per_tick;
    }

    //reset_pid_values_for_cc();
}
void pullover()
{

    double start_pos = (double)get_cfParam("Brake.start_position") / (double)get_cfParam("Brake.convert_position");
    double target_pos;

    target_pos = start_pos + pullover_tick_count;
    if ((start_pos + pullover_tick_count) >= get_cfParam("Brake.virtual_max_position"))
    {
        target_pos = get_cfParam("Brake.virtual_max_position");
    }
    brakeModule("Brake.target_position", target_pos);
}
void pofinish()
{
    set_rtParam("Motion.state", "value",STANDBY);
    return;
}
void initHA_pullover()
{
    pullover_HA = new HybridAutomata(PO_START, PO_FINISH);

    pullover_HA->setState(PO_DECIDE, decidePullover);
    pullover_HA->setState(PO, pullover);
    pullover_HA->setState(PO_FINISH, pofinish);

    START2DECIDE *s2d = new START2DECIDE();
    PO2DECIDE *po2d = new PO2DECIDE();
    PO2FINISH *po2f = new PO2FINISH();

    pullover_HA->setCondition(PO_START, s2d, PO_DECIDE);
    pullover_HA->setCondition(PO_DECIDE, NULL, PO);
    pullover_HA->setCondition(PO, po2d, PO_DECIDE);
    pullover_HA->setCondition(PO, po2f, PO_FINISH);
}
void operateHA_pullover()
{
    
    pullover_HA->operate();
    if(pullover_HA -> curState == PO_FINISH)
    {
       pullover_HA -> curState = PO_START;
    }
}

////////////////////////////////////////////////////////////////////////
///////////////////////////end of Pullover Module///////////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///////////////////////////Selftest Module//////////////////////////////
////////////////////////////////////////////////////////////////////////
enum
{
    ST_START,
    ST_T,
    ST_B,
    ST_G_MAX,
    ST_G_MIN,
    ST_S_MAX,
    ST_S_MIN,
    ST_S_O,
    ST_L_MAX,
    ST_L_MIN,
    ST_FINISH
};
class ST_T2T : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_throttle")) < get_cfParam("Throttle.virtual_max_position"))
            return true;
        else
            return false;
    }
};

class ST_T2B : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_throttle")) >= get_cfParam("Throttle.virtual_max_position"))
            return true;
        else
            return false;
    }
};
class ST_B2B : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_brake")) < get_cfParam("Brake.virtual_max_position"))
            return true;
        else
            return false;
    }
};
class ST_B2G_max : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_brake")) >= get_cfParam("Brake.virtual_max_position"))
            return true;
        else
            return false;
    }
};
class ST_G_max2G_max : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_gearstick")) != get_cfParam("Gearstick.drive_position"))
            return true;
        else
            return false;
    }
};
class ST_G_max2G_min : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_gearstick")) == get_cfParam("Gearstick.drive_position"))
            return true;
        else
            return false;
    }
};
class ST_G_min2G_min : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_gearstick")) != get_cfParam("Gearstick.park_position"))
            return true;
        else
            return false;
    }
};
class ST_G_min2S_max : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_gearstick")) == get_cfParam("Gearstick.park_position"))
            return true;
        else
            return false;
    }
};
class ST_S_max2S_max : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_steer")) != get_cfParam("Steer.virtual_max_position"))
            return true;
        else
            return false;
    }
};
class ST_S_max2S_min : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_steer")) == get_cfParam("Steer.virtual_max_position"))
            return true;
        else
            return false;
    }
};
class ST_S_min2S_min : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_steer")) != get_cfParam("Steer.virtual_min_position"))
            return true;
        else
            return false;
    }
};
class ST_S_min2S_o : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_steer")) == get_cfParam("Steer.virtual_min_position"))
            return true;
        else
            return false;
    }
};
class ST_S_o2S_o : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_steer")) != get_cfParam("Steer.start_position"))
            return true;
        else
            return false;
    }
};
class ST_S_o2L_max : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_steer")) == get_cfParam("Steer.start_position"))
            return true;
        else
            return false;
    }
};
class ST_L_max2L_max : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_center")) < get_cfParam("Lidar.center_max_degree") &&
        get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_left")) < get_cfParam("Lidar.left_max_degree") &&
        get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_right")) > get_cfParam("Lidar.right_min_degree") )
            return true;
        else
            return false;
    }
};
class ST_L_max2L_min : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_center")) >= get_cfParam("Lidar.center_max_degree") &&
        get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_left")) >= get_cfParam("Lidar.left_max_degree") &&
        get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_right")) <= get_cfParam("Lidar.right_min_degree") )
            return true;
        else
            return false;
    }
};
class ST_L_min2L_min : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_center")) > get_cfParam("Lidar.center_min_degree") &&
        get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_left")) > get_cfParam("Lidar.left_min_degree") &&
        get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_right")) < get_cfParam("Lidar.right_max_degree") )
            return true;
        else
            return false;
    }
};
class ST_L_min2F : public Condition
{
  public:
    bool check(HybridAutomata *HA)
    {
        if (get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_center")) <= get_cfParam("Lidar.center_min_degree") &&
        get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_left")) <= get_cfParam("Lidar.left_min_degree") &&
        get_slave_virtual_current_pos(get_cfParam("ECAT.motor_id_lidar_right")) >= get_cfParam("Lidar.right_max_degree") )
            return true;
        else
            return false;
    }
};
void selftestThrottle()
{
    static double target_pos = 0;
    double pos_per_tick = (double)get_cfParam("SelfTest.Throttle.position_per_tick") / (double)get_cfParam("Throttle.convert_position");

    target_pos += pos_per_tick;
    throttleModule("Throttle.target_position", target_pos);
}
void selftestBrake()
{
    static double target_pos = 0;
    double pos_per_tick = (double)get_cfParam("SelfTest.Brake.position_per_tick") / (double)get_cfParam("Brake.convert_position");

    target_pos += pos_per_tick;
    homingThrottle();
    brakeModule("Brake.target_position", target_pos);
}
void selftestGearstick_max()
{
    gearstickModule("Gearstick.target_position",3);
}
void selftestGearstick_min()
{
    gearstickModule("Gearstick.target_position",0);
}
void selftestSteer_max()
{
    homingBrake();
    steerModule("Steer.target_position", get_cfParam("Steer.virtual_max_position"));
}
void selftestSteer_min()
{
    steerModule("Steer.target_position", get_cfParam("Steer.virtual_min_position"));
}
void selftestSteer_origin()
{
    steerModule("Steer.target_position", get_cfParam("Steer.start_position"));
}
void selftestLidars_max()
{
    lidarModule("center_target_position", get_cfParam("Lidar.center_max_degree"));
    lidarModule("left_target_position", get_cfParam("Lidar.left_max_degree"));
    lidarModule("right_target_position", get_cfParam("Lidar.right_min_degree"));
}
void selftestLidars_min()
{
    lidarModule("center_target_position", get_cfParam("Lidar.center_min_degree"));
    lidarModule("left_target_position", get_cfParam("Lidar.left_min_degree"));
    lidarModule("right_target_position", get_cfParam("Lidar.right_max_degree"));
}
void selftestFinish()
{
    set_rtParam("Motion.state", "value",STANDBY);
    return;
}
void initHA_selftest()
{
 
    selftest_HA = new HybridAutomata(ST_START, ST_FINISH);

    selftest_HA->setState(ST_T, selftestThrottle);
    selftest_HA->setState(ST_B, selftestBrake);
    selftest_HA->setState(ST_G_MAX, selftestGearstick_max);
    selftest_HA->setState(ST_G_MIN, selftestGearstick_min);
    selftest_HA->setState(ST_S_MAX, selftestSteer_max);
    selftest_HA->setState(ST_S_MIN, selftestSteer_min);
    selftest_HA->setState(ST_S_O, selftestSteer_origin);
    selftest_HA->setState(ST_L_MAX, selftestLidars_max);
    selftest_HA->setState(ST_L_MIN, selftestLidars_min);
    selftest_HA->setState(ST_FINISH, selftestFinish);
    

    ST_T2T *t2t = new ST_T2T();
    ST_T2B *t2b = new ST_T2B();
    ST_B2B *b2b = new ST_B2B();
    ST_B2G_max *b2g_max = new ST_B2G_max();
    ST_G_max2G_max *g_max2g_max = new ST_G_max2G_max();
    ST_G_max2G_min *g_max2g_min = new ST_G_max2G_min();
    ST_G_min2G_min *g_min2g_min = new ST_G_min2G_min();
    ST_G_min2S_max *g_min2s_max = new ST_G_min2S_max();
    ST_S_max2S_max *s_max2s_max = new ST_S_max2S_max();
    ST_S_max2S_min *s_max2s_min = new ST_S_max2S_min();
    ST_S_min2S_min *s_min2s_min = new ST_S_min2S_min();
    ST_S_min2S_o *s_min2s_o = new ST_S_min2S_o();
    ST_S_o2S_o *s_o2s_o = new ST_S_o2S_o();
    ST_S_o2L_max *s_o2l_max = new ST_S_o2L_max();
    ST_L_max2L_max *l_max2l_max = new ST_L_max2L_max();
    ST_L_max2L_min *l_max2l_min = new ST_L_max2L_min();
    ST_L_min2L_min *l_min2l_min = new ST_L_min2L_min();
    ST_L_min2F *l_min2F= new ST_L_min2F();

    selftest_HA->setCondition(ST_START, NULL, ST_T);
    selftest_HA->setCondition(ST_T, t2t, ST_T);
    selftest_HA->setCondition(ST_T, t2b, ST_B);
    selftest_HA->setCondition(ST_B, b2b, ST_B);
    selftest_HA->setCondition(ST_B, b2g_max, ST_G_MAX);
    selftest_HA->setCondition(ST_G_MAX, g_max2g_max, ST_G_MAX);
    selftest_HA->setCondition(ST_G_MAX, g_max2g_min, ST_G_MIN);
    selftest_HA->setCondition(ST_G_MIN, g_min2g_min, ST_G_MIN);
    selftest_HA->setCondition(ST_G_MIN, g_min2s_max, ST_S_MAX);
    selftest_HA->setCondition(ST_S_MAX, s_max2s_max, ST_S_MAX);
    selftest_HA->setCondition(ST_S_MAX, s_max2s_min, ST_S_MIN);
    selftest_HA->setCondition(ST_S_MIN, s_min2s_min, ST_S_MIN);
    selftest_HA->setCondition(ST_S_MIN, s_min2s_o, ST_S_O);
    selftest_HA->setCondition(ST_S_O, s_o2s_o, ST_S_O);
    selftest_HA->setCondition(ST_S_O, s_o2l_max, ST_L_MAX);
    selftest_HA->setCondition(ST_L_MAX, l_max2l_max, ST_L_MAX);
    selftest_HA->setCondition(ST_L_MAX, l_max2l_min, ST_L_MIN);
    selftest_HA->setCondition(ST_L_MIN, l_min2l_min, ST_L_MIN);
    selftest_HA->setCondition(ST_L_MIN, l_min2F, ST_FINISH);
}

void operateHA_selftest()
{
    
    selftest_HA->operate();
    if(selftest_HA -> curState == ST_FINISH)
    {
       selftest_HA -> curState = ST_START;
    }
}

////////////////////////////////////////////////////////////////////////
///////////////////////////end of Selftest Module///////////////////////
////////////////////////////////////////////////////////////////////////