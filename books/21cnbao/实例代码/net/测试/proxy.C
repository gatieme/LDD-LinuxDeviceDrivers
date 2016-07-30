/*代理服务器源代码proxy.c*/
#include <stdio.h> 
#include <ctype.h> 
#include <errno.h> 
#include <signal.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/file.h> 
#include <sys/ioctl.h> 
#include <sys/wait.h> 
#include <sys/types.h> 
#include <netdb.h> 
#define TCP_PROTO　 "tcp" 
int proxy_port;　　　 				/*全局变量，指定代理服务器的端口*/ 
struct sockaddr_in hostaddr;　　 		/*全局变量，远端主机地址*/ 
extern int errno;　　 
extern char *sys_myerrlist[]; 
void parse_args (int argc, char **argv);	 /*参数转换函数*/
void daemonize (int servfd); 			/*创建守护进程函数*/
void do_proxy (int usersockfd); 		/*代理处理函数*/
void errorout (char msg);			/*错误输出函数*/
/*********************************************************
主函数
**********************************************************/
main (argc,argv) 
int argc; 
char **argv; 
{ 
　　　int clilen; 
　　　int childpid; 
　　　int sockfd, newsockfd; 
　　　struct sockaddr_in servaddr, cliaddr; 
/*把命令行参数转存到全局变量中*/
　　　parse_args(argc,argv); 
　　　/*为侦听客户请求准备一个地址*/ 
　　　bzero((char *) &servaddr, sizeof(servaddr)); 
　　　servaddr.sin_family = AF_INET; 
　　　servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
　　　servaddr.sin_port = proxy_port; 
　　　/*得到一个端口的文件描述符*/ 
　　　if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) { 
　　　　　fputs("failed to create server socket\r\n",stderr); 
　　　　　exit(1); 
　　　} 
　　　/*绑定到前面的地址上*/ 
　　　if　　 (bind(sockfd,(struct sockaddr_in *) &servaddr,sizeof(servaddr)) < 0) { 
　　　　　fputs("faild to bind server socket to specified port\r\n",stderr); 
　　　　　exit(1); 
　　　　} 
　　　/*准备接收*/ 
　　　listen(sockfd,5); 
　　/*把自身变为守护进程*/ 
　　daemonize(sockfd); 
　　/*进入一个循环，并发处理连接请求*/ 
　　while (1) {
　　　　/*接受连接请求*/ 
　　　　clilen = sizeof(cliaddr); 
　　　　newsockfd = accept(sockfd, (struct sockaddr_in *) &cliaddr, &clilen); 
　　　　if (newsockfd < 0 && errno == EINTR) 
　　　　　　continue; 
　　　　　else if (newsockfd < 0) 
　　　　　　/*出错，kill服务器*/ 
　　　　errorout("failed to accept connection");
　　　　/*产生一个子进程，进行连接处理*/ 
　　　　if ((childpid = fork()) == 0) { 
　　　　　　close(sockfd); 
　　　　　　do_proxy(newsockfd); 		/*真正的处理过程*/
　　　　　　exit(0); 
　　　　　} 
　　　　/*如果产生子进程失败，连接将被丢掉*/ 
　　　　　close(newsockfd); 
　　　　　} 
　　　}

/**************************************************************** 
进行参数转换，把从命令行得到的参数值赋给全局变量
****************************************************************/ 
void parse_args (argc,argv) 
int argc; 
char **argv; 
{ 
　　int i; 
　　struct hostent *hostp; 
　　struct servent *servp; 
　　unsigned long inaddr; 
　　struct { 
　　　　char proxy_port [16]; 
　　　　char isolated_host [64]; 
　　　　char service_name [32]; 
　　} pargs; 
/*输入不合规范*/
　　if (argc < 4) { 
　　　　　printf("usage: %s <proxy-port> <host> <service-name|port-number>\r\n", argv[0]); 
　　　　　exit(1); 
　　} 
/*（将什么）将输入参数先放到自定义的数据结构中*/
　　strcpy(pargs.proxy_port,argv[1]); 
　　strcpy(pargs.isolated_host,argv[2]); 
　　strcpy(pargs.service_name,argv[3]); 
/*检查端口号是否是数字，再赋给proxy_port*/
　　for (i = 0; i < strlen(pargs.proxy_port); i++) 
　　　　if (!isdigit(*(pargs.proxy_port + i))) 
　　　　　　break; 
　　if (i == strlen(pargs.proxy_port)) 
　　　　proxy_port = htons(atoi(pargs.proxy_port)); 
　　else { 
　　　　printf("%s: invalid proxy port\r\n",pargs.proxy_port); 
　　　　exit(0); 
　　} 
/*把远端服务器地址赋给hostaddr*/
　　bzero(&hostaddr,sizeof(hostaddr)); 
　　hostaddr.sin_family = AF_INET; 
/*不管是主机名还是IP地址，都把它转换为hostaddr 的地址*/
　　if ((inaddr = inet_addr(pargs.isolated_host)) != INADDR_NONE) 
　　　　bcopy(&inaddr,&hostaddr.sin_addr,sizeof(inaddr)); 
　　else if ((hostp = gethostbyname(pargs.isolated_host)) != NULL) 
　　　　bcopy(hostp->h_addr,&hostaddr.sin_addr,hostp->h_length); 
　　else { 
　　　　printf("%s: unknown host\r\n",pargs.isolated_host); 
　　　　exit(1); 
　　} 
/*不管是用数字表示端口还是用服务表示的端口，都把它转换后赋给hostaddr.sin_port*/
　　if ((servp = getservbyname(pargs.service_name,TCP_PROTO)) != NULL) 
　　　　hostaddr.sin_port = servp->s_port; 
　　else if (atoi(pargs.service_name) > 0) 
　　　　hostaddr.sin_port = htons(atoi(pargs.service_name)); 
　　else { 
　　　　printf("%s: invalid/unknown service name or port number\r\n", pargs.service_name); 
　　　　exit(1); 
　　} 
}

/**************************************************************** 
创建守护进程函数
****************************************************************/ 
void daemonize (servfd) 
int servfd; 
{ 
　　int childpid, fd, fdtablesize; 
　　/*忽略终端I/O读、写和stop 信号*/ 
　　　signal(SIGTTOU,SIG_IGN); 
　　　signal(SIGTTIN,SIG_IGN); 
　　　signal(SIGTSTP,SIG_IGN); 
　　/*通过fork子进程，kill父进程，把自身转入后台*/ 
　　if ((childpid = fork()) < 0) { 
　　　　fputs("failed to fork first child\r\n",stderr); 
　　　　exit(1); 
　　　} 
　　else if (childpid > 0) 
　　　exit(0); /*若是父进程，结束*/ 
　　　/*设为会话组长，摆脱原终端*/ 
　　setsid(0) ;
　　　　 
　　/*释放控制终端*/ 
　　if ((fd = open("/dev/tty",O_RDWR)) >= 0) { 
　　　　ioctl(fd,TIOCNOTTY,NULL); 
　　　　close(fd); 
　　} 
　　/*关闭除 servfd 外的所有文件描述符*/ 
　　for (fd = 0, fdtablesize = getdtablesize(); fd < fdtablesize; fd++) 
　　if (fd != servfd) 
　　　close(fd);
　　　/*改变工作目录到根目录*/ 
　　　chdir("/"); 
　　　/*重设文件掩码*/ 
　　　umask(0); 
} 


/*************************************************** 
代理处理函数
****************************************************************/ 
void do_proxy (usersockfd) 
int usersockfd; 
{ 
　　int isosockfd; 
　　fd_set rdfdset;
　　int connstat;
　　int iolen; 
　　char buf[2048]; 
　　/*作为一个客户端，新开一个端口以连接远端服务器*/ 
　　if ((isosockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) 
　　errorout("failed to create socket to host");
　　/*发连接请求*/ 
　　connstat = connect(isosockfd,(struct sockaddr *) &hostaddr, sizeof(
hostaddr)); 
	/*出错处理*/
　　switch (connstat) {
　　case 0: 
　　break; 
　　case ETIMEDOUT: 
　　case ECONNREFUSED: 
　　case ENETUNREACH: 
　　strcpy(buf,sys_myerrlist[errno]); 
　　strcat(buf,"\r\n"); 
　　write(usersockfd,buf,strlen(buf)); 
　　close(usersockfd); 
　　exit(1); 
　　break;
　　default: 
　　errorout("failed to connect to host"); 
　　} 
　　/*现在已经建立了连接，进入代理的数据反馈循环*/ 
　　while (1) { 
　　　　/* 使用select进行并发处理 */ 
　　　　FD_ZERO(&rdfdset);
　　　　FD_SET(usersockfd,&rdfdset);
　　　　FD_SET(isosockfd,&rdfdset);
　　if (select(FD_SETSIZE,&rdfdset,NULL,NULL,NULL) < 0) 
　　　errorout("select failed");
　　　/*客户端有数据发过来吗*/
　　if (FD_ISSET(usersockfd,&rdfdset)) { 
		/*小于或等于0意味着客户端已断*/
　　　　if ((iolen = recv(usersockfd,buf,sizeof(buf)),0) <= 0) 
　　　　　break; 
　　　　　send(isosockfd,buf,iolen,0); 
　　　　　/*把数据拷贝一份发给服务器端*/ 
　　　} 
　　/*远端服务器有数据发过来吗*/ 
　　if (FD_ISSET(isosockfd,&rdfdset)) { 
　　　　f ((iolen = recv(isosockfd,buf,sizeof(buf))) <= 0) 
　　　　　　break; /*接收数据长度小于或等于0标明连接已断*/ 
　　　　　　send(usersockfd,buf,iolen); 
　　　　　　/*把数据拷贝一份发给客户端*/ 
　　　　} 
　　　}
　 　 close(isosockfd); 
　　　close(usersockfd);
　　}

/**************************************************************** 
出错处理函数
****************************************************************/ 
void errorout (msg) 
char *msg; 
{ 
　　FILE *console;
　　console = fopen("/dev/console","a"); 
　　fprintf(console,"proxyd: %s\r\n",msg); 
　　fclose(console); 
　　exit(1); 
}
