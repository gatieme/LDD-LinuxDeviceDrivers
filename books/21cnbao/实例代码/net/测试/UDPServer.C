/*服务端程序UDPServer.c*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#define SERVER_PORT     8888
#define MAX_MSG_SIZE    1024

void udps_respon(int sockfd)
{
        struct sockaddr_in addr;
        int     addrlen,n;
        char    msg[MAX_MSG_SIZE];
        
        while(1)
        {       /*等待数据请求*/
                n=recvfrom(sockfd,msg,MAX_MSG_SIZE,0,
                        (struct sockaddr*)&addr,&addrlen);
                msg[n]=0;
                /*显示服务器端已经收到了信息*/
                fprintf(stdout,"I have received %s",msg);
				/*数据回送*/
                sendto(sockfd,msg,n,0,(struct sockaddr*)&addr,addrlen);
        }
}

int main(void)
{
        int sockfd;
        struct sockaddr_in      addr;
        
        sockfd=socket(AF_INET,SOCK_DGRAM,0);
        if(sockfd<0)
        {
                fprintf(stderr,"Socket Error:%s\n",strerror(errno));
                exit(1);
        }
        bzero(&addr,sizeof(struct sockaddr_in));
        addr.sin_family=AF_INET;
        addr.sin_addr.s_addr=htonl(INADDR_ANY);
        addr.sin_port=htons(SERVER_PORT);
        if(bind(sockfd,(struct sockaddr *)&ddr,sizeof(struct sockaddr_in))<0)
        {
                fprintf(stderr,"Bind Error:%s\n",strerror(errno));
                exit(1);
        }
        udps_respon(sockfd);
        close(sockfd);
}
