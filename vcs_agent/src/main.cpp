#include "vcs_agent/vcs_agent.h" 
#define MAX_EVENTS 10 
#define BUF_SIZE 255 
#define IP_ADDR "192.168.0.4"
#define STARTUP_DIR "/home/qjin/catkin_ws/src/vcs_agent/start_up.txt"
int vcsmd_sd;
int vcsd_sd;
double buffer[2];
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

double convert_mps_to_kmh(double linear_x)
{
    return linear_x * 3.6;
}

void sendtovcs(message *msg)
{
    int bytecount = 0;
    if(msg->result_code == 0)
    { 
        cout << "sendtovcs: seq_no "<<msg->seq_no<<endl;
        cout << "sendtovcs: ack_no "<< msg->ack_no<<endl;
        cout << "sendtovcs: cmd_code "<< msg->cmd_code<<endl;
        cout << "sendtovcs: param_id "<< msg->param_id<<endl;
        cout << "sendtovcs: param_val "<<msg->param_val<<endl;
        cout << "sendtovcs: result_code "<< msg->result_code<<endl;
        cout << "sendtovcs: result_msg " << msg->result_msg<<endl;
        cout << "///////////////////"<<endl;

        if ((bytecount = send(vcsd_sd,(char*)msg, sizeof(message), 0)) == -1)
        {   
            fprintf(stderr, "Error sending data : %s\n", strerror(errno));
        }
    }
    else if(msg->result_code == -1)
        cout<<"sendtovcs: unknown command"<<endl;
}
void VCSstartupCallback(const vcs_agent::Message1::ConstPtr& msg)
{   
    message vcs_cmd_msg;
    vcs_cmd_msg.result_code = 0;
    if((msg->command).compare("start vcs") == 0)//start vcs
    {
        FILE *fp = fopen(STARTUP_DIR,"r");

        char file_buf[50]={0,};
        if(fp != NULL)
        {
            while(1)
            {   
                fgets(file_buf, sizeof(file_buf),fp);
                if(feof(fp)) break;
                vcs_cmd_msg = parse_handler(file_buf);
                sendtovcs(&vcs_cmd_msg);
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
        sendtovcs(&vcs_cmd_msg);
    }
    else
    {
        cout<<"startup callback: msg->result_code "<< vcs_cmd_msg.result_code <<endl;
        string cmd = msg->command;
        cmd += "\n";
        vcs_cmd_msg = parse_handler((char*)cmd.c_str());
        sendtovcs(&vcs_cmd_msg);
    }
}

void twistCallback(const geometry_msgs::TwistStampedConstPtr &input_msg)
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
    sendtovcs(&packet);
    
    dtos = to_string(buffer[1]);
    set_ang += dtos;
    set_ang += "\n";
    packet = parse_handler((char*)set_ang.c_str());
    sendtovcs(&packet);
}

double x;
double y;
double th;
ros::Time stamp;
ros::Publisher odom_pub;
void updateOdometry(const double vx, const double vth, const ros::Time &cur_time)
{
    double dt = (cur_time - stamp).toSec();
    if (dt > 100.0){
        stamp = cur_time;
        return;
    }
    double delta_x = (vx * cos(th)) * dt;
    double delta_y = (vx * sin(th)) * dt;
    double delta_th = vth * dt;

    ROS_INFO("dt : %lf delta(x y th) : (%lf %lf %lf)", dt, delta_x, delta_y, delta_th);

    x += delta_x;
    y += delta_y;
    th += delta_th;
    stamp = cur_time;

    geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(th);

    nav_msgs::Odometry odom;
    odom.header.stamp = Time::now();
    odom.header.frame_id = "odom";

    odom.pose.pose.position.x = x;
    odom.pose.pose.position.y = y;
    odom.pose.pose.position.z = 0.0;
    odom.pose.pose.orientation = odom_quat;

    odom.child_frame_id = "base_link";
    odom.twist.twist.linear.x = vx;
    odom.twist.twist.angular.z = vth;

    odom_pub.publish(odom);

}
void printStatusBar(double tvel, double cvel)
{
    int i;
    for (i = 0; i < (int)cvel; i++)
    {
        cout << light_green;
        cout << "0";
    }
    cout << endl;

    for (i = 0; i < (int)(tvel); i++)
    {
        cout << " ";
    }
    cout << yellow << "^\n";

    for (i = 0; i < (int)(tvel - 2); i++)
    {
        cout << " ";
    }
    cout << yellow << tvel;
    cout << white;
}
double tmp_v = 0;
double tmp_a = 0;
void processRecvmsg(message *msg)
{
   // cout << "processRecvmsg: result_msg "<< msg->result_msg<<endl;
   // cout << "processRecvmsg: param_val "<< msg->param_val<<endl;
    if(msg->param_id == 20) 
    {
        tmp_v = msg->param_val;
        printStatusBar(buffer[0], tmp_v);
    }
		else if(msg->param_id == 21) tmp_a = msg->param_val;
    if(tmp_v) updateOdometry(tmp_v, tmp_a, ros::Time::now());
}



int main(int argc, char**argv)
{
    ros::init(argc, argv,"vcs_agent");
    ros::NodeHandle nh;
    vcs_agent::vcs vcs_ack;

    regcomp_all();
    ros::Subscriber client_sub = nh.subscribe("/twist_cmd",1,twistCallback);
    ros::Subscriber vcs_msg_sub = nh.subscribe("/vcs_msg",1,VCSstartupCallback);

    ros::Publisher agent_pub = nh.advertise<vcs_agent::vcs>("/vcs_ack", 10);
    odom_pub = nh.advertise<nav_msgs::Odometry>("/vehicle/odom", 10);

    message read_buffer;

    struct sockaddr_in vcsmd_addr;//sock addr for vcsmd
    struct sockaddr_in vcsd_addr;//sock addr for vcs

    //connect to vcsmd//
    vcsmd_sd = socket(PF_INET, SOCK_STREAM, 0);

    memset(&vcsmd_addr, 0, sizeof(vcsmd_addr));
    vcsmd_addr.sin_family = PF_INET;
    vcsmd_addr.sin_port = htons(9002);
    inet_aton(IP_ADDR, &vcsmd_addr.sin_addr);

    if(connect(vcsmd_sd, (struct sockaddr *)&vcsmd_addr, sizeof(vcsmd_addr)) ==-1)
    {
        printf("connect error: %s\n", strerror(errno));
        return 0;
    }
    close(vcsmd_sd);

    //connect to vcsd//
    sleep(1);
    vcsd_sd = socket(PF_INET, SOCK_STREAM, 0);

    memset(&vcsd_addr, 0, sizeof(vcsd_addr));
    vcsd_addr.sin_family = AF_INET;
    vcsd_addr.sin_port = htons(9000);
    inet_aton(IP_ADDR, &vcsd_addr.sin_addr);

    if(connect(vcsd_sd, (struct sockaddr *)&vcsd_addr, sizeof(struct sockaddr_in)) ==-1)
    {
        printf("connect error with vcsd\n");
        return 0;
    }

    printf("connected...\n");

    //epoll setting//
    int i;
    int str_len = 0;
    int event_cnt;
    int epfd;
    struct epoll_event ev, evs[MAX_EVENTS];

    epfd=epoll_create1(0);
    ev.events = EPOLLIN;
    ev.data.fd = vcsd_sd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, vcsd_sd, &ev);

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
                str_len = read(evs[i].data.fd,&read_buffer,sizeof(read_buffer));
                if(str_len==0){
                    epoll_ctl(epfd,EPOLL_CTL_DEL,evs[i].data.fd,NULL);
                    close(evs[i].data.fd);
                    printf("Closed client: %d\n",evs[i].data.fd);
                }
                else{
/*                    cout << "recvfromvcs: seq_no "<< read_buffer.seq_no<<endl;
                    cout << "recvfromvcs: ack_no "<< read_buffer.ack_no<<endl;
                    cout << "recvfromvcs: cmd_code "<<read_buffer.cmd_code<<endl;
                    cout << "recvfromvcs: param_id "<< read_buffer.param_id<<endl;
                    cout << "recvfromvcs: param_val "<< read_buffer.param_val<<endl;
                    cout << "recvfromvcs: result_code "<< read_buffer.result_code<<endl;
                    cout << "recvfromvcs: result_msg " << read_buffer.result_msg<<endl;
                    cout << "///////////////////"<<endl;*/
                    if(read_buffer.cmd_code == 0)
                        processRecvmsg(&read_buffer);
                    vcs_ack.seq_no = read_buffer.seq_no;
                    vcs_ack.ack_no = read_buffer.ack_no;
                    vcs_ack.cmd_code = read_buffer.cmd_code;
                    vcs_ack.param_id = read_buffer.param_id;
                    vcs_ack.param_val = read_buffer.param_val;
                    vcs_ack.result_code = read_buffer.result_code;
                    vcs_ack.result_msg = read_buffer.result_msg;
                    agent_pub.publish(vcs_ack);
                }
            }
        }
    }
    epoll_ctl(epfd,EPOLL_CTL_DEL,vcsd_sd,NULL);
    regfree_all();
    close(vcsd_sd);
    return 0;
}
