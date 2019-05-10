#include "vcs_bridge/vcs_bridge.h" 
#define MAX_EVENTS 10 
#define BUF_SIZE 255 
#define IP_ADDR "127.0.0.1"

namespace vcsBridgeNS
{
    vcsBridge::vcsBridge()
    {

        int vcsmd_sd;
        int vcsd_sd;
        double buffer[2];

        regcomp_all();
        ros::Subscriber client_sub = nh.subscribe("/twist_cmd",1,twistCallback);//("/target_val",100,sendtovcs)
        ros::Subscriber vcs_msg_sub = nh.subscribe("/vcs_msg",1,VCSstartupCallback);//("/startup_cmd",100,sendtovcs)

        ros::Publisher agent_pub = nh.advertise<vcs_bridge::vcs>("/vcs_ack", 10);
    }

    vcsBridge::~vcsBridge()
    {
    }


    double vcsBridge::convert_mps_to_kmh(double linear_x)
    {
        return linear_x * 3.6;
    }
    void vcsBridge::VCSstartupCallback(const vcs_bridge::Message1::ConstPtr& msg)
    {   
        message vcs_cmd_msg;
        int bytecount = 0;
        vcs_cmd_msg.result_code = 0;
        //    std::string path = ros::package::getPath("vcs_bridge");
        /*    using package::V_string;
              V_string pacakges;
              ros::package::getAll(packages);*/
        if((msg->command).compare("start vcs") == 0)//start vcs
        {
            FILE *fp = fopen("/home/rubicom/catkin_ws/src/vcs_bridge/start_up.txt","r");

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
        if(vcs_cmd_msg.result_code == 0)
        { 
            cout << "sendtovcs: seq_no "<< vcs_cmd_msg.seq_no<<endl;
            cout << "sendtovcs: ack_no "<< vcs_cmd_msg.ack_no<<endl;
            cout << "sendtovcs: cmd_code "<<  vcs_cmd_msg.cmd_code<<endl;
            cout << "sendtovcs: param_id "<<  vcs_cmd_msg.param_id<<endl;
            cout << "sendtovcs: param_val "<<  vcs_cmd_msg.param_val<<endl;
            cout << "sendtovcs: result_code "<<  vcs_cmd_msg.result_code<<endl;
            cout << "sendtovcs: result_msg " <<  vcs_cmd_msg.result_msg<<endl;

            if ((bytecount = send(vcsd_sd,(char*)& vcs_cmd_msg, sizeof(message), 0)) == -1)
            {   
                fprintf(stderr, "Error sending data : %s\n", strerror(errno));
            }
        }
        else if(vcs_cmd_msg.result_code == -1)
            cout<<"main callback:unknown command = "<< msg->command<<endl;
    }

    void vcsBridge::twistCallback(const geometry_msgs::TwistStampedConstPtr &input_msg)
    {
        message packet;
        int bytecount = 0;

        string set_vel ("set cruisecontrol.target_velocity ");
        string set_ang ("set steercontrol.target_angular_velocity ");
        string dtos;

        buffer[0] = convert_mps_to_kmh(input_msg->twist.linear.x);
        buffer[1] = input_msg->twist.angular.z;

        dtos = to_string(buffer[0]);
        set_vel += dtos;
        set_vel += "\n";
        packet = parse_handler((char*)set_vel.c_str());
        cout << "sendtovcs: seq_no "<< packet.seq_no<<endl;
        cout << "sendtovcs: ack_no "<< packet.ack_no<<endl;
        cout << "sendtovcs: cmd_code "<< packet.cmd_code<<endl;
        cout << "sendtovcs: param_id "<< packet.param_id<<endl;
        cout << "sendtovcs: param_val "<< packet.param_val<<endl;
        cout << "sendtovcs: result_code "<< packet.result_code<<endl;
        cout << "sendtovcs: result_msg " << packet.result_msg<<endl;

        if ((bytecount = send(vcsd_sd,(char*)&packet, sizeof(message), 0)) == -1)
        {   
            fprintf(stderr, "Error sending data : %s\n", strerror(errno));
        }
        dtos = to_string(buffer[1]);
        set_ang += dtos;
        set_ang += "\n";
        packet = parse_handler((char*)set_ang.c_str());
        cout << "sendtovcs: seq_no "<< packet.seq_no<<endl;
        cout << "sendtovcs: ack_no "<< packet.ack_no<<endl;
        cout << "sendtovcs: cmd_code "<< packet.cmd_code<<endl;
        cout << "sendtovcs: param_id "<< packet.param_id<<endl;
        cout << "sendtovcs: param_val "<< packet.param_val<<endl;
        cout << "sendtovcs: result_code "<< packet.result_code<<endl;
        cout << "sendtovcs: result_msg " << packet.result_msg<<endl;

        if ((bytecount = send(vcsd_sd,(char*)&packet, sizeof(message), 0)) == -1)
        {   
            fprintf(stderr, "Error sending data : %s\n", strerror(errno));
        }

    }
}
