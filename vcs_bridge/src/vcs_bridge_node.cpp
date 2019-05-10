#include "vcs_bridge/vcs_bridge.h" 
#define MAX_EVENTS 10 
#define BUF_SIZE 255 
#define IP_ADDR "127.0.0.1"

int main(int argc, char**argv)
{
    ros::init(argc, argv,"vcs_bridge");
    ros::NodeHandle nh;
    vcs_bridge::vcs vcs_ack;

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
                    cout << "recvfromvcs: seq_no "<< read_buffer.seq_no<<endl;
                    cout << "recvfromvcs: ack_no "<< read_buffer.ack_no<<endl;
                    cout << "recvfromvcs: cmd_code "<<read_buffer.cmd_code<<endl;
                    cout << "recvfromvcs: param_id "<< read_buffer.param_id<<endl;
                    cout << "recvfromvcs: param_val "<< read_buffer.param_val<<endl;
                    cout << "recvfromvcs: result_code "<< read_buffer.result_code<<endl;
                    cout << "recvfromvcs: result_msg " << read_buffer.result_msg<<endl;
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
