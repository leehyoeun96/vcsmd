#include "vcs_agent/vcs_agent.h" 
int main(int argc, char**argv)
{
    ros::init(argc, argv,"vcs_agent");

    vcsAgent::vAgent agt;
    agt.MainLoop();
    return 0;

}
