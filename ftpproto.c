#include "ftpproto.h"
#include "sysutil.h"
#include "str.h"

void handle_child(session_t* sess)
{
	writen(sess->conn_fd,"220 (tinyFtpd)\r\n",strlen("220 (tinyFtpd)\r\n"));
	//ѭ����ȡ�ͻ���FTP����
	int ret;
	while(1)
	{
		memset(sess->cmdline,0,sizeof(sess->cmdline));
		memset(sess->cmd,0,sizeof(sess->cmd));
		memset(sess->arg,0,sizeof(sess->arg));
		//��ȡһ������
		ret = readline(sess->conn_fd,sess->cmdline,MAX_COMMAND_LINE);
		//��ȡһ��ʧ�ܣ�������ǰ����
		if(ret <0 )
			ERR_EXIT("readline");
		//ret==0 : �ͻ��˶Ͽ������ӣ�������ǰ����
		else if(ret ==0)
			exit(EXIT_SUCCESS);
		//ȥ��\r\n
		printf("cmdline = [%s]\n",sess->cmdline);
		str_trim_crlf(sess->cmdline);
		printf("cmdline = [%s]\n",sess->cmdline);
		str_split(sess->cmdline, sess->cmd, sess->arg,' ');
		printf("cmd = [%s]  arg = [%s]\n",sess->cmd,sess->arg);
		//����FTP���������
		
	}
}