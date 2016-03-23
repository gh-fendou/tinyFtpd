#include "session.h"
#include "privsock.h"
#include "nobody.h"
#include "ftpproto.h"

void start_session(session_t *sess){
	
	
	//if(setegid(pw->pw_gid) <0 )
		//ERR_EXIT("setegid");
	//if(seteuid(pw->pw_uid) <0)
		//ERR_EXIT("seteuid");
	/* int sockFd[2];//nobody�ͷ������ͨ�ŵ�socketPair
	if(socketpair(PF_UNIX,SOCK_STREAM,0,sockFd) < 0 )
		ERR_EXIT("socketair"); */
	 priv_sock_init(sess);
	
	pid_t pid;
	pid = fork();
	if(pid < 0)
		ERR_EXIT("son fork");
	
	if(pid ==0)
	{
		//�ӽ���
		//����ftp���ݴ���Ľ���
		priv_sock_set_child_context(sess);
		handle_child(sess);
	
	}
	else
	{
		
		//������
		//nobody����
		priv_sock_set_parent_context(sess);
		handle_parent(sess);
		
		
		
	}
		
}