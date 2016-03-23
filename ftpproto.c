#include "ftpproto.h"
#include "sysutil.h"
#include "str.h"
#include "ftpcodes.h"
#include "tunable.h"
#include "privsock.h"


//�ж��Ƿ�PORT or PASVģʽ�ѿ���
int port_active(session_t* sess);
int pasv_active(session_t* sess);

int get_port_fd(session_t* sess);
int get_pasv_fd(session_t* sess);
//�������������׽��֣�����0ʧ�� 1�ɹ�
int get_transfer_fd(session_t* sess);
//�г���ǰĿ¼
int list_common(session_t* sess);
//�������ݶ�Ӧ����status ������Ӧ�ı�����
void ftp_reply(session_t* sess,int status,const char* text);
//�������ݶ�Ӧ����status �����-���ŵ���Ӧ�ı�����
void ftp_lreply(session_t* sess,int status,const char* text);


//static void do_user(session_t *sess);
//static void do_pass(session_t *sess);
static void do_cwd(session_t *sess);
static void do_cdup(session_t *sess);
static void do_quit(session_t *sess);
static void do_port(session_t *sess);
static void do_pasv(session_t *sess);
static void do_type(session_t *sess);
//static void do_stru(session_t *sess);
//static void do_mode(session_t *sess);
static void do_retr(session_t *sess);
static void do_stor(session_t *sess);
static void do_appe(session_t *sess);
static void do_list(session_t *sess);
static void do_nlst(session_t *sess);
static void do_rest(session_t *sess);
static void do_abor(session_t *sess);
static void do_pwd(session_t *sess);
static void do_mkd(session_t *sess);
static void do_rmd(session_t *sess);
static void do_dele(session_t *sess);
static void do_rnfr(session_t *sess);
static void do_rnto(session_t *sess);
static void do_site(session_t *sess);
static void do_syst(session_t *sess);
static void do_feat(session_t *sess);
static void do_size(session_t *sess);
static void do_stat(session_t *sess);
static void do_noop(session_t *sess);
static void do_help(session_t *sess);
typedef struct ftpcmd
{
	const char *cmd;
	void (*cmd_handler)(session_t *sess);
} ftpcmd_t;


static ftpcmd_t ctrl_cmds[] = {
	/* ���ʿ������� */
	{"USER",	do_user	},
	{"PASS",	do_pass	},
	{"CWD",		do_cwd	},
	{"XCWD",	do_cwd	},
	{"CDUP",	do_cdup	},
	{"XCUP",	do_cdup	},
	{"QUIT",	do_quit	},
	{"ACCT",	NULL	},
	{"SMNT",	NULL	},
	{"REIN",	NULL	},
	/* ����������� */
	{"PORT",	do_port	},
	{"PASV",	do_pasv	},
	{"TYPE",	do_type	},
	{"STRU",	/*do_stru*/NULL	},
	{"MODE",	/*do_mode*/NULL	},

	/* �������� */
	{"RETR",	do_retr	},
	{"STOR",	do_stor	},
	{"APPE",	do_appe	},
	{"LIST",	do_list	},
	{"NLST",	do_nlst	},
	{"REST",	do_rest	},
	{"ABOR",	do_abor	},
	{"\377\364\377\362ABOR", do_abor},
	{"PWD",		do_pwd	},
	{"XPWD",	do_pwd	},
	{"MKD",		do_mkd	},
	{"XMKD",	do_mkd	},
	{"RMD",		do_rmd	},
	{"XRMD",	do_rmd	},
	{"DELE",	do_dele	},
	{"RNFR",	do_rnfr	},
	{"RNTO",	do_rnto	},
	{"SITE",	do_site	},
	{"SYST",	do_syst	},
	{"FEAT",	do_feat },
	{"SIZE",	do_size	},
	{"STAT",	do_stat	},
	{"NOOP",	do_noop	},
	{"HELP",	do_help	},
	{"STOU",	NULL	},
	{"ALLO",	NULL	}
};

void handle_child(session_t* sess)
{
	ftp_reply(sess,FTP_GREET,"(tinyFtpd)");
	
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
		
		/* if(strcmp("USER",sess->cmd) ==0 )
		{
			
			do_user(sess);
		}
		else if(strcmp("PASS",sess->cmd) ==0)
		{
			do_pass(sess);
		} */
		int i=0;
		int size = sizeof(ctrl_cmds)/sizeof(ctrl_cmds[0]);
		while(i<size)
		{
			if(strcmp(ctrl_cmds[i].cmd , sess->cmd) ==0)
			{
				//���ҵ���Ӧcmd�������handle��NULL,��˵����ʵ�֣���ʼ��������
				if(ctrl_cmds[i].cmd_handler != NULL)
					ctrl_cmds[i].cmd_handler(sess);
				//δʵ�ֶ�Ӧhandle����
				else
				{
					ftp_reply(sess,FTP_COMMANDNOTIMPL,"unimplement command");
				}
				break;
			}
			
			i++;
		}
		//��������δ�ҵ���Ӧ���δʶ��
		if(i == size)
		{
			ftp_reply(sess,FTP_BADCMD,"unknown command");
		}
	}
}
//��nobody����GET_DATA_SOCK����  client�˿ں�  IP��ַ
//�ɹ�����1  ʧ�ܷ���0
int get_port_fd(session_t* sess)
{
		priv_sock_send_cmd(sess->child_fd,PRIV_SOCK_GET_DATA_SOCK);
		unsigned short port = ntohs(sess->port_addr->sin_port);
		char* ip = inet_ntoa(sess->port_addr->sin_addr);
		priv_sock_send_int(sess->child_fd, (int)port);
		priv_sock_send_buf(sess->child_fd, ip, strlen(ip));
		
		//��ȡӦ��
		char res = priv_sock_get_result(sess->child_fd);
		if(res == PRIV_SOCK_RESULT_BAD )
		{
			return 0;
		}
		else if(res == PRIV_SOCK_RESULT_OK)
		{
			sess->data_fd = priv_sock_recv_fd(sess->child_fd);
			return 1;
		}
		return 1;
}

int get_pasv_fd(session_t* sess)
{
	
	
}
int port_active(session_t* sess)
{
	//printf("----------");
	if(sess->port_addr!= NULL)
	{
		if(pasv_active(sess))
		{
			fprintf(stderr,"both port and pasv are active!\r\n");
			exit(EXIT_FAILURE);
		}
		return 1;
	}
		
	return 0;
} 
int pasv_active(session_t* sess)
{
	if(sess->listen_fd != -1)
	{
		if(port_active(sess))
		{
			fprintf(stderr,"both port and pasv are active!\r\n");
			exit(EXIT_FAILURE);
		}
		return 1;
	}
	return 0;
}
int get_transfer_fd(session_t* sess)
{
	//PORT����PASV��û�յ�
	if(!port_active(sess) && !pasv_active(sess))
	{
		ftp_reply(sess,FTP_BADSENDCONN,"USE PORT or PASV first");
		return 0;
	}
	int ret =1;
	//����ģʽ
	if(port_active(sess))
	{	
		//ʧ���򷵻�0
		if( get_port_fd(sess) == 0 )
			ret =0;
		
		/* 
		int fd = tcp_client(0);
		if(connect_timeout(fd,sess->port_addr,tunable_connect_timeout)<0)
		{
			close(fd);
			return 0;
		}
		sess->data_fd = fd; */
	}
	if(pasv_active(sess))
	{
		int fd = accept_timeout(sess->listen_fd,NULL,tunable_accept_timeout);
		close(sess->listen_fd);
		//����ʧ��
		if( fd == -1)
		{
			return 0;
		}
		sess->data_fd = fd;
	}
	if(sess->port_addr)
	{
		free(sess->port_addr);
		sess->port_addr = NULL;
	}
	return ret;
}
//LIST ����Ӧ����
int list_common(session_t* sess)
{
	DIR* dir = opendir(".");
	if(dir ==NULL)
	{
		return 0;
	}
	struct dirent* dt;
	struct stat sbuf;
	while( (dt = readdir(dir))!=NULL )
	{
		if(lstat(dt->d_name,&sbuf) < 0)
		{
			continue;
		}
		//���������ļ�����ͷ.�ŵ�
		if(dt->d_name[0] == '.')
			continue;
		char perms[] = "----------";
		perms[0] = '?';
		//��ȡ�ļ�����,Ȩ����Ϣ����perms
		mode_t mode = sbuf.st_mode;
		switch(mode & S_IFMT)
		{
		case S_IFREG:
			perms[0] = '-';
			break;
		case S_IFLNK:
			perms[0] = 'l';
			break;
		case S_IFSOCK:
			perms[0] = 's';
			break;
		case S_IFBLK:
			perms[0] = 'b';
			break;
		case S_IFDIR:
			perms[0] = 'd';
			break;
		case S_IFCHR:
			perms[0] = 'c';
			break;
		case S_IFIFO:
			perms[0] = 'p';
			break;
		}
		//ӵ���߶�дִ��Ȩ��
		if(mode & S_IRUSR)
		{
			perms[1] = 'r';
		}
		if(mode & S_IWUSR)
		{
			perms[2] = 'w';
		}
		if(mode & S_IXUSR)
		{
			perms[3] = 'x';
		}
		//���û���дִ��Ȩ��
		if(mode & S_IRGRP)
		{
			perms[4] = 'r';
		}
		if(mode & S_IWGRP)
		{
			perms[5] = 'w';
		}
		if(mode & S_IXGRP)
		{
			perms[6] = 'x';
		}
		//�����û���дִ��Ȩ��
		if(mode & S_IROTH)
		{
			perms[7] = 'r';
		}
		if(mode & S_IROTH)
		{
			perms[8] = 'w';
		}
		if(mode & S_IROTH)
		{
			perms[9] = 'x';
		}
		//����Ȩ�ޣ���uid
		if(mode & S_ISUID)
		{
			perms[3] = (perms[3]=='x')?'s':'S';
		}
		if(mode & S_ISGID)
		{
			perms[6] = (perms[6]=='x')?'s':'S';
		}
		if(mode & S_ISVTX)
		{
			perms[9] = (perms[9]=='x')?'s':'S';
		}
		
		//��ӡ�����buf��
		char buf[1024] = {0};
		//@off ��ǰ������
		int off =0;
		off += sprintf(buf,"%s ",perms);//�ļ����ͺ�Ȩ��λ����buf
		off +=sprintf(off + buf,"%3d %-8d %-8d",sbuf.st_nlink,sbuf.st_uid,sbuf.st_gid);//������ uid gid����buf
		off +=sprintf(off + buf,"%8lu ",sbuf.st_size);//�ļ���С����buf
		
		/* ��ȡʱ�䵽buf��
		�����޸�ʱ�䣬�ϴ��޸�ʱ��������ڰ������ϵ���ʾ��ݣ�������ʾ����24Сʱ��ʱ�� */
		char* p_date_format = "%b %e %H:%M";//ʱ���ʽ
		//ȡ��ǰʱ��
		struct timeval tv;
		gettimeofday(&tv,NULL);
		
		time_t local_time = tv.tv_sec;
		if(sbuf.st_mtime > local_time || (local_time-sbuf.st_mtime)>182*24*3600)
		{
			//man strftime
			p_date_format = "%b %e %Y";
		}
		
		char datebuf[64] ={0};
		struct tm* p_tm = localtime(&local_time);
		strftime(datebuf,sizeof(datebuf),p_date_format,p_tm);
		
		off +=sprintf(off + buf,"%s ",datebuf);//��ʽ��ʱ�����buf
		
		//����Ƿ��������ļ�Ҫ����ָ��
		if(S_ISLNK(sbuf.st_mode))
		{
			char tmp[1024] = {0};
			readlink(dt->d_name,tmp,sizeof(tmp));
			off +=sprintf(off + buf,"%s -> %s\r\n",dt->d_name,tmp);//���������ļ���->����ָ��
		}
		else
		{
			off +=sprintf(off + buf,"%s\r\n",dt->d_name);//�ļ�������buf
		}
		writen(sess->data_fd,buf,strlen(buf));
	}
	closedir(dir);
	return 1;
}
void ftp_reply(session_t* sess,int status,const char* text)
{
	char buf[1024] = {0};
	sprintf(buf,"%d %s\r\n",status,text);
	writen(sess->conn_fd,buf,strlen(buf));
}
void ftp_lreply(session_t* sess,int status,const char* text)
{
	char buf[1024] = {0};
	sprintf(buf,"%d-%s\r\n",status,text);
	writen(sess->conn_fd,buf,strlen(buf));
}
//����USER
static void do_user(session_t* sess)
{   struct passwd* pw = getpwnam(sess->arg);
	if(pw == NULL)
	{	//�û�������
		ftp_reply(sess,FTP_LOGINERR,"login incorrect");
		return;
	}
	sess->uid = pw->pw_uid;
	ftp_reply(sess,FTP_GIVEPWORD,"please specify the password");
	
}
//����PASS
static void do_pass(session_t* sess)
{
	struct passwd* pw = getpwuid(sess->uid);
	if(pw == NULL)
	{	//�û�������
		ftp_reply(sess,FTP_LOGINERR,"login incorrect");
		return;
	}
	struct spwd* sp = getspnam(pw->pw_name);
	if(sp == NULL)
	{	//�û���Ӧ���벻����
		ftp_reply(sess,FTP_LOGINERR,"login incorrect");
		return;
	}
	//��������ͨ��crypt�������м��ܣ�����spwd�н�����бȽ�,��spwd->sp_pwdp�����Ӽ���
	//ʹ�ú���char *crypt(const char *key, const char *salt); salt����
	char* encrypted_pass = crypt(sess->arg,sp->sp_pwdp);
	//��֤�����Ƿ�ƥ��
	if(strcmp(encrypted_pass,sp->sp_pwdp) !=0)
	{	//���벻ƥ��
		ftp_reply(sess,FTP_LOGINERR,"login incorrect");
		return;
	}
	//��¼��֤�ɹ���ǰ���̸�Ϊ��¼�û�����
	setegid(pw->pw_gid);
	seteuid(pw->pw_uid);
	chdir(pw->pw_dir);
	ftp_reply(sess,FTP_LOGINOK,"230 login sucessful");
}

static void do_cwd(session_t *sess)
{
}
static void do_cdup(session_t *sess)
{}
static void do_quit(session_t *sess)
{}
static void do_port(session_t *sess)
{
	 /*
	 ����IP�Ͷ˿ںţ���������������Ự�ṹ
	 */
	 unsigned int tmp[6];
	 //arg 192,168,44,1,9,159��ʽ������tmp�У�ip�Ͷ˿ں�
	 sscanf(sess->arg,"%u,%u,%u,%u,%u,%u,",&tmp[2],&tmp[3],&tmp[4],&tmp[5],&tmp[0],&tmp[1]);
	 sess->port_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	 memset(sess->port_addr,0,sizeof(struct sockaddr_in));
	 sess->port_addr->sin_family = AF_INET;
	 
	 unsigned char* p = (unsigned char*)&sess->port_addr->sin_port;
	 p[0] = tmp[0];
	 p[1] = tmp[1];
	 p = (unsigned char*)&sess->port_addr->sin_addr;
	 p[0] = tmp[2];
	 p[1] = tmp[3];
	 p[2] = tmp[4];
	 p[3] = tmp[5];
	 ftp_reply(sess,FTP_PORTOK,"PORT command successful,Consider using PASV");

}
static void do_pasv(session_t *sess)
{
	//227 Entering Passive Mode (192,168,44,128,139,222).
	char ip[16] = {0};
	getlocalip(ip);
	sess->listen_fd= tcp_server(ip,0);
	
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	//��ȡ����sockfd��Ϣ
	if(getsockname(sess->listen_fd,(struct sockaddr*)&addr,&addrlen) < 0)
	{
		ERR_EXIT("getsockname");
	}
	//��ӦPASV���͵�port��ip��Ϊ�����ֽ���
	unsigned short port = ntohs(addr.sin_port);
	unsigned int v[4];
	sscanf(ip,"%u.%u.%u.%u",&v[0],&v[1],&v[2],&v[3]);
	char text[1024] = {0};
	sprintf(text,"Entering Passive Mode (%u,%u,%u,%u,%u,%u).",v[0],v[1],v[2],v[3],port>>8,port&0xFF);
	
	ftp_reply(sess,FTP_PASVOK,text);
	
}
static void do_type(session_t *sess)
{
	if(strcmp(sess->arg,"A") ==0 )
	{
		sess->is_ascii = 1;
		ftp_reply(sess,FTP_TYPEOK,"Switching to ASCII mode.");
	}
	else if(strcmp(sess->arg,"I") ==0 )
	{
		sess->is_ascii = 0;
		ftp_reply(sess,FTP_TYPEOK,"Switching to Binary mode.");
	}
	else
	{
		ftp_reply(sess,FTP_BADCMD,"Unrecognized TYPE command.");
	}
}
//static void do_stru(session_t *sess);
//static void do_mode(session_t *sess);
static void do_retr(session_t *sess)
{}
static void do_stor(session_t *sess)
{}
static void do_appe(session_t *sess)
{}
static void do_list(session_t *sess)
{
	//������������
	//��Ӧ150 FTP_DATACONN
	//�����ļ��б�
	//�ر������׽���
	//��Ӧ226 FTP_TRANSFEROK
	if( (get_transfer_fd(sess)) == 0)
	{
		return;
	}
	ftp_reply(sess,FTP_DATACONN,"Here comes the directory listing.");
	if( list_common(sess) ==0)
		return;
	close(sess->data_fd);
	sess->data_fd = -1;
	ftp_reply(sess,FTP_TRANSFEROK,"Directory send OK.");
}
static void do_nlst(session_t *sess)
{}
static void do_rest(session_t *sess)
{}
static void do_abor(session_t *sess)
{}
static void do_pwd(session_t *sess)
{
	char text[1024] = {0};
	char dir[1024+1] = {0};
	getcwd(dir,1024);
	sprintf(text,"\"%s\"",dir);
	ftp_reply(sess,FTP_PWDOK,text);
	
}
static void do_mkd(session_t *sess)
{}
static void do_rmd(session_t *sess)
{}
static void do_dele(session_t *sess)
{}
static void do_rnfr(session_t *sess)
{}
static void do_rnto(session_t *sess)
{}
static void do_site(session_t *sess)
{}
static void do_syst(session_t *sess)
{
	ftp_reply(sess,FTP_SYSTOK,"UNIX Type: L8");
}
static void do_feat(session_t *sess)
{
	
	ftp_lreply(sess,FTP_FEAT,"Features:");
	writen(sess->conn_fd," EPRT\r\n",sizeof(" EPRT\r\n"));
	writen(sess->conn_fd," EPSV\r\n",sizeof(" EPSV\r\n"));
	writen(sess->conn_fd," MDTW\r\n",sizeof(" MDTW\r\n"));
	writen(sess->conn_fd," PASV\r\n",sizeof( " PASV\r\n"));
	writen(sess->conn_fd," REST_STREAM\r\n",sizeof(" REST_STREAM\r\n"));
	writen(sess->conn_fd," SIZE\r\n",sizeof(" SIZE\r\n"));
	writen(sess->conn_fd," TVFS\r\n",sizeof(" TVFS\r\n"));
	writen(sess->conn_fd," UTF8\r\n",sizeof(" UTF8\r\n"));
	writen(sess->conn_fd," EPRT\r\n",sizeof(" EPRT\r\n"));
	writen(sess->conn_fd," EPRT\r\n",sizeof(" EPRT\r\n"));
	ftp_reply(sess,FTP_FEAT,"End\r\n"); 
}
static void do_size(session_t *sess)
{}
static void do_stat(session_t *sess)
{}
static void do_noop(session_t *sess)
{}
static void do_help(session_t *sess)
{}