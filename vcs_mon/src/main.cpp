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

int hzCt = 0;
int pktCnt = 0;
float ndtscore = 0;
int cvel = 0;
int tvel = 0;

int mps_to_kmh(double act_vel)
{
    return (int)(act_vel * 3.6);
}

void printBarGraph()
{
    pktCnt++;
    if(pktCnt == hzCt)
    {
        int i;
        int switch_color = 1;

        for (i = 0; i < cvel; i++)
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

        for (i = 0; i < (tvel); i++)
        {
            cout << " ";
        }
        cout << yellow << "^\n";

        for (i = 0; i <(tvel - 1); i++)
        {
            cout << " ";
        }
        cout << yellow << tvel << endl;
	if(ndtscore > 0.5)
	     cout << light_red <<ndtscore <<endl;
	else
	     cout << light_blue <<ndtscore <<endl;
        cout << white<< endl;
        pktCnt = 0;
    }

}

void printStatusBar(const vcs_mon::graph::ConstPtr& msg)
{
    cvel = mps_to_kmh(msg->cvel);
    tvel = msg->tvel;
	printBarGraph();
}

void scoreCallback(const vcs_mon::NDTStat::ConstPtr &msg)
{
	ndtscore = msg->score;
	printBarGraph();
}
int main(int argc, char **argv)
{
    ros::init(argc, argv, "vcs_mon");
    ros::NodeHandle nh;
    ros::Subscriber graph_msg_sub = nh.subscribe("/graph_value",100,printStatusBar);
    ros::Subscriber score_sub = nh.subscribe("ndt_stat", 100, scoreCallback);
    if(argv[1] == NULL) 
    {
        printf("set print hz\n");
        return 1;
    }
    hzCt = atoi(argv[1]);
    if(hzCt == 0)
    {
        printf("set hz correctly!\n");
        return 1;
    }

		ros::Rate rate(10);
    printf("set hz %d\n", hzCt);
	spin();
    return 0;
}
