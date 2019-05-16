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

#define MAX_MISSION 2
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

	double init_pose[4]={14.9361858368, 1.67250847816,-0.00892763260947, 0.999960147894};
	double goal_pose[MAX_MISSION][4]={
				{-0.122846618295, 93.5581130981, -0.99828613548, 0.058521719973},
                                {2.17429161072, 0.396320402622, 0.0, 1.0},
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
