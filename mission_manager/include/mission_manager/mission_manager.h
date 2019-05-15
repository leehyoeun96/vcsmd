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

#define MAX_MISSION 4
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

	double init_pose[4]={18.9262599945, 95.3431091309, -0.999870045252, 0.0161211850601};
	double goal_pose[MAX_MISSION][4]={
				{-19.4966564178, 91.7895507812, -0.994828294927, 0.101570978205},
                                {-35.154548645, 66.4704055786, -0.636849333142, 0.770988279338},
                                {-30.9346904755, 35.598815918, -0.613035404341, 0.790055436678},
                                {-6.5039358139, 3.72778320312, -0.350655305441, 0.936504595166},
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
