#include "nobody.h"
#include "sysutil.h"
void handle_parent(session_t* sess)
{
	char cmd;
	//���շ����ӽ��̵�socket��Ϣ������
	while(1)
	{
		read(sess->parent_fd,&cmd,1);
		//������������
	}
}