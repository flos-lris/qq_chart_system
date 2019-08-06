 1 //qq聊天系统----服务端
  2 
  3 #include"tool.h"
  4 #include<dirent.h>
  5 #include<sys/stat.h>
  6 
  7 int g_uid = 1000;   //这是QQ号,我们规定从1000开始
  8 
  9 void init();  //服务端的初始化
 10 
 11 int registe(struct MSG *msg); //用户的注册,服务端也要有,要处理客户端的注册信息
 12 
 13 int login(struct MSG *msg);  //用户的登录
 14 
 15 int getname(int num,char *name);  //通过QQ号获取昵称
 16 
 17 int add_user(struct MSG *msg);    //处理添加好友
 18 
 19 int is_online(int num);          //判断QQ是否在线
 20 
 21 int del_user(struct MSG *msg);   //删除好友
 22 
 23 int exit_user(struct MSG *msg);
 24 
 25 int my_send(struct MSG *msg);   //客户端发来的信息
 26 
 27 int my_recv(struct MSG *msg,int sockfd,struct sockaddr_in *addr);   //返回的消息
 28 int main()
 29 {
 30     int sockfd;
 31     init();
 32     sockfd = create_socket();
 33     bind_port(sockfd);
 34     struct MSG msg;
 35     struct sockaddr_in cli_addr;
 36     while(1)
 37     {
 38         printf("服务端正在运行,等待客户端发送请求!\n");
 39         recv_msg(sockfd,&msg,&cli_addr);
40         switch(msg.m_type)
 41         {
 42                 case MT_REG:   //用户注册
 43                         printf("用户注册!\n");
 44                         if(registe(&msg))
 45                         {
 46                             //注册成功
 47                             strcpy(msg.m_msg,"YES");
 48                         }
 49                         else
 50                         {
 51                             //注册失败
 52                             strcpy(msg.m_msg,"NO");
 53                         }
 54                         send_msg(sockfd,&msg,&cli_addr);
 55                         break;
 56                 case MT_LOG:
 57                         if(login(&msg))
 58                         {
 59                             strcpy(msg.m_msg,"YES");
 60                             //把该用户添加到在线数组,因为这边在记录同一时刻在线人数
 61                             g_ol_users[OL_NUM].user.u_num = msg.m_send.u_num;
 62                             getname(msg.m_send.u_num,g_ol_users[OL_NUM].user.u_name);
 63                             g_ol_users[OL_NUM].addr = cli_addr;
 64                             ++OL_NUM; //自增在线用户的数量,因为在case MT_LOG里面
 65                         }
 66                         else
 67                             strcpy(msg.m_msg,"NO");
 68                         send_msg(sockfd,&msg,&cli_addr);  //向客户端回传结果
 69                         break;
 70                 case MT_FND:
 71                         {
 72                             int i;
 73                             //这里用strcpy表示我们即将开始正常给客户端发送信息
 74                             strcpy(msg.m_msg,"YES");
 75                             for(i=0 ; i<OL_NUM ; ++i)
 76                             {
 77                                 //遍历在线用户数组,看有多少在线用户,不是在线好友
 78                                 msg.m_recv = g_ol_users[i].user;
 79                                 //吧包含在线用户信息的消息发送给客户端
 80                                 send_msg(sockfd,&msg,&cli_addr);
 81                             }
 82                             //表示在线用户发送完毕,把end字符串放入结构体,然后客户端会扫描,扫到就结束
 83                             strcpy(msg.m_msg,"end");
 84                             send_msg(sockfd,&msg,&cli_addr);
 85                         }
 86                         break;
 87                 case MT_ADD:
 88                         add_user(&msg);
 89                         send_msg(sockfd,&msg,&cli_addr);
 90                         break;
 91                 case MT_SHW:
 92                         {
 93                             FILE *fp = NULL;
 94                             char filename[128];
 95                             sprintf(filename,"%s%d",USERS_DIR,msg.m_send.u_num);
 96                             fp = fopen(filename,"r");
 97                             if(fp == NULL)
 98                             {
 99                                 printf("打开%s文件!\n",filename);
100                             }
101                             else
102                             {
103                                 int num;  //QQ号
104                                 char buf[128];
105                                 char name[128];
106                                 while(1)
107                                 {
108                                     fgets(buf,128,fp);
109                                     if(feof(fp))
110                                             break;
111                                     num = atoi(buf);
112                                     msg.m_recv.u_num = num;
113                                     getname(num,msg.m_recv.u_name);  //得到昵称
114                                     if(is_online(num))
115                                             strcpy(msg.m_msg,"YES");
116                                     else
117                                             strcpy(msg.m_msg,"NO");
118                                     send_msg(sockfd,&msg,&cli_addr);
119                                 }
120                                 fclose(fp);
121                                 strcpy(msg.m_msg,"end");    //所有好友发送完毕,最后发送end
122                                 send_msg(sockfd,&msg,&cli_addr);
123                             }
124                         }
125                         break;
126                 case MT_DEL:
127                         {
128                             if(del_user(&msg))
129                             {
130                                 //删除成功
131                                 strcpy(msg.m_msg,"YES");
132                             }
133                             else
134                             {
135                                 strcpy(msg.m_msg,"NO");
136                             }
137                             send_msg(sockfd,&msg,&cli_addr);
138                         }
139                         break;
140                 case MT_EXT:
141                         if(exit_user(&msg))
142                                 strcpy(msg.m_msg,"YES");
143                         else
144                                 strcpy(msg.m_msg,"NO");
145                         send_msg(sockfd,&msg,&cli_addr);
146                         break;
147                 case MT_MSG_SEND:
148                         if(my_send(&msg))
149                                 strcpy(msg.m_msg,"YES");
150                         else
151                                 strcpy(msg.m_msg,"NO");
152                         send_msg(sockfd,&msg,&cli_addr);
153                         break;
154                 case MT_MSG_RECV:
155                         my_recv(&msg,sockfd,&cli_addr);
156                         break;
157                 default:
158                         printf("未知类型的消息!\n");
159                         break;
160         }
161     }
162 
163     return 0;
164 }
165 
166 void init()
167 {
168     //1.创建目录,需要注意，得先检查是否有目录存在，因为我们的服务器是要关闭的
169     DIR *dir = NULL;
170     dir = opendir(USERS_DIR);
171     if(dir == NULL)
172     {
173         printf("准备创建目录...\n");
174         if(mkdir(USERS_DIR,S_IRWXU) == -1)
175                 printf("创建目录%s失败!\n",USERS_DIR);
176         else
177                 printf("创建目录%s成功\n",USERS_DIR);
178     }
179     else
180     {
181         printf("%s目录已经存在!\n",USERS_DIR);
182         closedir(dir);
183     }
184     //2.创建文件,实现细节里面有
185     FILE *pf = NULL;
186     pf = fopen(USERS_INFO_FILE,"r");
187     if(pf == NULL)
188     {
189         pf = fopen(USERS_INFO_FILE,"w");
190         if(pf == NULL)
191         {
192             printf("创建文件失败!\n");
193         }
194         else
195         {
196             printf("创建文件成功!\n");
197         }
198     }
199     else
200     {
201         //3.设置当前的QQ号
202         int num;
203         char buf[128];
204         while(!feof(pf))
205         {
206             fgets(buf,128,pf);
207             num = atoi(buf);  //得到每行QQ号
208             if(g_uid<=num)
209                     g_uid = num+1;
210         }
211     }
212     printf("新注册的QQ号码为: %d\n",g_uid);
213 
214 }
215 
216 //用户注册:给客户端分配QQ号,信息保存在passwd.txt,在USER目录下创建好友信息文件
217 int registe(struct MSG *msg)
218 {
219     msg->m_send.u_num = g_uid;
220 
221     FILE *fp = fopen(USERS_INFO_FILE,"a+");
222     if(fp == NULL)
223     {
224         printf("打开账号密码信息文件失败!\n");
225         return 0;
226     }
227     char buf[BUFSIZE];
228     sprintf(buf,"%d,%s,%s\n",g_uid,msg->m_send.u_name,msg->m_msg);
229     fputs(buf,fp);
230     fclose(fp);
231 
232     //在USERS目录下创建以该QQ号为文件名的文件
233     sprintf(buf,"%s%d",USERS_DIR,g_uid);
234     fp = fopen(buf,"w");
235     if(fp == NULL)
236     {
237         printf("在USERS目录下创建用户好友文件失败!\n");
238         return 0;
239     }
240     printf("创建好友文件成功!\n");
241     fclose(fp);
242     ++g_uid;
243     return 1;
244 }
245 
246 int login(struct MSG *msg)
247 {
248     FILE *fp = fopen(USERS_INFO_FILE,"r");
249     if(fp == NULL)
250     {
251         printf("打开账号密码信息文件失败!\n");
252         return 0;
253     }
254 
255     //说明打开成功,开始查看QQ号与密码是否匹配
256     char buf[128];
257     int num;        //记录QQ号
258     char passwd;    //记录密码
259 
260     while(!feof(fp))
261     {
262         //注意,fgets读取,会默认加一个换行符,所以,在读密码的时候,会出现错误
263         //我们就把下面strncmp函数第三个参数，提取消息的长度，而不是pstr指针，因为pstr指针中还有一个回车
264         fgets(buf,128,fp); //读取一行
265         num = atoi(buf);
266         if(msg->m_send.u_num == num)
267         {
268             //在文件中找到对应的qq号,进行密码验证
269             const char *pstr = NULL;
270             //这个strtok函数可以分隔字符串，因为我们是QQ号，昵称，密码，我们要越过
271             //昵称，把密码取出来，需要用到
272             pstr = strtok(buf,",");   //这是得到QQ号
273             printf("qq号:%s\n",pstr);
274 
275             pstr = strtok(NULL,",");  //这是获得昵称
276             printf("昵称:%s\n",pstr);
277 
278             pstr = strtok(NULL,",");
279             printf("密码:%s\n",pstr);
280 
281             if(strncmp(pstr,msg->m_msg,strlen(msg->m_msg)) == 0)
282             {
283                 //密码匹配  
284                 printf("密码匹配!\n");
285                 fclose(fp);
286                 return 1;
287             }
288             else
289             {
290                 //密码不对
291                 printf("密码不匹配!\n");
292                 fclose(fp);
293                 return 0;
294             }
295         }
296     }
297     printf("没有找到这个账户:%d\n",msg->m_send.u_num);
298     fclose(fp);
299     return 0;
300 }
301 
302 
303 
304 int getname(int num,char *name)
305 {
306     //从passwd.txt中根据QQ号,提取昵称
307     FILE *fp = fopen(USERS_INFO_FILE,"r");
308     if(fp == NULL)
309     {
310         //打开文件失败
311             return 0;
312     }
313     char buf[128];
314     char *pstr = NULL;
315     while(!feof(fp))
316     {
317         fgets(buf,128,fp);
318         if(atoi(buf) == num)
319         {
320             //找到了匹配的QQ号
321             pstr = strtok(buf,",");
322             pstr = strtok(NULL,",");
323             strcpy(name,pstr);
324             fclose(fp);
325             return 1;
326         }
327     }
328     fclose(fp);
329     return 0;
330 }
331 
332 int add_user(struct MSG *msg)
333 {
334     //判断QQ号是不是有效的
335     FILE *fp = fopen(USERS_INFO_FILE,"r");
336     if(fp == NULL)
337     {
338         printf("打开用户密码信息文件失败!\n");
339         return 0;
340     }
341     int valid = 0;  //QQ号是否有效,初始默认为无效
342     char buf[128];
343     while(!feof(fp))
344     {
345         fgets(buf,128,fp);
346         if(atoi(buf) == msg->m_recv.u_num)
347         {
348             valid = 1;
349             break;
350         }
351     }
352     fclose(fp);
353     //如果是无效QQ号
354     if(valid == 0)
355     {
356         strcpy(msg->m_msg,"NO");
357         return 0;
358     }
359     //是否之前已经是好友了
360     char filename[128];
361     sprintf(filename,"%s%d",USERS_DIR,msg->m_send.u_num);
362     fp = fopen(filename,"r");
363     if(fp == NULL)
364     {
365         printf("打开文件%s失败!\n",filename);
366         strcpy(msg->m_msg,"NO");
367         return 0;
368     }
369     int exist = 0;  //该好友是否已经存在,默认不存在
370     //这一步清空buf很重要,因为buf上面用过了,虽然fgets本身没读到文件的内容,但是buf本身的内容没清空，也不知道哪来的内容
371     //导致会显示好友已经存在
372     memset(buf,0,128);
373     while(feof(fp) == 0)
374     {
375         fgets(buf,128,fp);
376         if(atoi(buf) == msg->m_recv.u_num)
377         {
378             exist = 1;
379             break;
380         }
381     }
382     fclose(fp);
382     fclose(fp);
383     //根据exist的值来确定是否添加为好友
384     if(exist)
385     {
386         strcpy(msg->m_msg,"re");
387         return 0;
388     }
389 
390     else
391     {
392         //保存我们将要写入文件的一行的信息
393         char line[128];
394         //保存昵称
395         char name[128];
396         //设置为好友
397         fp = fopen(filename,"a");
398         getname(msg->m_recv.u_num,name);
399         sprintf(line,"%d,%s\n",msg->m_recv.u_num,name);
400         fputs(line,fp);
401         fclose(fp);
402 
403 
404         //修改被加好友的文件
405         sprintf(filename,"%s%d",USERS_DIR,msg->m_recv.u_num);
406         fp = fopen(filename,"a");
407         getname(msg->m_send.u_num,name);
408         sprintf(line,"%d,%s\n",msg->m_send.u_num,name);
409         fputs(line,fp);
410         fclose(fp);
411         strcpy(msg->m_msg,"YES");
412         return 1;
413 
414     }
415 }
416 
417 int is_online(int num)
418 {
419     int i;
420     for(i = 0;i < OL_NUM; ++i)
421     {
422         if(g_ol_users[i].user.u_num == num)
423             return 1;
424     }
425     printf("已经没有好友在线了\n");
426     return 0;
427 }
428 
429 int del_user(struct MSG *msg)
430 {
431     int res = 0;             //是否删除成功
432     FILE *fp = NULL;
433     char *pbuf = NULL;       //缓冲数组  
434     long size = 0;           //文件大小
435     char filename[128];
436     char buf[128];           //保存从文件中读取到的数据(一行)
437 
438     int self_num,del_num;
439     self_num = msg->m_send.u_num;   //自己的QQ
440     del_num = msg->m_recv.u_num;    //要删除的QQ
441 
442     //要删除的QQ是不是自己的好友
443     sprintf(filename,"%s%d",USERS_DIR,self_num);
444     fp = fopen(filename,"r+");
445     if(fp == NULL)
446     {
447         printf("打开文件%s失败!\n",filename);
448         return 0;
449     }
450     //获取文件大小
451     fseek(fp,0,SEEK_END);
452     size = ftell(fp);   //得到文件大小
453     rewind(fp);         //把文件指针移动到文件首,fseek也可以
454 
455     //申请缓冲区
456     pbuf = (char*)malloc(size);
457     //从文件中读取数据
458     while(1)
459     {
460         fgets(buf,128,fp);
461         if(feof(fp))
462             break;
463         //判断该行是不是想要删除的数据
464         if(atoi(buf) == del_num)
465         {
466             res = 1;
467         }
468         else
469         {
470             strcat(pbuf,buf);      //把文件读取的数据放回去,因为不是要删除的
471         }
472     }
473     fclose(fp);
474     //判断是否删除了好友
475     if(res)
476     {
477             fp = fopen(filename,"w");
478             fputs(pbuf,fp);             //把缓冲区的内容写回文件中
479             free(pbuf);                 //把缓冲区释放掉
480             //被删除好友的文件中也要进行相应的操作,其实这里用递归更好
481             sprintf(filename,"%s%d",USERS_DIR,del_num);
482             fp = fopen(filename,"r");
483             if(fp == NULL)
484             {
485                 printf("打开文件%s失败!\n",filename);
486                 return 0;
487             }
488             //这里，为啥要用size呢，是为了给缓冲区一个大小，删除常见操作
489             fseek(fp,0,SEEK_END);
490             size = ftell(fp);
491             pbuf = (char *)malloc(size);
492             rewind(fp);
493             while(1)
494             {
495                 fgets(buf,128,fp);
496                 if(feof(fp))
497                     break;
498                 if(atoi(buf) != self_num)
499                 {
500                     strcat(pbuf,buf);
501                 }
502             }
503             fclose(fp);
504             //把缓冲区的内容写会文件中
505             fp = fopen(filename,"w");
506             fputs(pbuf,fp);
507             fclose(fp);
508             free(pbuf);
509             return 1;
510     }
511 }
512 
513 int exit_user(struct MSG *msg)
514 {
515     int num = msg->m_send.u_num;
516     int i;
517     int ctl = 0;            //控制变量,初始为0,当发现要删除的QQ用户时,设为1
518     for(i=0 ; i<OL_NUM ; ++i)
519     {
520         if(g_ol_users[i].user.u_num == num)
521         {
522             ctl = 1;
523         }
524         //因为,数组就10个用户,万一你删第十个,会有错误,所以限制一下
525         if(ctl && i < OL_NUM - 1)
526         {
527             //将后一个在线用户覆盖该用户
528             g_ol_users[i] = g_ol_users[i + 1];
529         }
530 
531     }
532     //删除用户减一
533     if(ctl)
534     {
535         --OL_NUM;
536         return 1;
537     }
538     return 0;
539 }
540 
541 int my_send(struct MSG *msg)
542 {
543     char filename[128];
544     FILE *fp = NULL;
545 
546     char buf[128];
547     int valid = 0;          //判断QQ是否有效
548     char name[128];
549 
550     //验证接收方的QQ是否有效
551     sprintf(filename,"%s%d",USERS_DIR,msg->m_send.u_num);
552     fp = fopen(USERS_INFO_FILE,"r");
553     while(1)
554     {
555         fgets(buf,128,fp);
556         if(feof(fp))
557                 break;
558         if(atoi(buf) == msg->m_recv.u_num)
559         {
560             valid = 1;
561             break;
562         }
563     }
564     fclose(fp);
565     if(!valid)
566     {
567         return 0;
568     }
569 
570     //用接收方的QQ作为文件名
571     sprintf(filename,"%d",msg->m_recv.u_num);
572     fp = fopen(filename,"a");
573 
574     //昵称[QQ]:消息-----格式
575     getname(msg->m_send.u_num,name);
576     sprintf(buf,"%s[%d]:%s",name,msg->m_send.u_num,msg->m_msg);
577     fputs(buf,fp);  //写入文件中
578     fclose(fp);
579     return 1;
580 
581 }
582 
583 int my_recv(struct MSG *msg,int sockfd,struct sockaddr_in *addr)
584 {
585     FILE *fp = NULL;
586     char filename[128];
587     sprintf(filename,"%d",msg->m_send.u_num);
588     fp = fopen(filename,"r");
589     if(fp == NULL)
590     {
591         strcpy(msg->m_msg,"");
592         send_msg(sockfd,msg,addr);
593         return 0;
594     }
595     //读取成功,发送给我们的客户端
596     char buf[128];
597     while(1)
598     {
599         fgets(buf,128,fp);
600         if(feof(fp))
601                 break;
602         strcpy(msg->m_msg,buf);
603         send_msg(sockfd,msg,addr);
604 
605     }
606     strcpy(msg->m_msg,"");
607     send_msg(sockfd,msg,addr);
608     fclose(fp);
609 
610     // 删除该文件,就是保存聊天信息的文件
611     remove(filename);
612     return 1;

