  1//qq聊天系统-----客户端
  2 
  3 #include"tool.h"
  4 
  5 //全局变量
  6 struct sockaddr_in ser_addr;
  7 struct sockaddr_in recv_addr;
  8 int g_num;  //用来记录你的QQ号,因为查询好友及后面的操作需要遍历QQ号，所以把当前登录的QQ号保存一下
  9 
 10 //写一个登录界面
 11 void show_login();
 12 
 13 //用户的选择
 14 int choose_menu(int min,int max);
 15 
 16 //用户注册函数
 17 int registe(int sockfd);
 18 
 19 //用户登录函数
 20 int login(int sockfd);
 21 
 22 //显示功能菜单的函数
 23 void show_menu();
 24 
 25 //查询在线好友
 26 void find_user(int sockfd);
 27 
 28 //添加好友
 29 int add_user(int sockfd);
 30 
 31 //显示好友
 32 int show_user(int sockfd);
 33 
 34 //删除好友
 35 int del_user(int sockfd);
 36 
 37 //退出聊天
 38 void exit_chat(int sockfd);
 39 
 40 //给好友发送信息
 41 int my_send(int sockfd);
 42 
 43 //接收发来的消息
 44 int my_recv(int sockfd);
 45 
 46 int main()
 47 {
 48     int uid;
 49 
 50     int sockfd;
 51     sockfd = create_socket();
 52 
 53     int choose;
 54     //把地址清为0
 55     bzero(&ser_addr,sizeof(struct sockaddr_in));
 56     ser_addr.sin_family = AF_INET;
 57     ser_addr.sin_port = htons(PORT);
 58     ser_addr.sin_addr.s_addr = inet_addr(SER_IP);
 59 
 60     LOGIN:
 61     show_login();
 62     choose = choose_menu(1,2);
 63     printf("你选择的是:%d\n",choose);
 64 
 65     switch(choose)
 66     {
 67         case 1:
 68                 if(login(sockfd))
 69                         printf("登陆成功!\n");
 70                 else
 71                 {
 72                     printf("登录失败!\n");
 73                     exit(-1);
 74                 }
 75                 break;
 76         case 2:
 77                 uid = registe(sockfd);
 78                 if(uid)
79                 {
 80                         printf("注册成功! QQ号为:%d\n",uid);
 81                         setbuf(stdin,NULL);
 82                         getchar();
 83                         goto LOGIN;
 84                 }
 85                 else
 86                 {
 87                         printf("注册失败!\n");
 88                         exit(-1);
 89                 }
 90                 break;
 91     }
 92     //登录成功之后,就是显示好友列表这种东西
 93     int running = 1;
 94     while(running)
 95     {
 96         show_menu();
 97         choose = choose_menu(0,6);
 98 
 99         switch(choose)
100         {
101             case 1:
102                     printf("显示好友\n");
103                     show_user(sockfd);
104                     break;
105             case 2:
106                     printf("查询在线用户\n");
107                     find_user(sockfd);
108                     break;
109             case 3:
110                     printf("添加好友\n");
111                     add_user(sockfd);
112                     break;
113             case 4:
114                     printf("删除好友\n");
115                     del_user(sockfd);
116                     break;
117             case 5:
118                     printf("向好友发送信息\n");
119                     my_send(sockfd);
120                     break;
121             case 6:
122                     printf("接收好友发来的信息!\n");
123                     my_recv(sockfd);
124                     break;
125             case 0:
126                     printf("退出\n");
127                     exit_chat(sockfd);
128                     running = 0;
129                     break;
130         }
131         setbuf(stdin,NULL);
132         getchar();
133         return 0;
134     }
135     return 0;
136 }
137 
138 void show_login()
139 {
140     system("clear");
141     printf("欢迎使用模拟QQ聊天系统!\n");
142     printf("1.用户登录\n");
143     printf("2.用户注册\n");
144 }
145 
146 int choose_menu(int min,int max)
147 {
148     int choose;
149     printf("请选择[%d~%d]:",min,max);
150     while(scanf("%d",&choose) != 1 || choose <min || choose > max)
151     {
152         printf("输入错误,请重新选择[%d~%d]:",min,max);
153 
154         setbuf(stdin,NULL);  //linux下的清空缓冲区,fflush在linux不能用，那是windows
155     }
156     return choose;
157 }
158 
159 
160 int  registe(int sockfd)
161 {
162     struct sockaddr_in addr;
163     struct MSG msg;
164     msg.m_type = MT_REG;
165     printf("*************用户注册*************\n");
166     printf("请输入你的昵称[不得超过20个字节]:");
167 
168     //这里数在输入前清空一下缓冲区，一个习惯，防止代码过多的时候，缓冲有东西但是你却不知道
169     setbuf(stdin,NULL);
170     scanf("%s",msg.m_send.u_name);
171     printf("\n请输入你的密码[不得超过%d字节]:",BUFSIZE);
172     setbuf(stdin,NULL);
173     scanf("%s",msg.m_msg);
174 
175     //向服务端发送注册请求
176     send_msg(sockfd,&msg,&ser_addr);
177     //接受服务端的返回信息
178     recv_msg(sockfd,&msg,&addr);
179 
180     if(strcmp(msg.m_msg,"YES") ==0 )
181     {
182         //注册成功
183             return msg.m_send.u_num;  //返回QQ号
184     }
185     return 0;
186 
187 }
188 
189 int login(int sockfd)
190 {
191     struct sockaddr_in addr;
192     struct MSG msg;
193     msg.m_type = MT_LOG;
194     system("clear");
195     printf("************用户登录************\n");
196     printf("请输入QQ号:");
197     setbuf(stdin,NULL);
198     scanf("%d",&msg.m_send.u_num);
199     g_num = msg.m_send.u_num;   //再一次保存,因为这个后面会用到,当然啦,你不用这个,就用msg.m_send中的num也可以
200 
201     printf("请输入密码:");
202     setbuf(stdin,NULL);
203     scanf("%s",msg.m_msg);
204 
205     //向服务端发送登录请求
206     send_msg(sockfd,&msg,&ser_addr);
207     //接收服务端的返回信息
208     recv_msg(sockfd,&msg,&addr);
209     if(strcmp(msg.m_msg,"YES") == 0)
210     {
211         return 1;
212     }
213     else
214     {
215         return 0;
216     }
217 
218 }
219 
220 void show_menu()
221 {
222     system("clear");
223     printf("**********菜单*****************\n");
224     printf("1.显示好友\n");
225     printf("2.查询在线好友\n");
226     printf("3.添加好友\n");
227     printf("4.删除好友\n");
228     printf("5.向好友发送信息\n");
229     printf("6.接收好友信息\n");
230     printf("0.退出\n");
231     printf("*******************************\n");
232 }
233 
234 void find_user(int sockfd)
235 {
236     struct MSG msg;
237     struct sockaddr_in addr;
238     //通过给服务端发送QQ号,去查找在线好友,不是用昵称
239     msg.m_type = MT_FND;
240     msg.m_send.u_num = g_num;  //qq号,这就是我们设的全局变量的作用
241 
242     //现在准备向服务端发送, 注意是查找服务器在线的QQ号,不是所有注册的QQ号
243     send_msg(sockfd,&msg,&ser_addr);
244 
245     printf("**************在线好友列表***************\n");
246     //接收服务端的返回信息
247     while(1)
248     {
249         recv_msg(sockfd,&msg,&addr);
250         if(strcmp(msg.m_msg,"end") == 0)
251         {
252             //服务端发送完毕
253             break;
254         }
255         else
256         {
257             //服务端发来的在线好友
258             printf("QQ:%d,昵称:%s\n",msg.m_recv.u_num,msg.m_recv.u_name);
259 
260         }
261     }
262     printf("****************************************\n");
263 }
264 
265 int add_user(int sockfd)
266 {
267     //添加在线好友
268     struct MSG msg;
269     struct sockaddr_in recv_addr; //发送端的地址结构
270     msg.m_type = MT_ADD;
271     int uid;
272     printf("请输入好友QQ号:");
273     setbuf(stdin,NULL);
274     scanf("%d",&uid);
275     //添加好友的QQ号和自己的QQ号不能相同
276     if(uid == g_num)
277     {
278         printf("不能和自己的QQ号相同!\n");
279         return -1;
280     }
281     msg.m_send.u_num = g_num;  //把自己的QQ号保存到m_send成员中
282     msg.m_recv.u_num = uid;      //把欲添加的QQ号 保存到m_recv中
283     
284     //把添加好友请求的消息发送给服务器
285     send_msg(sockfd,&msg,&ser_addr);
286     //接收服务端的返回信息
287     recv_msg(sockfd,&msg,&recv_addr);
288     if(strcmp(msg.m_msg,"YES") == 0)
289     {
290         printf("添加好友成功!\n");
291         return 1;
292     }
293     else if(strcmp(msg.m_msg,"re"))
294     {
295         printf("该用户已经是你的好友!\n");
296         return 0;
297     }
298     else
299     {
300         printf("添加好友失败!\n");
301         return 0;
302     }
303 }   
304 
305 int show_user(int sockfd)
306 {
307     struct MSG msg;
308     msg.m_type = MT_SHW;
309     msg.m_send.u_num = g_num;   //把自己QQ号放在Msg结构体变量中,要发给服务端的
310 
311     //向服务端发送好友的消息
312     send_msg(sockfd,&msg,&ser_addr);
313     //接收服务端的消息
314     printf("***************我的好友*********************\n");
315     while(1)
316     {
317         recv_msg(sockfd,&msg,&recv_addr);
318         if(strcmp(msg.m_msg,"end") == 0)
319         {
320             break;
321         }
322         else if(strcmp(msg.m_msg,"ok") == 0)
323         {
324             //该好友在线
325             printf("QQ:%d,昵称:%s,[在线]\n",msg.m_recv.u_num,msg.m_recv.u_name);
326         }
327         else
328         {
329             //该好友不在线
330             printf("QQ:%d,昵称:%s,[不在线]\n",msg.m_recv.u_num,msg.m_recv.u_name);
331         }
332 
333     }
334     printf("******************************************\n");
335 }
336 
337 int del_user(int sockfd)
338 {
339     struct MSG msg;
340     msg.m_type = MT_DEL;
341     int num;
342     printf("**************删除好友**********************\n");
343     printf("请输入要删除的好友QQ:");
344     setbuf(stdin,NULL);
345     scanf("%d",&num);
346 
 347     //删除的QQ不能是自己
348     if(num == g_num)
349     {
350         printf("不能删除自己!\n");
351         return 0;
352     }
353     msg.m_send.u_num = g_num;
354     msg.m_send.u_num = num;
355 
356     send_msg(sockfd,&msg,&ser_addr);
357     recv_msg(sockfd,&msg,&recv_addr);
358 
359     if(strcmp(msg.m_msg,"YES") == 0)
360     {
361         printf("删除成功!\n");
362         return 1;
363     }
364     else
365     {
366         printf("删除失败!\n");
367         return 0;
368     }
369 }
370 
371 void exit_chat(int sockfd)
372 {
373     struct MSG msg;
374     msg.m_type = MT_EXT;
375     msg.m_send.u_num = g_num;
376 
377     //给服务器发送消息
378     send_msg(sockfd,&msg,&ser_addr);
379     recv_msg(sockfd,&msg,&recv_addr);
380     if(strcmp(msg.m_msg,"YES") == 0)
381     {
382         //退出成功
383         printf("退出成功!\n");
384     }
385     else
386     {
387         printf("退出失败!\n");
388     }
389 }
390 
391 
392 int my_send(int sockfd)
393 {
394     struct MSG msg;
395     msg.m_type = MT_MSG_SEND;  //发送信息
396     msg.m_send.u_num = g_num;  //发送者的QQ号
397 
398     printf("输入对方的QQ号:");
399     int num;
400     setbuf(stdin,NULL);
401     scanf("%d",&num);
402     msg.m_recv.u_num = num;     //接收者的QQ号
403     printf("输入消息的内容:");
404     setbuf(stdin,NULL);
405     fgets(msg.m_msg,BUFSIZE,stdin);
406     //把消息发送给服务端
407     send_msg(sockfd,&msg,&ser_addr);
408     //接收信息
409     recv_msg(sockfd,&msg,&recv_addr);
410     if(strcmp(msg.m_msg,"YES") == 0)
411     {
412         printf("发送成功!\n");
413         return 1;
414     }
415     else
416     {
417         printf("消息发送失败!\n");
418         return 0;
419     }
420 
421 }
422 
423 int my_recv(int sockfd)
424 {
425     struct MSG msg;
426     msg.m_type = MT_MSG_RECV;   //接收消息
427     msg.m_send.u_num = g_num;   //把接受者QQ放入结构体
428 
429     //把该消息发给服务端
430     send_msg(sockfd,&msg,&ser_addr);
431     //接收服务端的回传消息
432     while(1)
433     {
434         recv_msg(sockfd,&msg,&recv_addr);
435         if(strcmp(msg.m_msg,"") == 0)       //如果是空消息就终止
436             break;
437         printf("%s",msg.m_msg);
438     }
439 }


                                                             

