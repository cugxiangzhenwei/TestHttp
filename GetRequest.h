#include"httpCommon.h"
/*
*@brief GET类型的http请求处理入口函数
*@param iSockClient 客户端Socket描述符
*@param strUrl 客户端请求的资源URL,相对地址
*@param  strHeaders 客户端请求的http头，可从中获取必要的字段值
*/
int ProGetRequest(int iSockClient,const std::string &strUrl,const std::string &/* strHeaders*/);

