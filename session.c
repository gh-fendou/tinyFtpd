#include "session.h"

void start_session(session_t *sess){
	int sockFd[2];
	if(socketpair(PF_UNIX,SOCK_STREAM,0,sockFd) < 0 )
		ERR_EXIT("socketair");
	
	pid_t pid;
	pid = fork();
	if(pid < 0)
		ERR_EXIT("son fork");
	
	if(pid ==0)
	{
		//�ӽ���
		//����ftp���ݴ���Ľ���
		close(sockFd[0]);  //�ӽ���ʹ��sockFd[1]�븸����ͨ��
		handle_child(sess);
	
	}
	else
	{
		//������
		//nobody����
		close(sockFd[1]);  //������ʹ��sockFd[0]���ӽ���ͨ��
		handle_parent(sess);
		
		
		
	}
		
}