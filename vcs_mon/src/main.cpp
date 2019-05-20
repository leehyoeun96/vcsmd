#include "vcs_mon/vcs_mon.h"

char black[] = {0x1b, '[', '0', ';', '3', '0', 'm', 0};
char dark_gray[] = {0x1b, '[', '1', ';', '3', '0', 'm', 0};
char red[] = {0x1b, '[', '0', ';', '3', '1', 'm', 0};
char light_red[] = {0x1b, '[', '1', ';', '3', '1', 'm', 0};
char green[] = {0x1b, '[', '0', ';', '3', '2', 'm', 0};
char light_green[] = {0x1b, '[', '1', ';', '3', '2', 'm', 0};
char brown[] = {0x1b, '[', '0', ';', '3', '3', 'm', 0};
char yellow[] = {0x1b, '[', '1', ';', '3', '3', 'm', 0};
char blue[] = {0x1b, '[', '0', ';', '3', '4', 'm', 0};
char light_blue[] = {0x1b, '[', '1', ';', '3', '4', 'm', 0};
char purple[] = {0x1b, '[', '0', ';', '3', '5', 'm', 0};
char light_purple[] = {0x1b, '[', '1', ';', '3', '5', 'm', 0};
char cyan[] = {0x1b, '[', '0', ';', '3', '6', 'm', 0};
char light_cyan[] = {0x1b, '[', '1', ';', '3', '6', 'm', 0};
char light_gray[] = {0x1b, '[', '0', ';', '3', '7', 'm', 0};
char white[] = {0x1b, '[', '1', ';', '3', '7', 'm', 0};

void printStatusBar(const vcs_mon::graph::ConstPtr& msg)
{
    int i;
    int switch_color = 1;

    for (i = 0; i < (int)msg->cvel; i++)
    {
	if(i % 10 == 0)
	{
           cout << light_green;
	   if(switch_color == 1)
	   {
		cout << light_green;
		switch_color = 0;
 	   }
	   else if(switch_color == 0)
	   {
		cout << green;
		switch_color = 1;
 	   }

	}
        cout << "0";
    }
    cout << endl;

    for (i = 0; i < (int)(msg->tvel); i++)
    {
        cout << " ";
    }
    cout << yellow << "^\n";

    for (i = 0; i < (int)(msg->tvel - 1); i++)
    {
        cout << " ";
    }
    cout << yellow << msg->tvel << endl;
    cout << white;
}   

int main(int argc, char **argv)
{
    ros::init(argc, argv, "vcs_mon");
    ros::NodeHandle nh;
    ros::Subscriber graph_msg_sub = nh.subscribe("/graph_value",1,printStatusBar);

    while(ros::ok())
    {
        ros::spinOnce();
    }
    return 0;
}
