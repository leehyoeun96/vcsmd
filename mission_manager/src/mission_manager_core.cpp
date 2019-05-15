#include "mission_manager/mission_manager.h"

namespace mManagerNS
{ 

 mManager::mManager()
 {
	bReset = false;
	vertex[4] = 0;
	missionNo = 1;
	seqno = 0;
	pub_vcs_msg = nh.advertise<mission_manager::Message>("/vcs_msg", 10);
	pub_initial_pose = nh.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 10);
	pub_goal_polygon = nh.advertise<geometry_msgs::PolygonStamped>("/goal_polygon", 10);
	pub_goal_pose = nh.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 10);
	sub_current_pose = nh.subscribe("/current_pose", 10, &mManager::callbackGetCurrentPose, this);
	sub_vcs_ack = nh.subscribe("/vcs_ack", 10, &mManager::callbackGetVCSack, this);
	initialpose_msg.header.frame_id = "map";
	initialpose_msg.pose.pose.position.z = 0;
	initialpose_msg.pose.pose.orientation.x = 0;
	initialpose_msg.pose.pose.orientation.y = 0;
	goalpose_msg.header.frame_id = "map";
	goalpose_msg.pose.position.z = 0;
	goalpose_msg.pose.orientation.x = 0;
	goalpose_msg.pose.orientation.y = 0;
	
 	goalpolygon_msg.header.frame_id = "world";
 	goalpolygon_msg.polygon.points.resize(4);
 }

 mManager::~mManager()
 {
 }
 void mManager::CleanAndStart()
 {
	//shutdown
	int stat = system("/home/autoware/Autoware/ros/src/util/packages/mission_manager/script/shutdown");
	//restart
	stat = system("roslaunch mission_manager global_planner.launch &");
	sleep(5);
	
	pub_initial_pose.publish(initialpose_msg); //when you drive in road, disable this line
	pub_goal_pose.publish(goalpose_msg);
	stat = system("roslaunch mission_manager local_planner.launch &");
 }
			
 void mManager::MatchMissionGoalArea(double* curr)
 {
	int margin = 5;
	double cur_goal_x, cur_goal_y;
	double* goal;	
	cout<<"missionNo : "<<missionNo<<endl;
	if(missionNo > MAX_MISSION)
	{
		cout<<"end!!"<<endl;
		int stat = system("/home/autoware/Autoware/ros/src/util/packages/mission_manager/script/shutdown");
		exit(0);
	}
	goal = goal_pose[missionNo];
	cur_goal_x = goal_pose[missionNo - 1][0];
	cur_goal_y = goal_pose[missionNo - 1][1];
	
	int lu_x = cur_goal_x - margin;
	int lu_y = cur_goal_y - margin;
	int rd_x = cur_goal_x + margin;
	int rd_y = cur_goal_y + margin;
	int z = 0.988161278591;
	
	bool bIsGoal = (lu_x) <= curr[0] 
		    && (rd_x) >= curr[0] 
		    && (lu_y) <= curr[1]
	 	    && (rd_y) >= curr[1];
	
	goalpolygon_msg.polygon.points[0].x = lu_x;
	goalpolygon_msg.polygon.points[0].y = rd_y;
	goalpolygon_msg.polygon.points[0].z = z;
 	goalpolygon_msg.polygon.points[1].x = lu_x;
	goalpolygon_msg.polygon.points[1].y = lu_y;
	goalpolygon_msg.polygon.points[1].z = z;
	goalpolygon_msg.polygon.points[2].x = rd_x;
	goalpolygon_msg.polygon.points[2].y = lu_y;
	goalpolygon_msg.polygon.points[2].z = z;
	goalpolygon_msg.polygon.points[3].x = rd_x;
	goalpolygon_msg.polygon.points[3].y = rd_y;
	goalpolygon_msg.polygon.points[3].z = z;

	if(bIsGoal)
	{
		if(missionNo != MAX_MISSION)
		{
		 settingPoseMsg(curr, goal);
		 CleanAndStart();
		}
		bIsGoal = false;
		missionNo++;
	}
 }

 void mManager::settingPoseMsg(double* init, double* goal)
 {
	initialpose_msg.pose.pose.position.x = init[0];
	initialpose_msg.pose.pose.position.y = init[1];
	initialpose_msg.pose.pose.orientation.z = init[2];
	initialpose_msg.pose.pose.orientation.w = init[3];
 
	goalpose_msg.pose.position.x = goal[0];
	goalpose_msg.pose.position.y = goal[1];
	goalpose_msg.pose.orientation.z = goal[2];
	goalpose_msg.pose.orientation.w = goal[3];
 }


 void mManager::callbackGetCurrentPose(const geometry_msgs::PoseStampedConstPtr& msg)
 {
	double goalPose[4];
	double cur_pose[4] = {msg->pose.position.x, msg->pose.position.y, msg->pose.orientation.z, msg->pose.orientation.w};

//	ROS_INFO( "x: %f y: %f\n",cur_pose[0], cur_pose[1]);
//	cout<<"missionNo : " << missionNo << endl;
	MatchMissionGoalArea(cur_pose);
 }

 void mManager::settingVCS(string msg)
 {
	 string substr, subvalue;
	 string::size_type pos;
	 pos = msg.find_last_of(" ");
	 cout << msg.substr(0,pos) <<endl;
	 cout << msg.substr(pos+1) <<endl;
	 substr = msg.substr(0,pos);
	 subvalue = msg.substr(pos+1);

	 if(substr.compare("pose lidar") == 0)
	 {	
		 vcs_msg.command = substr;
		 vcs_msg.value = atoi(subvalue.c_str()); 
	 }
	 else
	 {	
		 vcs_msg.command = msg;
		 vcs_msg.value = 0; 
	 }
	 pub_vcs_msg.publish(vcs_msg);

	 cout << msg <<endl;
 }

 void mManager::callbackGetVCSack(const mission_manager::vcs_msg& msg)
 {
//	cout<<"vcs ack: seq_no "<<msg.seq_no<<endl;
//	cout<<"vcs ack: ack_no "<<msg.ack_no<<endl;
//	cout<<"vcs ack: cmd_code "<<msg.cmd_code<<endl;
//	cout<<"vcs ack: param_id "<< msg.param_id<<endl;
//	cout<<"vcs ack: param_val "<<msg.param_val<<endl;
//	cout<<"vcs ack: result_code "<< msg.result_code<<endl;
//	cout<<"vcs ack: result_msg "<<msg.result_msg<<endl;
}

 void mManager::MainLoop()
 {
	ros::Rate loop_rate(10);
	for(int i = 0; i<2;i++)
	{	
		cout<<"set VCS and lidar (ex. start vcs, pose lidar [value])"<<endl;
		getline(cin, message);
		settingVCS(message);
	}
	settingPoseMsg(init_pose, goal_pose[0]);
	CleanAndStart();
	//epoll setting//
	epfd = epoll_create1(0);
	ev.events = EPOLLIN;
	ev.data.fd = 0;
	epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &ev);

	while(ros::ok())
	{    
		ros::spinOnce();
		event_cnt= epoll_wait(epfd,evs,MAX_EVENTS,1);
		if(event_cnt ==-1){
			perror("epoll");
			break;
		}
		else if(event_cnt ==0){
			continue;
		}
		for(i=0;i<=event_cnt;i++){
			if(evs[i].events ==EPOLLIN){
				getline(cin, message);
				settingVCS(message);
			}
		}

		pub_goal_polygon.publish(goalpolygon_msg);
	}
	epoll_ctl(epfd,EPOLL_CTL_DEL,0,NULL);
	return;
 }
}
