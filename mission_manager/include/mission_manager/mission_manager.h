#include <ros/ros.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PolygonStamped.h>
#include "mission_manager/Message.h"
#include "mission_manager/vcs_msg.h"

#define MAX_MISSION 5
#define MAX_EVENTS 10
#define BUF_SIZE 100
using namespace std;
using namespace ros;

namespace mManagerNS
{

class mManager
{
  private:
	string message;
	int i;
	int event_cnt;
	int epfd;
	struct epoll_event ev, evs[MAX_EVENTS];

	double init_pose[4]={-413.095275879, -55.8339004517, 0.153418667355, 0.988161278591};
	double goal_pose[MAX_MISSION][4]={
				{-307.649017334, -18.9735794067, 0.146586847705, 0.989197804324},
                                {-233.744873047, 6.70031738281, 0.122183437189, 0.992507535325},
                                {-64.6641082764, 66.4837417603, 0.155610846069, 0.987818437055},
                                {41.8825073242, 103.160705566, 0.125714313914, 0.992066485311},
                                {120.737976074, 130.851776123, 0.184384473071, 0.982854193708}
                                };

	bool bReplan;
	bool bReset;
	double vertex[4];
	int missionNo;
	int seqno;
	mission_manager::Message vcs_msg;
	geometry_msgs::PoseWithCovarianceStamped initialpose_msg;
	geometry_msgs::PoseStamped goalpose_msg;
	geometry_msgs::PolygonStamped goalpolygon_msg;

	ros::Publisher pub_vcs_msg;
	ros::Publisher pub_initial_pose;
	ros::Publisher pub_goal_pose;
	ros::Publisher pub_goal_polygon;
	ros::Subscriber sub_current_pose;
	ros::Subscriber sub_vcs_ack;
	ros::NodeHandle nh;

	void CleanAndStart();
	void callbackGetCurrentPose(const geometry_msgs::PoseStampedConstPtr& msg);
	void callbackGetVCSack(const mission_manager::vcs_msg & msg);
	void MatchMissionGoalArea(double* curr);
	void settingPoseMsg(double* init, double* goal);
	void settingVCS(string mesg);
 public:
	mManager();
	~mManager();
	void MainLoop();
};

}
