#include"GetRequest.h"
#include"UrlCode.h"
#include<log.h>
#include<vector>
#include<assert.h>
std::string strJsFunResizeImg ="\
<script language=\"JavaScript\">\
		function resizeimg(obj,maxW,maxH)\
		{\
			 var imgW=obj.width;\
			 var imgH=obj.height;\
			 if(imgW>maxW||imgH>maxH)\
			 {       \
					  var ratioA=imgW/maxW;\
					  var ratioB=imgH/maxH;\
					  if(ratioA>ratioB)\
					  {\
							   imgW=maxW;\
							   imgH=imgH / ratioA;\
					  }\
					  else\
					  {\
							   imgH=maxH;\
							   imgW=imgW / ratioB;\
					  }\
					  obj.width=imgW;\
					  obj.height=imgH;\
			 }\
		}\
	</script>\
";
void list_dir_images(int client,const char * pszWorkDir,const char * url)
{
	sendheaders(client,url,"text/html",-1,-1,-1);
	struct dirent * entry = NULL;
	DIR * pDir = NULL;
    struct stat statbuf;
	std::string strData = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/></head>";
	send(client,strData.c_str(),strData.length(),0);
	strData = "<body>";
	send(client,strData.c_str(),strData.length(),0);
	pDir = opendir(pszWorkDir);
	std::vector<std::string> vFolders;
	std::vector<std::string> vImages;
	while(NULL!=(entry = readdir(pDir)))
	{
	  char szFullPath[1024];
	  sprintf(szFullPath,"%s%s",pszWorkDir,entry->d_name);
	  printf("get one item [%s]\n",szFullPath);
      lstat(szFullPath,&statbuf);
      if( S_ISDIR(statbuf.st_mode) )
      {
		  if(strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0)
          continue;
		 vFolders.push_back(szFullPath);
      }
      else
      {
		char * pExt = strstr(entry->d_name,".");
		if(pExt && (
		strcasecmp(pExt,".jpg")==0 ||
		strcasecmp(pExt,".jpeg")==0||
		strcasecmp(pExt,".png")==0||
		strcasecmp(pExt,".gif")==0))
		{
		 vImages.push_back(szFullPath);
		}	
      }
	}
	if(vImages.size() >0)
	{
		send(client,strJsFunResizeImg.c_str(),strJsFunResizeImg.length(),0);	
	}
	for(size_t i=0;i<vFolders.size();i++)
	{
		char * pName = strrchr(vFolders[i].c_str(),'/');
		if(pName)
		{
			char name[256];
			strcpy(name,pName+1);
			char szBuffer[2048];
			sprintf(szBuffer,"&nbsp;<A href=\"%s%s/\">%s</A>&nbsp;&nbsp;[DIR]<br/>"
			,url,name,name);
			send(client,szBuffer,strlen(szBuffer),0);
		}
	}
	if(vImages.size()>0)
	{
	strData = "<div align=\"center\">";
	send(client,strData.c_str(),strData.length(),0);
	strData = "<div id=\"imgbox\" style=\"width:1920px;height:1080px;border:1px solid #CCCCCC\">";
	send(client,strData.c_str(),strData.length(),0);
	}
	for(size_t i=0;i<vImages.size();i++)
	{
		char * pName = strrchr(vImages[i].c_str(),'/');
		if(pName)
		{
			char name[256];
			strcpy(name,pName+1);
			char szBuffer[2048];
			sprintf(szBuffer,"<a href='%s%s'><img src=%s%s onload=\"resizeimg(this,1200,800)\"></a>\n",url,name,url,name);
			send(client,szBuffer,strlen(szBuffer),0);	
		}
		else
		{
			printf("can't get filename of file :%s\n",vImages[i].c_str());
		}
	}
	if(vImages.size()>0)
	{
   		strData = "</div></div>";
    	send(client,strData.c_str(),strData.length(),0);
	}
	strData = "</body></html>";
	send(client,strData.c_str(),strData.length(),0);
}

void list_dir_items(int client,const char * pszWorkDir,const char * url)
{
	printf("list_dir_items调用...\n");
	sendheaders(client,url,"text/html",-1,-1,-1);
	struct dirent * entry = NULL;
	DIR * pDir = NULL;
    struct stat statbuf;
	std::string strData = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/></head>";
	send(client,strData.c_str(),strData.length(),0);
	strData = "<body>";
	send(client,strData.c_str(),strData.length(),0);
	pDir = opendir(pszWorkDir);
	while(NULL!=(entry = readdir(pDir)))
	{
	  char szFullPath[1024];
	  sprintf(szFullPath,"%s%s",pszWorkDir,entry->d_name);
	  printf("get one item [%s]\n",szFullPath);
      lstat(szFullPath,&statbuf);
      if( S_ISDIR(statbuf.st_mode) )
      {
		  if(strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0)
          continue;

        char szBuffer[2048];
		sprintf(szBuffer,"&nbsp;<A href=\"%s%s/\">%s</A>&nbsp;&nbsp;[DIR]<br/>"
		,url,entry->d_name, entry->d_name);
		send(client,szBuffer,strlen(szBuffer),0);
      }
      else
      {
            char szBuffer[2048];
			sprintf(szBuffer,"&nbsp;<A href=\"%s%s\">%s</A>&nbsp;&nbsp;%lld&nbsp;bytes<br/>"
			,url,entry->d_name,entry->d_name,statbuf.st_size );
			send(client,szBuffer,strlen(szBuffer),0);
      }
	}
	strData = "</body></html>";
	send(client,strData.c_str(),strData.length(),0);
	printf("list_dir_items调用结束!\n");
}
void cat(int client, FILE *resource,long long iFileOffset,long long iReadBytes) {
    //返回文件数据
	printf("cat 开始发送文件...\n");
    char buf[1024];	
  	if(0!=fseek(resource,iFileOffset,SEEK_SET))
	{
		WriteFormatLog(LOG_TYPE_ERROR,"fseek error:%s",strerror(errno));
	}
	long long iFinished = 0;
   	int iRead = fread(buf,sizeof(char),1024,resource); 
	do{
		if(iRead)
		{
			int iRev = send(client,buf,iRead,0);
			if(iRev ==-1)
			{
				printf("cat function Send erro occur:%s\n",strerror(errno));
				break;
			}
   			iFinished += iRead;
			if(iFinished == iReadBytes)
			{
				break;
			}
			iRead = fread(buf,sizeof(char),1024,resource); 
		}
		
	}while(iRead >0);
	printf("cat 发送文件调用结束!\n");
}

void serve_file(int client, const char *filename,long long iFileLen,long long iFileOffset,long long iReadBytes) {
    //返回文件数据
    printf("serve_file input data is[%s],iFileLen=%lld,iFileOffset=%lld,iReadBytes=%lld\n",filename,iFileLen,iFileOffset,iReadBytes);
	    //返回文件数据
    FILE *resource = NULL;
    resource = fopen(filename, "r");
    if (resource == NULL)
        not_found(client);
    else {
        //先返回头部信息
		std::string strType = getContentTypeFromFileName(filename);
		WriteFormatLog(LOG_TYPE_INFO,"file [%s] type return[%s]\n",filename,strType.c_str());
		sendheaders(client, filename,strType.c_str(),iFileLen,iFileOffset,iFileOffset + iReadBytes);
		WriteFormatLog(LOG_TYPE_INFO,"开始发送文件数据...");
		cat(client, resource,iFileOffset,iReadBytes);
		WriteFormatLog(LOG_TYPE_INFO,"文件数据发送完毕!");
    }
    fclose(resource);
}

int ProGetRequest(int iSockClient,const std::string &strUrl,const std::string & strHeaders)
{
    if (strcmp(strUrl.c_str(),"/") == 0) //根目录
	{
		chdir(g_strHomeDir.c_str());
	}
	string strDeCodeUrl = UrlDecode(strUrl);
	std::string strFullPath = g_strHomeDir + strDeCodeUrl;
	printf("get resource full path:%s,iSockClient:%d\n",strFullPath.c_str(),iSockClient);
	struct stat st;
	if(stat(strFullPath.c_str(),&st)==-1)
	{
		WriteFormatLog(LOG_TYPE_INFO,"%s路径无法找到",strFullPath.c_str());
		not_found(iSockClient);	
		return 0;
	}
	
	if(S_ISDIR(st.st_mode))
	{
		WriteFormatLog(LOG_TYPE_INFO,"枚举目录内的文件：%s",strFullPath.c_str());
		list_dir_items(iSockClient,strFullPath.c_str(),strUrl.c_str());
	//	list_dir_images(iSockClient,strFullPath.c_str(),strUrl.c_str());
		chdir(strFullPath.c_str());
		chdir(strFullPath.c_str());
	
	}
	else
	{
/*		char * pExt = strrchr(strFullPath.c_str(),'.');
		if(pExt && (strcasecmp(pExt,".flv")==0 ||strcasecmp(pExt,".mp4")==0))
		{
			FILE * resource = fopen("/Users/xiangzhenwei/video.html","r");
			char buf[1024];
			std::string strConfigData;
			int iRead = fread(buf,sizeof(char),1024,resource); 
			do{
				if(iRead)
				{
					strConfigData += buf;
   					iRead = fread(buf,sizeof(char),1024,resource); 
				}
			}while(iRead >0);
				
				sendheaders(iSockClient,"video.html","text/html");
				char * pData = (char*)malloc(strConfigData.length() + 2048);
				sprintf(pData,strConfigData.c_str(),"TestVideo","TestVideo",strUrl.c_str());
				send(iSockClient,pData,strlen(pData),0);
				free(pData);
				fclose(resource);
		}
		else
		{*/
			WriteFormatLog(LOG_TYPE_INFO,"serve_file 准备调用：%s",strFullPath.c_str());
			char * pRange = strstr(strHeaders.c_str(),"Range: bytes=");
			long long iFileOffset = 0;
			long long iReadBytes  = st.st_size;
			if(pRange)
			{
				pRange = pRange + strlen("Range: bytes=");
				char * pEnd = strstr(pRange,"\n");
				assert(pEnd!=NULL);
				pEnd = pEnd - 1;
				if(*pRange == '-') // -500 最后500个字节
				{
					sscanf(pRange,"-%lld",&iReadBytes);
					iFileOffset = st.st_size - iReadBytes;
				}
				else if(*pEnd == '-') // 500- 500字节以后的范围
				{
					 sscanf(pRange,"%lld-",&iFileOffset);
				//	 iReadBytes =  st.st_size - iFileOffset;
					 iReadBytes = 500;	
				}
				else // 100-500  100到500个字节之间的范围
				{
					long long iEnd = 0;
					sscanf(pRange,"%lld-%lld",&iFileOffset,&iEnd);
					iReadBytes = iEnd - iFileOffset;
				}	 	
			}
			serve_file(iSockClient,strFullPath.c_str(),st.st_size,iFileOffset,iReadBytes);
		//}
	}
	return 0;
}





