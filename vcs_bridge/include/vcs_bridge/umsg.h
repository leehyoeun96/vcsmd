typedef struct _message
{
    int seq_no;
    int ack_no;
    int cmd_code;//0 means get / 1 means set
    int param_id;//0 means target velocity/ 1 means target omega//temporally
    double param_val;//used as reply for get
    int result_code;//result of cmd(0 means success)
    char result_msg[100];//infor/warning/error msg
}message;
