/*
* Socket wrapper for FLASCC 
*/

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socketwrapper.h"

#include <emscripten/emscripten.h>
#include <cstdlib>
#include <cstring>

extC char* js_gethostbyname(const char*);
extC int js_socket();
extC void js_bind(int socket, const char* host, int port);
extC char* js_getsockname(int socket);
extC int js_getsockport(int socket);
extC void js_close(int socket);
extC int js_select();
extC void js_sendto(int socket, const char* host, int port, const char* byteArr, int length);
extC char* js_recvfrom(int socket, int length);
extC char* js_recvfromgethost();
extC int js_recvfromgetport();


extC int	wrapaccept(int socket,  struct sockaddr * serv_addr, socklen_t *addrlen){
	printf("wrapaccept sock=%d\n",socket);
	/*inline_as3(
		"import FlashSocket.Socketmanager;\n"
		"%0 = Socketmanager.getInstance().socket();\n"
		: "=r"(socket):
	);
    if (serv_addr->sa_family == AF_INET){
        struct sockaddr_in* sa =(sockaddr_in*) serv_addr;
		int port = ntohs( sa->sin_port );
		const char* host = inet_ntoa(sa->sin_addr);
		inline_as3(
			"import FlashSocket.Socketmanager;\n"
			"Socketmanager.getInstance().connect(%0,CModule.readString(%2, %3),%1);\n"
			: :"r"(socket),"r"(port),"r"(host),"r"(strlen(host))
		);
		
		return socket;
	
	} else {
		printf("\nSocketwrapper connect !AF_INET not implemented");
		return -1;
	}*/
	return 0;
}

extC int wrapbind(int socket, const struct sockaddr * serv_addr, socklen_t addrlen)
{
	printf("wrapbind sock=%d\n",socket);
    if (serv_addr->sa_family == AF_INET)
    {
        struct sockaddr_in* sa =(sockaddr_in*) serv_addr;
		int port = ntohs( sa->sin_port );
		const char* host = inet_ntoa(sa->sin_addr);
		
		printf("wrapbind hostname=%s, port=%d\n",host, port);
		js_bind(socket, host, port);
		/*AS3_DeclareVar(hostname, String);
		AS3_CopyCStringToVar(hostname, host, strlen(host));
		inline_as3(
			"import flash.external.ExternalInterface;\n"
			"var result = ExternalInterface.call(\"bind\", %0, hostname, %1);"
						 : : "r"(socket), "r"(port));*/
		return 0;	
	}
	else
	{
		printf("\nSocketwrapper connect !AF_INET not implemented");
		return -1;
	}
	return 0;
}

extC int wrapconnect(int socket, const struct sockaddr *serv_addr, socklen_t addrlen){
	printf("wrapconnect sock=%d\n",socket);
    /*if (serv_addr->sa_family == AF_INET){
        struct sockaddr_in* sa =(sockaddr_in*) serv_addr;
		int port = ntohs( sa->sin_port );
		const char* host = inet_ntoa(sa->sin_addr);
		inline_as3(
			"import FlashSocket.Socketmanager;\n"
			"Socketmanager.getInstance().connect(%0,CModule.readString(%2, %3),%1);\n"
			: :"r"(socket),"r"(port),"r"(host),"r"(strlen(host))
		);
		return 0;
	
	} else {
		printf("\nSocketwrapper connect !AF_INET not implemented");
		return -1;
	}*/
	return 0;
}
extC int	wrapgetpeername(int socket, struct sockaddr * name, socklen_t * namelen){
	printf("\nwrapgetpeername  sock=%d\n",socket);
	/*if (name->sa_family == AF_INET){
		struct sockaddr_in* sa =(sockaddr_in*) name;
		char *addr;
		int port;
		inline_as3(
			"import FlashSocket.Socketmanager;\n"
			"var stringptr:int = CModule.mallocString(Socketmanager.getInstance().getSockHost(%1));\n"
			"CModule.write32(%2, stringptr);\n"
			"%0 = Socketmanager.getInstance().getSockPort(%1);\n"
		:"=r"(port):"r"(socket),"r"(&addr)
		);
		sa->sin_port=htons(port);
		sa->sin_addr.s_addr =inet_addr(addr);
		return 0;
	} else {
		printf("\nSocketwrapper getsockname !AF_INET not implemented");
	}*/
	return -1;
}

extC int	wrapgetsockname(int socket, struct sockaddr *name, socklen_t *namelen)
{
	printf("wrapgetsockname\n");
	struct sockaddr_in* sa =(sockaddr_in*) name;
	sa->sin_family = AF_INET;
	char *addr;
	int port;
	
	/*inline_as3(
		"import flash.external.ExternalInterface;\n"
		"var result = ExternalInterface.call(\"getsockname\", %0);" : : "r"(socket)); 																	
	
	AS3_MallocString(addr, result);*/
	addr = js_getsockname(socket);
	printf("sockname=%s\n", addr);
	
	/*inline_as3(
		"import flash.external.ExternalInterface;\n"
		"%0 = ExternalInterface.call(\"getsockport\", %1);" : "=r"(port) : "r"(socket));*/
	port = js_getsockport(socket);
	printf("sockport=%d\n", port);
	
	sa->sin_port=htons(port);
	sa->sin_addr.s_addr =inet_addr(addr);
	return 0;
}

extC int	wrapgetsockopt(int, int, int, void * __restrict, socklen_t * __restrict){
	printf("\nSocketwrapper getsockopt no need implement for Flash");
	return 0;
}

extC int	wraplisten(int s, int)
{
	printf("wraplisten\n");
/*	inline_as3(
		"import FlashSocket.Socketmanager;\n"
		"Socketmanager.getInstance().sockListen(%0);\n"
		: :"r"(s)
	);*/
	return 0;
}

extC ssize_t wraprecv(int s, void *buf, size_t len, int flags)
{
	printf("wraprecv\n");
	/*int length = 0;
	inline_as3(
		"import FlashSocket.Socketmanager;\n"
		"import flash.utils.ByteArray;\n"
		"var buf:ByteArray = new ByteArray();\n"
		"buf = Socketmanager.getInstance().recvdata(%2);\n"
		"%0 = buf.length;\n"
		"CModule.writeBytes(%1, buf.length, buf);\n"
		:"=r"(length):"r"((int)buf),"r"(s)
	);
	return (ssize_t)length;*/
	return 0;
}

int nRcvLength = -1;
extC void rcvLength(int length)
{
	nRcvLength = length;
}

extC ssize_t wraprecvfrom(int socket, void *buf, size_t len, int flags, struct sockaddr * serv_addr, socklen_t * addrlen)
{
	//printf("wraprecvfrom\n");
	struct sockaddr_in* sa = (sockaddr_in*) serv_addr;
	sa->sin_family = AF_INET;
	int length = 0;
	char * host = NULL;
	int port = 0;
	/*inline_as3(
		"import flash.external.ExternalInterface;\n"
		"import flash.utils.ByteArray;\n"
		//"import mx.utils.Base64Encoder;\n"
		"var bytes:ByteArray = new ByteArray();\n"
		"var result:String = ExternalInterface.call(\"recvfrom\", %1, %3);\n"
		"trace(\"recieve result is \" + result)\n"
		"if(result==null)\n"
		"{\n"
		"	%0=0\n"
		"}\n"
		"else\n"
		"{\n"
		//"var answer:Array= result.split(\",\");\n"
		"bytes.writeUTFBytes(result)
		//"for(var i:int = 0; i < result.length; ++i)\n"
		//"{\n"
		//"	bytes.writeByte(int(result[i]))\n"
		//"}\n"
		"bytes.position=0\n"
		"if(result.length==0)\n"
		"{\n"
		"	%0=0\n"
		"}\n"
		"else\n"
		"{\n"
		"	%0=bytes.length\n"
		"}\n"
		"CModule.writeBytes(%2, bytes.length, bytes);\n"
		"}\n"
		 : "=r"(length) : "r"(socket), "r"((int)buf), "r"((int)len)
		//"trace(\"data string = \" + tmp)\n"
		//"var b64:Base64Encoder = new Base64Encoder();"
    	//"b64.encodeBytes(bytes);"

	);
	printf("\nwraprecvfrom len:%d\n",length);
	inline_as3(
		"import flash.external.ExternalInterface;\n"
		"var resHost:String = ExternalInterface.call(\"recvfromgethost\")\n");	
	AS3_MallocString(host, resHost);
	printf("host=%s\n", host);
	
	inline_as3(
		"import flash.external.ExternalInterface;\n"
		"var resPort:int = ExternalInterface.call(\"recvfromgetport\")\n");			
	AS3_GetScalarFromVar(port, resPort);
	printf("port=%d\n", port);*/
	
	char *res = js_recvfrom(socket, len);
	if(res != NULL && nRcvLength != -1)
	{
		host = js_recvfromgethost();
		port = js_recvfromgetport();
		memcpy(buf, res, (size_t)nRcvLength);
		length = nRcvLength;
		nRcvLength = -1;
	}
	else
	{
		length = 0;
	}
	
	sa->sin_port=htons(port);
	sa->sin_addr.s_addr = inet_addr(host);
	if (length==0){
		length=-1;
	}
	return (ssize_t)length;
}

extC ssize_t	wraprecvmsg(int, struct msghdr *, int){
	printf("\nSocketwrapper recvmsg not implemented");
	return -1;
}
extC ssize_t wrapsend(int s, const void * buf, size_t len, int flags){
	printf("wrapsend sock=%d\n",s);
	/*printf("\n");
	inline_as3(
		"import FlashSocket.Socketmanager;\n"
		"import flash.utils.ByteArray;\n"
		"var bytes:ByteArray = new ByteArray();\n"
		"CModule.readBytes(%1, %2,bytes);\n"
		"Socketmanager.getInstance().senddata(%0,bytes);\n"
		: :"r"(s),"r"((int)buf),"r"((uint)len)
	);
	
	return len;*/
	return 0;
}
extC ssize_t wrapsendto(int socket, const void * buf,  size_t len,
						 int flags, const struct sockaddr * serv_addr, socklen_t addrlen)
{
	struct sockaddr_in* sa =(sockaddr_in*) serv_addr;
	int port = ntohs( sa->sin_port );
	const char* host = inet_ntoa(sa->sin_addr);
	//printf("\nwrapsendto sock=%d addr=%s:%d\n",socket,host,port);
	/*char* t = (char*)buf;
	for(int i = 0; i < len; ++i)
	{
		printf("%d\n", t[i]);
	}*/
	js_sendto(socket, host, port, (const char*)buf, len);
	/*AS3_DeclareVar(hostname, String);
	AS3_CopyCStringToVar(hostname, host, strlen(host));

	inline_as3(
		"import flash.external.ExternalInterface;\n"
		"import flash.utils.ByteArray;\n"
		"var bytes:ByteArray = new ByteArray();\n"
		"CModule.readBytes(%2, %3,bytes);\n"
		"bytes.position = 0;\n"		
    	"var a:Array=new Array(bytes.length);\n"
		"for(var i:int=bytes.length;i-->0;)\n"
		"{\n"
  		"	a[i]=bytes[i];\n"
  		"}\n"
		"var result = ExternalInterface.call(\"sendto\", %0, hostname, %1, a);"
		 :: "r"(socket), "r"(port), "r"((int)buf),"r"((uint)len)
	);	
	return len;*/
	//printf("wrapsendto sock=%d\n",socket);
	return len;
}

extC ssize_t	wrapsendmsg(int, const struct msghdr *, int){
	printf("\nSocketwrapper sendmsg not implemented");
	return -1;
}

extC int	wrapsetsockopt(int, int, int, const void *, socklen_t){
	printf("\nSocketwrapper setsockopt not implemented");
	return 0;
}
extC int	wrapshutdown(int, int){
	printf("\nSocketwrapper shutdown not implemented");
	return -1;
}
extC int	wrapsockatmark(int){
	printf("\nSocketwrapper sockatmark not implemented");
	return -1;
}

extC int wrapsocket(int domain, int type, int protocol)
{
	printf("wrapsocket\n");
/*	int result=-1;
	inline_as3(
		"import FlashSocket.Socketmanager;\n"
		"%0 = Socketmanager.getInstance().socket();\n"
		: "=r"(result):
	);
	printf(" sock=%d\n",result);
	return result;*/
	
	int result = -1;
	/*inline_as3(
		"import flash.external.ExternalInterface;\n"
		"var res = ExternalInterface.call(\"socket\");");
	AS3_GetScalarFromVar(result, res);*/
	result = js_socket();
	printf("sock=%d\n",result);
	return result;
}

extC int	wrapsocketpair(int, int, int, int *){
	printf("\nSocketwrapper socketpair not implemented");
	return -1;
}

extC int wrapclose(int s)
{
	printf("wrapclose sock=%d\n",s);
/*	inline_as3(
		"import flash.external.ExternalInterface;\n"
		"ExternalInterface.call(\"close\", %0);" :: "r"(s)
	);*/
	/*inline_as3(
		"import FlashSocket.Socketmanager;\n"
		"Socketmanager.getInstance().closesock(%0);\n"
		: :"r"(s)
	);*/
	js_close(s);
	return 1;
}

extC int wrapselect(int, fd_set *, fd_set *, fd_set *, struct timeval * tv)
{
	//printf("wrapselect\n");
	int res = 0;
	/*inline_as3(
		"import flash.external.ExternalInterface;\n"
		"var result = ExternalInterface.call(\"select\", 1);");
	AS3_GetScalarFromVar(res, result);*/
	/*inline_as3(
		"import FlashSocket.Socketmanager;\n"
		"%0 = Socketmanager.getInstance().selectsock();\n"
		: "=r"(res):
	);*/
	res = js_select();
	//printf("select returns %d\n", res);
	return res;
}

extC void simpletest(char* t)
{
    printf("text taken %s\n", t);
}

extC struct hostent *wrapgethostbyname(const char * name){
	//printf("wrapgethostbyname %s\n", name);
	int LIST_LEN = 2;
	struct hostent *he = new hostent();
	char *ip = js_gethostbyname(name);
	/*emscripten_run_script("\
		var ip = document.udpflash.getHostByName(hostname);\
		var testF = Module['cwrap']('simpletest', '', ['string']);\
		testF('sdfgsdfgsfdgsfgsdfg');\
		");*/
	/*AS3_DeclareVar(hostname, String);
	AS3_CopyCStringToVar(hostname, name, strlen(name));
	inline_as3(
		"import flash.external.ExternalInterface;\n"
		"var result = ExternalInterface.call(\"gethostbyname\", hostname);"
	);
	char *ip = NULL;
	AS3_MallocString(ip, result);*/
	//printf("ip=%s\n", ip);
	
    he->h_addrtype = AF_INET;
	he->h_length = sizeof(int);
	he->h_name = (char *)name;
    struct in_addr *inp = new in_addr();
   	inet_aton(ip, inp);
    struct in_addr **list= (in_addr**)malloc(LIST_LEN * sizeof(in_addr*));
    list[0]=inp;
    list[1]=NULL;
    he->h_addr_list=(char**)list;
	return he;
}

