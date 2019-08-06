 1 //这个头文件写我们服务端和客户端要用到的函数
  2 
  3 #ifndef QQCHAT_TOOLS_H
  4 #define QQCHAT_TOOLS_H
  5 
  6 #include<sys/types.h>
  7 #include<sys/socket.h>
  8 #include<netinet/in.h>
  9 #include<arpa/inet.h>
 10 #include<unistd.h>
 11 #include<string.h>
 12 #include<stdlib.h>
 13 #include<stdio.h>
 14 
 15 
 16 #define MT_REG 1  //注冊帳號(message type)
 17 #define MT_LOG 2  //用戶登錄
 18 #define MT_EXT 3  //用戶退出
 19 #define MT_ADD 4  //添加好友
 20 #define MT_FND 5  //查找好友
 21 #define MT_DEL 6  //刪除好友
 22 #define MT_MSG_SEND 7  //傳送消息
 23 #define MT_MSG_RECV 8  //接收消息
 24 #define MT_PAS 9  //修改密碼
 25 #define MT_NME 10  //修改昵称
 26 #define MT_RET 12 //給客戶端的回饋
 27 #define MT_SHW 11 //显示好友
 28 
 29 
 30 //2.用户信息存放的目录
 31 #define USERS_DIR "./USERS/"
 32 
 33 //3.用戶帳號密碼信息文件
 34 #define USERS_INFO_FILE "./passwd.txt"
 35 
 36 //4.服務端綁定的端口號
 37 #define PORT 5050
 38 
 39 //5.服務端IP地址
 40 #define SER_IP "127.0.0.1"
 41 
 42 //6.緩衝區大小
 43 #define BUFSIZE 256
 44 
 45 //8.昵稱大小
 46 #define NAMESIZE 21
 47 
 48 //7.用戶帳號結構
 49 struct USER
 50 {
 51     int u_num; //QQ_number
 52     char u_name[NAMESIZE + 1];  //昵稱
 53 };
 54 
 55 
 56 //9.消息結構
 57 struct MSG
 58 {
 59     int m_type;  //消息的類形
 60     struct USER m_send;  //發送方
 61     struct USER m_recv;  //接收方
 62     char m_msg[BUFSIZE];  //
 63 };
 64 
 65 //10.在线用户信息,这是服务器要保存的
 66 struct OL_INFO
 67 {
 68     struct USER user;           //用户QQ号和昵称
 69     struct sockaddr_in addr;    //用户的地址结构
 70 };
 71 
 72 //11.在线用户数组,就是在那一时刻,多少客户端链接到了服务端
 73 struct OL_INFO g_ol_users[10];
 74 int OL_NUM;  //在线用户的数量

 78 //函数
 79 int create_socket();  //创建套接字
 80 int bind_port(int sockfd);  //绑定端口
 81 int send_msg(int sockfd,struct MSG *msg,struct sockaddr_in *addr);  //发送消息
 82 int recv_msg(int sockfd,struct MSG *msg,struct sockaddr_in *addr);  //接受信息
 83 
 84 
 85 
 86 
 87 
 88 #endif

