/*客户端程序UDPClient.c，使用方法UDPClient ServerIP ServerPort*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#define MAX_BUF_SIZE    1024

void udpc_requ(int sockfd,const struct sockaddr_in *addr,int len)
{
        char buffer[MAX_BUF_SIZE];
        int n;
        while(1)        
        {        /*从键盘读入，写到服务端*/
                fgets(buffer,MAX_BUF_SIZE,stdin);
                sendto(sockfd,buffer,strlen(buffer),0,addr,len);
                bzero(buffer,MAX_BUF_SIZE);
                /*从网络上读，写到屏幕上*/
                n=recvfrom(sockfd,buffer,MAX_BUF_SIZE,0,NULL,NULL);
                buffer[n]=0;
                fputs(buffer,stdout);
        }
}

int main(int argc,char **argv)
{
        int sockfd,port;
        struct sockaddr_in      addr;
        
        if(argc!=3)
        {
                fprintf(stderr,"Usage:%s server_ip server_port\n",argv[0]);
                exit(1);
        }
        
        if((port=atoi(argv[2]))<0)
        {
                fprintf(stderr,"Usage:%s server_ip server_port\n",argv[0]);
                exit(1);
        }
        
        sockfd=socket(AF_INET,SOCK_DGRAM,0);
        if(sockfd<0)
        {
                fprintf(stderr,"Socket  Error:%s\n",strerror(errno));
                exit(1);
        }       
        /*填充服务器端的资料*/
        bzero(&addr,sizeof(struct sockaddr_in));
        addr.sin_family=AF_INET;
        addr.sin_port=htons(port);
        if(inet_aton(argv[1],&addr.sin_addr)<0)
        {
                fprintf(stderr,"Ip error:%s\n",strerror(errno));
                exit(1);
        }
        udpc_requ(sockfd,&addr,sizeof(struct sockaddr_in));
        close(sockfd);
}
