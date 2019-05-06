#include "vcs_client/vcs_client.h"
ros::Subscriber client_sub;
ros::Subscriber client_sub1;
ros::Subscriber vcs_msg_sub;
ros::Publisher vcs_cmd_pub;
ros::Publisher vcs_val_pub;

vcs_client::agent vcs_cmd_msg;
vcs_client::agent vcs_val_msg;
double buffer[2];

double convert_mps_to_kmh(double linear_x)
{
    return linear_x * 3.6;
}

void VCSstartupCallback(const vcs_client::mm::ConstPtr& msg)
{
    vcs_cmd_msg.result_code = 0;
//    std::string path = ros::package::getPath("vcs_client");
/*    using package::V_string;
    V_string pacakges;
    ros::package::getAll(packages);*/
    if((msg->command).compare("start vcs") == 0)//start vcs
    {
        FILE *fp = fopen("/home/rubicom/catkin_ws/src/vcs_client/start_up.txt","r");

        char file_buf[50]={0,};
        if(fp != NULL)
        {
            while(1)
            {
                fgets(file_buf, sizeof(file_buf),fp);
                if(feof(fp)) break;
                vcs_cmd_msg = parse_handler(file_buf);
            }
            fclose(fp);
        }
        else cout << "can't read file" <<endl;
    }
    else if((msg->command).compare("pose lidar") == 0) //pose lidar
    {
        string set_pose ("set poselidar.mode ");
        string dtos; 
        dtos = to_string(msg->value);
        set_pose += dtos;
        set_pose += "\n";
        vcs_cmd_msg = parse_handler((char*)set_pose.c_str());
    }
    else
    {
        cout<<"main callback:msg->result_code "<< vcs_cmd_msg.result_code <<endl;
        string cmd = msg->command;
        cmd += "\n";
        vcs_cmd_msg = parse_handler((char*)cmd.c_str());
    }
    if(vcs_cmd_msg.result_code == 0) vcs_cmd_pub.publish(vcs_cmd_msg);
    else if(vcs_cmd_msg.result_code == -1)
        cout<<"main callback:unknown command = "<< msg->command<<endl;
}
void curVelCallback(const geometry_msgs::TwistStampedConstPtr &msg)
{
    //buffer[2]= convert_mps_to_kmh(msg->twist.linear.x);
}
void twistCallback(const geometry_msgs::TwistStampedConstPtr &input_msg)
{
    string set_vel ("set cruisecontrol.target_velocity ");
    string set_ang ("set steercontrol.target_angular_velocity ");
    string dtos;
    
    buffer[0] = convert_mps_to_kmh(input_msg->twist.linear.x);
    buffer[1] = input_msg->twist.angular.z;
    
    dtos = to_string(buffer[0]);
    set_vel += dtos;
    set_vel += "\n";
    vcs_val_msg = parse_handler((char*)set_vel.c_str());
    vcs_val_pub.publish(vcs_val_msg);
    
    dtos = to_string(buffer[1]);
    set_ang += dtos;
    set_ang += "\n";
    vcs_val_msg = parse_handler((char*)set_ang.c_str());
    vcs_val_pub.publish(vcs_val_msg);
}

int main(int argc, char**argv)
{
    ros::init(argc, argv,"vcs_client");
    ros::NodeHandle nh;
    regcomp_all();
    client_sub = nh.subscribe("/twist_cmd",1,twistCallback);
    vcs_msg_sub = nh.subscribe("/vcs_msg",1,VCSstartupCallback);
    client_sub1 = nh.subscribe("/current_velocity",1,curVelCallback);
    vcs_cmd_pub = nh.advertise<vcs_client::agent>("/startup_cmd",100);
    vcs_val_pub = nh.advertise<vcs_client::agent>("/target_val",100);
    
    cout<<"hello!"<<endl;
   while(ros::ok())
    {
        ros::spinOnce();
    }
    regfree_all();
    return 0;
}

