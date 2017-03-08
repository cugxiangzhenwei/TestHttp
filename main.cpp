#include<pthread.h>
#include"httpCommon.h" 
#include"GetRequest.h"
#include"PostRequest.h"
#include<log.h>
#include<signal.h>
void * ThreadFuncClient(void *arg)
{
    int * pSocket = (int*)(arg);
	int iSocket = *pSocket;	
	sigset_t signal_mask;
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGPIPE);
	int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error/n");
	} 
	std::string strHeader;
	std::string str;
	printf("开始获取http请求头...\n");
	bool bError = false;
	do
	{
		str = get_oneline(iSocket,bError);
		if(bError)
			break;

		strHeader +=str;
		strHeader += "\n";
	}while(!str.empty());
	printf("获取http请求头完毕！\n");
	printf("%s",strHeader.c_str());
	if(bError)
	{
		close(iSocket);
		printf("获取请求头发生错误，结束客户请求！\n");
		return NULL;
	}
	HTTP_METHOD hm = GetMethod(strHeader);
	printf("HTTP请求类型:%d\n",hm);
	std::string strUrl = GetURL(strHeader);
	printf("HTTP请求URL:%s\n",strUrl.c_str());
	if(hm == HTTP_GET)
	{
		ProGetRequest(iSocket,strUrl,strHeader);		
	}
	else if(hm == HTTP_POST)
	{
		ProPostRequest(iSocket,strUrl,strHeader);	
	}
	printf("处理完成一次客户端请求!\n\n");
	close(iSocket);
	return NULL;
}

void PrintHelp()
{
	printf("请指定端口号和工作路径！\n示例:\n./s.exe -p 5000 -d /Usr/xiangzhenwei\n");
}
int bind_socket(int iport)
{
	int iListenfd = socket(PF_INET,SOCK_STREAM,0);
	if(iListenfd ==-1)
	{
		fprintf(stderr,"error socket:%s\n",strerror(errno));
		return -1;
	}
	struct sockaddr_in addr;
	bzero(&addr,sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;		
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//	inet_pton(AF_INET,ip,&addr.sin_addr);
	int iRev = bind(iListenfd,(struct sockaddr *)&addr,(socklen_t)sizeof(addr));
	if(iRev ==-1)
	{
		fprintf(stderr,"error bind to port %d,%s\n",iport,strerror(errno));
		return -1;
	}		
	return iListenfd;
}
void PrintClientInfo(const struct sockaddr & addrClient,int iSockClient)
{
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

}
int main(int argc,char *argv[])
{
	if(argc <3)
	{
		PrintHelp();
		return 0;
	}
	int i=1;
	int iport = -1;
	std::string strHome = "";
	while(argv[i]!=NULL)
	{
		if(strcasecmp(argv[i],"-p")==0 && i < argc -1)
		{
			i++;
			iport =  atoi(argv[i]);
		}	
		else if(strcasecmp(argv[i],"-d")==0 && i < argc -1)
		{
			i++;
			strHome = argv[i];
		}
		else
			i++;
	}
	if(iport <0 || strHome.empty())
	{
		printf("端口号或工作目录参数不正确!\n");
		PrintHelp();
		return 0;
	}
	WriteFormatLog(LOG_TYPE_INFO,"server启动，端口号：%d,工作目录:%s\n",iport,strHome.c_str());
	SetHomeDir(strHome.c_str());
	//忽略SIGPIPE信号的方法,避免socket关闭后，发送数据SIGPIPE信号导致进程退出
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;//设定接受到指定信号后的动作为忽略
	sa.sa_flags = 0;
	if (sigemptyset(&sa.sa_mask) == -1 || //初始化信号集为空
	sigaction(SIGPIPE, &sa, 0) == -1) { //屏蔽SIGPIPE信号
	perror("failed to ignore SIGPIPE; sigaction");
	exit(EXIT_FAILURE);
	}
	int iSockSvr = bind_socket(iport);
	if(iSockSvr==-1)
		return 0;	
	int iRev = listen(iSockSvr,5);
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
		int iClientFd = accept(iSockSvr,(struct sockaddr *)(&addrClient),& iSockLen);	
		PrintClientInfo(addrClient,iClientFd);
		pthread_t  pt;
		iRev = pthread_create(&pt,NULL,ThreadFuncClient,&iClientFd);
	}
	close(iSockSvr);
}
