#pragma once
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
typedef enum
{
 HTTP_GET =0,
 HTTP_POST,
 HTTP_PUT,
 HTTP_NONE
}HTTP_METHOD;


#define SERVER_STRING "Server:向振伟开发服httpd/0.1.1\r\n"

/* http 工作目录*/
extern std::string g_strHomeDir;

inline bool IsSpace(char c)
 {
	return ((int)(c)==32);
 }
/*设置http工作目录 */
void SetHomeDir(const char * pszStrHomeDir);
/*
*从header中判断请求的http类型，是GET，PUT还是其它的类型
 */
HTTP_METHOD GetMethod(const std::string & strHeader);
/*
*从header中获取请求的URL地址
*/
std::string GetURL(const std::string & strHeader);
/*从客户端socket描述符中获取一行数据，读到\r\n结束,返回一行数据，出错时，bError被设置为true*/
std::string get_oneline(int iSocket,bool & bError);
/*
* @brief 发送http响应头
* @param client 客户端socket描述符
* @param filename 请求的资源文件名
* @param pszFileType 资源类型，例如"text/html" "text/plain"等
* @param iStreamLen 消息头后面跟从要发送的实体数据的字节数,如果后续发送的是html，则该值使用默认值-1
* @return 无返回值
*/
void sendheaders(int client, const char *filename,const char *pszFileType,long long iStreamLen = -1,long long iRangeBegin=-1,long long iRangeEnd = -1);

/* 发送请求，通知客户端页面无法找到，响应码为404*/
void not_found(int client);
/*
* @brief 根据后缀名获取请求资源的类型，用于填充http响应头中的Content-Type字段
* @param pszFilename 请求的资源文件名
* @return 返回Content-Type类型
*/
std::string getContentTypeFromFileName(const char* pszFileName);
