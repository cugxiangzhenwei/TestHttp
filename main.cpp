#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string>
#include <dirent.h>
#include<errno.h>
#include<pthread.h>
typedef enum
{
 HTTP_GET =0,
 HTTP_POST,
 HTTP_PUT,
 HTTP_NONE
}HTTP_METHOD;

 HTTP_METHOD GetMethod(const std::string & strHeader)
{
	const char * patterns[] = {"GET","POST","PUT",NULL};
	HTTP_METHOD hm = HTTP_NONE;
	for(int i=0; patterns[i]!=NULL;i++)
	{
		if(strncmp(patterns[i],strHeader.c_str(),strlen(patterns[i]))==0)
		{
			hm = (HTTP_METHOD)(i);		
			break;
		}	
	}
	return hm;
}
std::string get_oneline(int iSocket)
{	
	int iRead = 0;
    char c='\0';
	int i= 0;
	char * pData = (char*)malloc(1024);
	while(c!='\n')
	{
		iRead = recv(iSocket,&c,1,0);
		pData[i] = c;
		if(iRead >0 && c == '\r')
		{
			iRead = recv(iSocket,&c,1,MSG_PEEK);
			if(iRead >0 && c == '\n')
				iRead = recv(iSocket,&c,1,0);
			else
				c = '\n';
					
			pData[i] = '\0';
			break;				
		}
		i++;	
	}
	std::string str = pData;
	free(pData);
	return str;
}
void * ThreadFuncClient(void *arg)
{
    int * pSocket = (int*)(arg);
	int iSocket = *pSocket;	
	std::string strHeader;
	std::string str;
	do
	{
		str = get_oneline(iSocket);
		strHeader +=str;
		strHeader += "\n";
	}while(!str.empty());
	printf("%s\n",strHeader.c_str());
	HTTP_METHOD hm = GetMethod(strHeader);
	printf("HTTP请求类型:%d\n",hm);
	close(iSocket);
	return NULL;
}
int main(int argc,char *argv[])
{
	if(argc <2)
	{
		printf("缺少端口号参数!\n");
		return 0;
	}
	int iport = atoi(argv[1]);
	int iSockSvr = socket(PF_INET,SOCK_STREAM,0);
	if(iSockSvr==0)
	{
		printf("failed to create socket,file:%s,line:%d,function:%s\n",
		__FILE__,__LINE__,__func__);
		return 0;
	}
	struct sockaddr_in svrAddr;
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_port = htons(iport);	
	svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int iRev = bind(iSockSvr,(struct sockaddr *)&svrAddr,socklen_t(sizeof(svrAddr)));
	if(iRev == -1)
	{
		printf("bind port %d failed:%s ,file:%s,line:%d,function:%s\n",
			iport,strerror(errno),__FILE__,__LINE__,__func__);
		return 0;
	}
	iRev = listen(iSockSvr,5);
	if(iRev == -1)
	{
		printf("listen failed:%s ,file:%s,line:%d,function:%s\n",
			strerror(errno),__FILE__,__LINE__,__func__);
		return 0;
	}
	while(1)
	{
		struct sockaddr addrClient;
		socklen_t iSockLen = (socklen_t)(sizeof(addrClient));
		int iSockClient = accept(iSockSvr,(struct sockaddr *)(&addrClient),& iSockLen);	
		if(addrClient.sa_family == AF_INET)
		{
			struct sockaddr_in * pClientAddr = (struct sockaddr_in *)(&addrClient);
			printf("client ip:%s,port:%d,sock:%d\n",inet_ntoa(pClientAddr->sin_addr),
				ntohs(pClientAddr->sin_port),iSockClient);
		}
		else if(addrClient.sa_family == AF_INET6)
		{
			struct sockaddr_in6 *pClientAddr = (struct sockaddr_in6 *)(&addrClient);
			char szIp[1024];
		 	const char * pszIp = inet_ntop(AF_INET6,&(pClientAddr->sin6_addr),szIp,
				sizeof(char)*1024);
			printf("client ip:%s,port:%d,sock:%d\n",pszIp,
				ntohs(pClientAddr->sin6_port),iSockClient);
		}
		pthread_t  pt;
		iRev = pthread_create(&pt,NULL,ThreadFuncClient,&iSockClient);
	}
	close(iSockSvr);
}
