#include "ftpproto.h"
#include "sysutil.h"
#include "str.h"

//�������ݶ�Ӧ����status ������Ӧ�ı�����
void ftp_reply(session_t* sess,int status,const char* text);
static void do_user(session_t* sess);
static void do_pass(session_t* sess);


void handle_child(session_t* sess)
{
	ftp_reply(sess->conn_fd,FTP_GREET,"(tinyFtpd)");
	
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
		str_trim_crlf(sess->cmdline);
		printf("cmdline = [%s]\n",sess->cmdline);
		//������cmd  arg
		str_split(sess->cmdline, sess->cmd, sess->arg,' ');
		printf("cmd = [%s]  arg = [%s]\n",sess->cmd,sess->arg);
		//��������FTP���������
		//����ת��Ϊ��д
		str_upper(sess->cmd);
		
		if(strcmp("USER",sess->cmd) ==0 )
		{
			
			do_user(sess);
		}
		else if(strcmp("PASS",sess->cmd) ==0)
		{
			do_pass(sess);
		}
	}
}

void ftp_reply(session_t* sess,int status,const char* text)
{
	char buf[1024] = {0};
	sprintf(buf,"%d %s\r\n",status,text);
	writen(sess->conn_fd,buf,strlen(buf);
}

//����USER
static void do_user(session_t* sess)
{   struct passwd* pw = getpwnam(sess->arg);
	if(pw == NULL)
	{
		ftp_reply(sess->conn_fd,FTP_LOGINERR,"login incorrect");
	}
	ftp_reply(sess->conn_fd,FTP_GIVEPWORD,"please specify the password");
	
}
//����PASS
static void do_pass(session_t* sess)
{
	ftp_reply(sess->conn_fd,FTP_LOGINOK,"230 login sucessful");
}