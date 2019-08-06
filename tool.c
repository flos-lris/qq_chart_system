  1 #include"tool.h"
  2 
  3 int create_socket()
  4 {   
  5     int sockfd;
  6     sockfd = socket(PF_INET,SOCK_DGRAM,0);
  7     if(sockfd == -1)
  8     {
  9         printf("创建套接字失败!\n");
 10         exit(-1);
 11     }
 12     return sockfd;
 13 }
 14 
 15 int bind_port(int sockfd)
 16 {   
 17     struct sockaddr_in addr;
 18     bzero(&addr,sizeof(addr));
 19     addr.sin_family = AF_INET;
 20     addr.sin_port = htons(PORT);
 21     addr.sin_addr.s_addr = htonl(INADDR_ANY);
 22     
 23     if(bind(sockfd,(struct sockaddr *)&addr,sizeof(struct sockaddr)) == -1)
 24     {
 25         printf("绑定端口失败!\n");
 26         exit(-1);
 27     }
 28     
 29     return 1;
 30 }
 31 
 32 
 33 int send_msg(int sockfd,struct MSG *msg,struct sockaddr_in *addr)
 34 {   
 35     ssize_t size;
 36     size = sendto(sockfd,msg,sizeof(struct MSG),0,(struct sockaddr*)addr,sizeof(struct sockaddr));
 37     if(size == -1)
 38     {
 39         printf("发送消息失败!\n");
 40         return -1;
 41     }
 42     else if(size != sizeof(struct MSG))
 43     {
 44         printf("发送的消息不完整!\n");
 45     }
 46     else
 47     {
 48 //      printf("发送消息成功!\n");
 49         return 1;
 50     }
 51 }
 52 
 53 
 54 int recv_msg(int sockfd,struct MSG *msg,struct sockaddr_in *addr)
 55 {
 56     ssize_t size;
 57     //接下来这一步很关键,就是必须给len初始化为地址结构通用类型,len是用来接受传过来的消息的大小,这在帮助文档下是写的
 58     //说需要在被调用之前初始化
 59     //但是啊,我发下我没初始化,好像也是对的,很奇怪
 60     socklen_t len = sizeof(struct sockaddr);
 61 
 62     size = recvfrom(sockfd,msg,sizeof(struct MSG),0,(struct sockaddr*)addr,&len);
 63     if(size == -1)
 64     {
 65         printf("接收消息失败!\n");
 66         return -1;
 67     }
 68     else if(size != sizeof(struct MSG))
 69     {
 70         printf("发送的消息不完整!\n");
 71     }
 72     else
 73     {
 74 //      printf("发送消息成功!\n");
 75         return 1;
 76     }
 77 }

