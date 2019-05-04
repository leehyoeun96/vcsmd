//#include "../include/mission_manager/mission_manager.h"
/*
void callbackGetCurrentPose(const geometry_msgs::PoseStampedConstPtr& msg)
{
	cout<<msg->pose.position.x<<endl;
}
*/

#include <ros/ros.h>
#include "mission_manager/Message1.h"

using namespace std;

ros::Publisher pub;
mission_manager::Message1 msg;
int main(int argc, char **argv)
{
    ros::init(argc, argv, "mission_manager");
    ros::NodeHandle nh;

    pub = nh.advertise<mission_manager::Message1>("pub_msg",100);
    ros::Rate loop_rate(10);
 
    int count = 0;
    while(ros::ok())
        {
            mission_manager::Message1 msg;
            msg.data = count;
            pub.publish(msg);
            loop_rate.sleep();
            ++count;
        }
    return 0;
}
