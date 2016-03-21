#include "session.h"

void start_session(session_t *sess){
	struct passwd* pw = getpwnam("nobody");
	if(pw == NULL)
		return;
	if(setegid(pw->pw_gid) <0 )
		ERR_EXIT("setegid");
	if(seteuid(pw->pw_uid) <0)
		ERR_EXIT("seteuid");
	int sockFd[2];//nobody�ͷ������ͨ�ŵ�socketPair
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
		sess->child_fd = sockFd[1];
		handle_child(sess);
	
	}
	else
	{
		//������
		//nobody����
		close(sockFd[1]);  //������ʹ��sockFd[0]���ӽ���ͨ��
		sess->parent_fd = sockFd[0];
		handle_parent(sess);
		
		
		
	}
		
}