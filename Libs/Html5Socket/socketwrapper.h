#ifndef _SOCKET_WRAPPER_H_
#define	_SOCKET_WRAPPER_H_

#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#ifdef __cplusplus
	#define  extC extern "C" 
#else
	#define extC
#endif



extC int wrapclose(int s);
/*#undef close
#define HTML5_close(a) wrapclose(a)
#define BW_close_0() close()
#define close_GET_MACRO(a, macro, ...) macro
#define close_MACRO_CHOOSER(...) close_GET_MACRO(__VA_ARGS__, HTML5_close,BW_close_0 )
#define close(...) close_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define close(a) wrapclose(a)

extC int wrapaccept(int, struct sockaddr * __restrict, socklen_t * __restrict);
/*#undef accept
#define HTML5_accept(a,b,c) wrapaccept(a,b,c)
#define BW_accept_0() accept()
#define BW_accept_1(a) accept(a)
#define BW_accept_2(a,b) accept(a,b)
#define accept_GET_MACRO(a, b, c, macro, ...) macro
#define accept_MACRO_CHOOSER(...) accept_GET_MACRO(__VA_ARGS__, HTML5_accept,BW_accept_2,BW_accept_1,BW_accept_0 )
#define accept(...) accept_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define accept(a,b,c) wrapaccept(a,b,c)

extC int wrapbind(int, const struct sockaddr *, socklen_t);
/*#undef bind
#define HTML5_bind(a,b,c) wrapbind(a,b,c)
#define BW_bind_0() bind()
#define BW_bind_1(a) bind(a)
#define BW_bind_2(a,b) bind(a,b)
#define bind_GET_MACRO(a, b, c, macro, ...) macro
#define bind_MACRO_CHOOSER(...) bind_GET_MACRO(__VA_ARGS__, HTML5_bind,BW_bind_2,BW_bind_1,BW_bind_0 )
#define bind(...) bind_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define bind(a,b,c) wrapbind(a,b,c)

extC int wrapconnect(int, const struct sockaddr *, socklen_t);
/*#undef connect
#define HTML5_connect(a,b,c) wrapconnect(a,b,c)
#define BW_connect_2(a,b) connect(a,b)
#define connect_GET_MACRO(a, b, c, macro, ...) macro
#define connect_MACRO_CHOOSER(...) connect_GET_MACRO(__VA_ARGS__, HTML5_connect,BW_connect_2,, )
#define connect(...) connect_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define connect(a,b,c) wrapconnect(a,b,c)

extC int wrapgetpeername(int, struct sockaddr * __restrict, socklen_t * __restrict);
/*#undef getpeername
#define getpeername(a,b,c) wrapgetpeername(a,b,c)*/

extC int wrapgetsockname(int, struct sockaddr * __restrict, socklen_t * __restrict);
/*#undef getsockname
#define getsockname(a,b,c) wrapgetsockname(a,b,c)*/

extC int wrapgetsockopt(int, int, int, void * __restrict, socklen_t * __restrict);
/*#undef getsockopt
#define getsockopt(a,b,c,d,e) wrapgetsockopt(a,b,c,d,e)*/

extC int wraplisten(int, int);
/*#undef listen
#define HTML5_listen(a,b) wraplisten(a,b)
#define BW_listen_1(a) listen(a)
#define listen_GET_MACRO(a, b, macro, ...) macro
#define listen_MACRO_CHOOSER(...) listen_GET_MACRO(__VA_ARGS__, HTML5_listen,BW_listen_1,)
#define listen(...) listen_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define listen(a,b) wraplisten(a,b)

extC ssize_t wraprecv(int, void *, size_t, int);
/*#undef recv
#define HTML5_recv(a,b,c,d) wraprecv(a,b,c,d)
#define BW_recv_0() recv()
#define BW_recv_1(a) recv(a)
#define BW_recv_2(a,b) recv(a,b)
#define recv_GET_MACRO(a, b, c, d, macro, ...) macro
#define recv_MACRO_CHOOSER(...) recv_GET_MACRO(__VA_ARGS__, HTML5_recv,,BW_recv_2,BW_recv_1,BW_recv_0)
#define recv(...) recv_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define recv(a,b,c,d) wraprecv(a,b,c,d)

extC ssize_t wraprecvfrom(int, void *, size_t, int, struct sockaddr * , socklen_t * );
/*#undef recvfrom
#define HTML5_recvfrom(a,b,c,d,e,f) wraprecvfrom(a,b,c,d,e,f)
#define BW_recvfrom_3(a,b,c) recvfrom(a,b,c)
#define BW_recvfrom_4(a,b,c,d) recvfrom(a,b,c,d)
#define recvfrom_GET_MACRO(a, b, c, d, e, f, macro, ...) macro
#define recvfrom_MACRO_CHOOSER(...) recvfrom_GET_MACRO(__VA_ARGS__, HTML5_recvfrom,,BW_recvfrom_4,BW_recvfrom_3,,,)
#define recvfrom(...) recvfrom_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define recvfrom(a,b,c,d,e,f) wraprecvfrom(a,b,c,d,e,f)

extC ssize_t wraprecvmsg(int, struct msghdr *, int);
/*#undef recvmsg
#define HTML5_recvmsg(a,b,c) wraprecvmsg(a,b,c)
#define BW_recvmsg() recvmsg()
#define recvmsg_GET_MACRO(a, b, c, macro, ...) macro
#define recvmsg_MACRO_CHOOSER(...) recvmsg_GET_MACRO(__VA_ARGS__, HTML5_recvmsg,,,BW_recvmsg )
#define recvmsg(...) recvmsg_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define recvmsg(a,b,c) wraprecvmsg(a,b,c)

extC ssize_t wrapsend(int, const void *, size_t, int);
/*#undef send
#define HTML5_send(a,b,c,d) wrapsend(a,b,c,d)
#define BW_send_0() send()
#define BW_send_1(a) send(a)
#define BW_send_2(a,b) send(a,b)
#define BW_send_3(a,b,c) send(a,b,c)
#define send_GET_MACRO(a, b, c, d, macro, ...) macro
#define send_MACRO_CHOOSER(...) send_GET_MACRO(__VA_ARGS__, HTML5_send,BW_send_3,BW_send_2,BW_send_1,BW_send_0)
#define send(...) send_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define send(a,b,c,d) wrapsend(a,b,c,d)

extC ssize_t	wrapsendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
/*#undef sendto
#define HTML5_sendto(a,b,c,d,e,f) wrapsendto(a,b,c,d,e,f)
#define BW_sendto_2(a,b) sendto(a,b)
#define BW_sendto_3(a,b,c) sendto(a,b,c)
#define BW_sendto_4(a,b,c,d) sendto(a,b,c,d)
#define sendto_GET_MACRO(a, b, c, d, e, f, macro, ...) macro
#define sendto_MACRO_CHOOSER(...) sendto_GET_MACRO(__VA_ARGS__, HTML5_sendto,,BW_sendto_4,BW_sendto_3,BW_sendto_2,,)
#define sendto(...) sendto_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define sendto(a,b,c,d,e,f) wrapsendto(a,b,c,d,e,f)

extC ssize_t	wrapsendmsg(int, const struct msghdr *, int);
/*#undef sendmsg
#define sendmsg(a,b,c) wrapsendmsg(a,b,c)*/


#if __BSD_VISIBLE
//int	sendfile(int, int, off_t, size_t, struct sf_hdtr *, off_t *, int);
//int	setfib(int);
#endif

extC int wrapsetsockopt(int, int, int, const void *, socklen_t);
/*#undef setsockopt
#define setsockopt(a,b,c,d,e) wrapsetsockopt(a,b,c,d,e)*/

extC int	wrapshutdown(int, int);
#undef shutdown
#define shutdown(a,b) wrapshutdown(a,b)

extC int	wrapsockatmark(int);
#undef sockatmark
#define sockatmark(a) wrapsockatmark(a)

extC int wrapsocket(int, int, int);
/*#undef socket
#define HTML5_socket(a,b,c) wrapsocket(a,b,c)
#define BW_socket(a) socket(a)
#define socket_GET_MACRO(a, b, c, macro, ...) macro
#define socket_MACRO_CHOOSER(...) socket_GET_MACRO(__VA_ARGS__, HTML5_socket,,BW_socket, )
#define socket(...) socket_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
//#define socket(a,b,c) wrapsocket(a,b,c)

extC int	wrapsocketpair(int, int, int, int *);
#undef socketpair
#define socketpair(a,b,c,d) wrapsocketpair(a,b,c,d)



/* select.h */
extC int wrapselect(int, fd_set *, fd_set *, fd_set *, struct timeval *);
/*#undef select
#define select(a,b,c,d,e) wrapselect(a,b,c,d,e)*/

/* netdb.h */
//#undef gethostbyname
extC struct hostent *wrapgethostbyname(const char *);
//#define gethostbyname(a) wrapgethostbyname(a)

/*extC void testmacro1(int);
extC void testmacro3(int,int,int);

#define HTML5_testmacro(a,b,c) testmacro3(a,b,c)
#define BW_testmacro(a) testmacro1(a)
#define GET_MACRO(a, b, c, macro, ...) macro
#define testmacro_MACRO_CHOOSER(...) GET_MACRO(__VA_ARGS__, HTML5_testmacro,,BW_testmacro, )
#define testmacro(...) testmacro_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
#endif
