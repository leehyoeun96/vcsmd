#include "vcs_agent/vcs_agent.h"
namespace vcsAgent
{
    vAgent::vAgent()
    {
        client_sub = nh.subscribe("/twist_cmd",100,&vAgent::twistCallback,this);
        vcs_msg_sub = nh.subscribe("/vcs_msg",100,&vAgent::VCSstartupCallback,this);
        estop_sub = nh.subscribe("/mission_estop",100,&vAgent::estopCallback,this);

        agent_pub = nh.advertise<vcs_agent::vcs>("/vcs_ack", 10);
        graph_pub = nh.advertise<vcs_agent::mon>("/graph_value", 10);
        odom_pub = nh.advertise<nav_msgs::Odometry>("/vehicle/odom", 10);
    }
    vAgent::~vAgent()
    {

    }
    double vAgent::convert_mps_to_kmh(double linear_x)
    {
        return linear_x * 3.6;
    }

    void vAgent::sendtovcs(message *msg)
    {
        int bytecount = 0;
        if(msg->param_id == 16) buffer[0] = msg->param_val;//for printStatusBar
        if((msg->param_id == 2) && (msg->param_val == 1)) 
        {
            cout<<"send tvel"<<endl;
            becat = 1;//ecat on 
        }
        else if((msg->param_id == 1) || ((msg->param_id == 2) && (msg->param_val == 2)))
        {
            cout<<"can't send tvel"<<endl;
            becat = 0;//f or ecat off
        }
        
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
    void vAgent::estopCallback(const std_msgs::BoolConstPtr& msg)
    {
        if(msg->data == true)
        {
		estop_flag = true;
            if(!becat)
            {
                cout<<"ecat up/on first"<<endl;
                return;
            }
            message packet;

            string set_vel ("set cruisecontrol.target_velocity ");
            string dtos;

            dtos = to_string(-1);
            set_vel += dtos;
            set_vel += "\n";
            packet = parse_handler((char*)set_vel.c_str());
            sendtovcs(&packet);

        }
	else estop_flag = false;
    }

    void vAgent::VCSstartupCallback(const vcs_agent::Message1::ConstPtr& msg)
    {   
        message vcs_cmd_msg;
        vcs_cmd_msg.result_code = 0;
        if((msg->command).compare("vcs up") == 0)
        {
            FILE *fp = fopen(VCSUP_DIR,"r");

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
        else if((msg->command).compare("vcs on") == 0)
        {
            FILE *fp = fopen(VCSON_DIR,"r");

            //setting ecat num motors
            string set_num_motors ("set ecat.num_motors ");
            string dtos; 
            dtos = to_string(msg->value);
            set_num_motors += dtos;
            set_num_motors += "\n";
            vcs_cmd_msg = parse_handler((char*)set_num_motors.c_str());
            sendtovcs(&vcs_cmd_msg);
            //

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
        else if((msg->command).compare("vcs off") == 0)
        {
            FILE *fp = fopen(VCSOFF_DIR,"r");

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
        else if((msg->command).compare("vcs down") == 0)
        {
            FILE *fp = fopen(VCSDOWN_DIR,"r");

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
        else
        {
            cout<<"startup callback: msg->result_code "<< vcs_cmd_msg.result_code <<endl;
            string cmd = msg->command;
            cmd += "\n";
            vcs_cmd_msg = parse_handler((char*)cmd.c_str());
            sendtovcs(&vcs_cmd_msg);
        }
    }

    void vAgent::twistCallback(const geometry_msgs::TwistStampedConstPtr &input_msg)
    {
        if(!becat)
        {
            cout<<"ecat up/on first"<<endl;
            return;
        }
        if(estop_flag)
        {
            cout<<"estop on"<<endl;
            return;
        }
        message packet;

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

		double vAgent::convertSteeringAngleToAngularVelocity(const double cvel, const double cang)
		{
			//odom related parameters
			static const double RAD2DEG = 180.0 / M_PI;
			static const double DEG2RAD = M_PI / 180.0;

			static const double minimum_turning_radius = 5.3;
			static const double maximum_steering_angle = 462.5;
			static const double wheel_base = 2.65;
			static const double maximum_tire_angle_deg =
				(asin(wheel_base / minimum_turning_radius) * RAD2DEG); // deg
			//odom related paramteres end

			double current_tire_angle = maximum_tire_angle_deg * (cang / maximum_steering_angle) * DEG2RAD;
			double current_angular_velocity =
				tan(current_tire_angle) * cvel / wheel_base;
			return current_angular_velocity;
		}
		void vAgent::updateOdometry(const double vx, const double ang, const ros::Time &cur_time)
		{
			tf::TransformBroadcaster odom_broadcaster_;
			//
			static const double GwayAngle2SteeringAngle = 0.1055;
			double cur_steering_angle = ang * GwayAngle2SteeringAngle;
			std::cout << "vx, angle : " << vx << ' ' << cur_steering_angle << '\n';
			double vth = convertSteeringAngleToAngularVelocity(vx, cur_steering_angle);

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

			geometry_msgs::TransformStamped odom_trans;
			odom_trans.header.stamp = Time::now();
			odom_trans.header.frame_id = "odom";
			odom_trans.child_frame_id = "base_link";

			odom_trans.transform.translation.x = x;
			odom_trans.transform.translation.y = y;
			odom_trans.transform.translation.z = 0.0;
			odom_trans.transform.rotation = odom_quat;

			odom_broadcaster_.sendTransform(odom_trans);

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

    void vAgent::processRecvmsg(message *msg)
    {
        vcs_agent::mon mon_val;
        // cout << "processRecvmsg: result_msg "<< msg->result_msg<<endl;
        // cout << "processRecvmsg: param_val "<< msg->param_val<<endl;
        if(msg->param_id == 20) 
        {
            tmp_v = msg->param_val;
            //printStatusBar(buffer[0], tmp_v);
            mon_val.tvel = buffer[0];
            mon_val.cvel = tmp_v;
            graph_pub.publish(mon_val);
        }
        else if(msg->param_id == 21) tmp_a = msg->param_val;
        updateOdometry(tmp_v, tmp_a, ros::Time::now());
    }

    int vAgent::MainLoop()
    {
        vcs_agent::vcs vcs_ack;
        regcomp_all();
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
        becat = 0;
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
                        cout << "recvfromvcs: seq_no "<< read_buffer.seq_no<<endl;
                        cout << "recvfromvcs: ack_no "<< read_buffer.ack_no<<endl;
                        cout << "recvfromvcs: cmd_code "<<read_buffer.cmd_code<<endl;
                        cout << "recvfromvcs: param_id "<< read_buffer.param_id<<endl;
                        cout << "recvfromvcs: param_val "<< read_buffer.param_val<<endl;
                        cout << "recvfromvcs: result_code "<< read_buffer.result_code<<endl;
                        cout << "recvfromvcs: result_msg " << read_buffer.result_msg<<endl;
                        cout << "///////////////////"<<endl;
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
}
