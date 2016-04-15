#ifndef _SESSION_H
#define _SESSION_H
#include "common.h"
typedef struct session
{
	//����ģʽ�±���������ipֵ
	char localip[20];
	//control connection
	uid_t uid;
	int conn_fd;
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];
	
	//�������Ӳ���
	struct sockaddr_in* port_addr;
	//���ݴ����׽��� ��������client�ɹ���/����client�������Ӻ�������
	int data_fd;
	//�����׽��� ��������client���� ����
	int listen_fd;
	//process communicate 
	//���ӽ���ͨ�ŵ�sockfd
	int parent_fd;
	int child_fd;
	
	/*Э��״̬*/
	//�Ƿ�asciiģʽ 
	int is_ascii;
	//�ϵ�����
	long long restart_pos;
	//������RNFR
	char* rnfr_name;
	//�����ñ���
	unsigned int uplaod_rate_max;
	unsigned int download_rate_max;
	long start_sec;//��ʼ����ʱ��
	long start_usec;
	
	
} session_t;
void start_session(session_t *sess);


#endif //_SESSION_H_