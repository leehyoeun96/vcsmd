#include "mission_manager/mission_manager.h"
#include <ros/ros.h>
#include <iostream>

int main(int argc, char **argv)
{
    ros::init(argc, argv, "mission_manager_test");
    mManagerNS::mManager mMgr;
    mMgr.MainLoop();
    return 0;
}
