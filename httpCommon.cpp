#include"httpCommon.h"
#include<unistd.h>
std::string g_strHomeDir;
void SetHomeDir(const char * pszStrHomeDir)
{
	if(pszStrHomeDir == NULL)
	{
		char buf[256];
		char * pReturn = getcwd(buf,256);
		if(pReturn!=NULL)
			g_strHomeDir = buf;
	}
	else
		g_strHomeDir = pszStrHomeDir;

	printf("设置工作目录为：%s\n",g_strHomeDir.c_str());
}
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
std::string GetURL(const std::string & strHeader)
{
	if(strHeader.size()<=3)
	{	
		printf("GetURL param strHeader[%s] is error,\
		file:%s,line:%d,function:%s\n",strHeader.c_str(),__FILE__,__LINE__,__func__);
		return "";
	}
//	printf("GetURL:%d before while\n",__LINE__);
	int iPos = 3; // from third character to find url
	while(IsSpace(strHeader[iPos]))
 		iPos ++; // skip blank space
	
//	printf("GetURL:%d after while\n",__LINE__);
	std::string strUrl(1024,'\0');	
	int iLen = 0;
	while(!IsSpace(strHeader[iPos]))
	{
		strUrl[iLen++] = strHeader[iPos];
 		iPos ++; // skip not blank character
	}
	strUrl[iLen] = '\0';
	return strUrl;
}
std::string get_oneline(int iSocket,bool & bError)
{	
	int iRead = 0;
    char c='\0';
	int i= 0;
//	printf("get_oneline开始获取http头一行数据...\n");
	char * pData = (char*)malloc(1024);
	bError = false;
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
		else if(iRead ==-1)
		{
			printf("get_oneline ,recv failed :%s,retun:%d\n",strerror(errno),iRead);
			bError = true;
			break;
		}
		else if(iRead == 0)
		{
			printf("the connection is closed by client!\n");
			bError = true;
			break;
		}	
		i++;	
	}
	if(bError)
		return "";

//	printf("get_oneline获取http头一行数据结束！\n");
	std::string str = pData;
	free(pData);
	return str;
}
void sendheaders(int client, const char */*filename*/,const char *pszFileType,long long iStreamLen,long long iRangeBegin,long long iRangeEnd) {
    //先返回文件头部信息
	printf("sendheaders开始发送http响应头\n");
    char buf[1024];
	bool bIsPartial = false;
    if(iStreamLen >0 &&  (iRangeEnd - iRangeBegin) != iStreamLen)
		bIsPartial = true;
	if(!bIsPartial)
		strcpy(buf, "HTTP/1.0 200 OK\r\n");
	else
	{
		strcpy(buf,"HTT/1.0 206 Partial Content\r\n");
		printf("http response code 206:partial Content\n");
	}
    int iRev = send(client, buf, strlen(buf), 0);
	if(iRev < 0)
	{
		printf("send failed return:%d,error:%s\n",iRev,strerror(errno));
		return;
	}
    strcpy(buf, SERVER_STRING);
   	iRev =  send(client, buf, strlen(buf), 0);
	if(iRev < 0)
	{
		printf("send failed return:%d,error:%s\n",iRev,strerror(errno));
		return;
	}
    sprintf(buf, "Content-Type: %s\r\n",pszFileType);
    iRev = send(client, buf, strlen(buf), 0);
	if(iRev < 0)
	{
		printf("send failed return:%d,error:%s\n",iRev,strerror(errno));
		return;
	}
	
//	std::string strTypeStream = "application/octet-stream";
	sprintf(buf,"Content-Length: %lld\r\n",iStreamLen);
	iRev = send(client,buf,strlen(buf),0);
	if(iRev < 0)
	{
		printf("send failed return:%d,error:%s\n",iRev,strerror(errno));
		return;
	}
	strcpy(buf,"Accept-Ranges: bytes\r\n");
	iRev =send(client,buf,strlen(buf),0);
	if(iRev < 0)
	{
		printf("send failed return:%d,error:%s\n",iRev,strerror(errno));
		return;
	}
	strcpy(buf,"ETag: \"2f38a6cac7cec51:160c\"\r\n");
	iRev = send(client,buf,strlen(buf),0);
	if(iRev < 0)
	{
		printf("send failed return:%d,error:%s\n",iRev,strerror(errno));
		return;
	}
	if(bIsPartial)
	{
		sprintf(buf,"Content-Range: bytes %lld-%lld/%lld",iRangeBegin,iRangeEnd,iStreamLen);	
    	iRev = send(client,buf,strlen(buf),0);
		if(iRev < 0)
		{
			printf("send failed return:%d,error:%s\n",iRev,strerror(errno));
			return;
		}
		printf("%s\n",buf);
	}
    strcpy(buf, "\r\n");
 	iRev = send(client, buf, strlen(buf), 0);
	if(iRev < 0)
	{
		printf("send failed return:%d,error:%s\n",iRev,strerror(errno));
		return;
	}
	printf("发送http响应头完毕!\n");
}
void not_found(int client) {
    //返回http 404协议
    char buf[1024];
    //先发送http协议头
    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    //再发送serverName
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    //再发送Content-Type
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    //发送换行符
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    //发送html主体内容
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}



