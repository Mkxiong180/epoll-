#ifndef UTILITY_H_INCLUDE
#define UTILITY_H_INCLUDE

#include<iostream>
#include<list>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>
using namespace std;

//clients_list save all the clients's socket
list<int> clients_list;

//server ip
#define SERVER_IP "10.170.37.74"

//server port
#define SERVER_PORT 8888

//epoll size
#define EPOLL_SIZE 5000

//message buffer size
#define BUF_SIZE 0xFFFF

#define SERVER_WELCOME "welcome you join to the chat room! your chat id is: Client #%d"

#define SERVER_MESSAGE "ClientID %d say>>%s"

#define EXIT "EXIT"

#define CAUTION "There is only one in the room"


//将文件描述符设置为非阻塞方式（利用fcntl函数）
int setnonblocking(int sockfd)
{
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
	return 0;
}

//将文件描述符fd添加到epollfd标示的内核时间表中，并注册EPOLLIN和EPOOLET事件，EPOLLIN是数据可读文件，EPOOLET表明是ET工作方式，最后将文件描述符设置非阻塞方式
/**
	*@param epollfd:epoll句柄
	*@param fd:文件描述符
	*@param enable_et:enable_et=true,
	采用epoll的ET工作方式，否则采用LT工作方式
**/
void addfd(int epollfd,int fd,bool enable_et)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	if (enable_et)
		ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	setnonblocking(fd);
	printf("fd added to epoll!\n\n");
}

/*
	*@param clientfd:socket descriptor
	*@return len;
*/
int sendBroadcastmessage(int clientfd)
{
	//buf[BUF_SIZE] receive new chat message
	//message[BUF_SIZE] save format message
	char buf[BUF_SIZE], message[BUF_SIZE];
	bzero(buf, BUF_SIZE);//将前BUF_SIZE个字符清0
	bzero(message, BUF_SIZE);

	//receive message
	printf("read form client(clientID=%d)\n", clientfd);
	int len = recv(clientfd, buf, BUF_SIZE, 0);

	if (len == 0)//client closed connection
	{
		close(clientfd);
		clients_list.remove(clientfd);//server remove the client
		printf("ClientID=%d closed.\n now there are %d client in the room\n", clientfd, (int)clients_list.size());
	}
	else   //broadcast message
	{
		if (clients_list.size() == 1)//this means there is only one in the room
		{
			send(clientfd, CAUTION, strlen(CAUTION), 0);
			return len;
		}
		//format message to broadcast
		sprintf(message,SERVER_MESSAGE,clientfd,buf);

		list<int>::iterator it;
		for (it = clients_list.begin(); it != clients_list.end(); it++)
		{
			if (*it != clientfd)
			{
				if (send(*it, message, BUF_SIZE, 0) < 0)
				{
					perror("error");
					exit(-1);
				}
			}
		}
	}
	return len;
}
#endif


