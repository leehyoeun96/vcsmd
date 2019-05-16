Before you run this node on a road, check below.

In launch/global_planner.launch
1. change sim_mode = false
2. do not use wf_simulator

In launch/mission_manager.launch
1. do not use map.launch

In src/mission_manager_core.cpp
1. do not publish init_pose

In include/mission_manager/mission_manager.h
1. check shutdown script directory
2. check init/goal_pose
