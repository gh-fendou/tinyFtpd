#include "ftpproto.h"
#include "sysutil.h"
void handle_child(session_t* sess)
{
	writen(sess->conn_fd,"220 (tinyFtpd)\r\n",strlen("220 (tinyFtpd)\r\n"));
	//ѭ����ȡ�ͻ���FTP����
	while(1)
	{
		memset(sess->cmdline,0,sizeof(sess->cmdline));
		memset(sess->cmd,0,sizeof(sess->cmd));
		memset(sess->arg,0,sizeof(sess->arg));
		//��ȡһ������
		readline(sess->conn_fd,sess->cmdline,MAX_COMMAND_LINE);
		//����FTP���������
		
	}
}