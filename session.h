#ifndef _SESSION_H
#define _SESSION_H
#include "common.h"
typedef struct session
{
	//control connection
	uid_t uid;
	int conn_fd;
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];
	//process communicate
	//���ӽ���ͨ�ŵ�sockfd
	int parent_fd;
	int child_fd;
	
	//Э��״̬
	//�Ƿ�asciiģʽ
	int is_ascii;
} session_t;
void start_session(session_t *sess);


#endif //_SESSION_H_