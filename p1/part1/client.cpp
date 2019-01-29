#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>

int main(int argc, char const *argv[])
{
	struct sockaddr_in address;
	int sock, valread;
	struct sockaddr_in serv_addr;
	char *hello = "Hello from client";
	char buffer[1000000];
	char readin[1000];
	struct timeval start, end;
	long secs, usecs;
	float mtime;
	memset(buffer, 0x30, sizeof(buffer));
	sock = socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);

	inet_pton(AF_INET, "18.221.175.74", &serv_addr.sin_addr);

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	//For Q1
	//for(int i=0;i<100;i++){
	//	gettimeofday(&start, NULL);
	//	send(sock,"1",1,0);
	//	read(sock, readin, sizeof(readin));
	//	gettimeofday(&end, NULL);
	//	secs  = end.tv_sec  - start.tv_sec;
	//	usecs = end.tv_usec - start.tv_usec;
	//	mtime = ((secs) * 1000 + usecs/1000.0);
	//	printf("%f\n", mtime);
	//}
	
	//For Q2
	//for (int j = 0; j<100;j++){
	//	gettimeofday(&start, NULL);
	//	for (int i=0;i<10;i++){
	//		send(sock, buffer, sizeof(buffer), 0);
	//	}
	//	send(sock, "END", 3, 0);
	//	read(sock, readin, sizeof(readin));
	//	gettimeofday(&end, NULL);
	//	secs  = end.tv_sec  - start.tv_sec;
	//	usecs = end.tv_usec - start.tv_usec;
	//	mtime = ((secs) * 1000 + usecs/1000.0);
	//	printf("%f\n", mtime);
	//}
	
	//For Q3 E1
	//for (int j=0; j<100;j++){
	//	sock = socket(AF_INET, SOCK_STREAM, 0);
	//	gettimeofday(&start, NULL);
	//	connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	//	send(sock,"1",1,0);
	//	read(sock, readin, sizeof(readin));
	//	gettimeofday(&end, NULL);
	//	secs  = end.tv_sec  - start.tv_sec;
	//	usecs = end.tv_usec - start.tv_usec;
	//	mtime = ((secs) * 1000 + usecs/1000.0);
	//	printf("%f\n", mtime);
	//	send(sock, "end", 3, 0);
	//}
	
	//For Q3 E2
	for (int j=0;j<100;j++){
		sock = socket(AF_INET, SOCK_STREAM, 0);
		gettimeofday(&start, NULL);
		connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
		for(int i=0;i<100;i++){
			send(sock, "1", 1, 0);
			read(sock, readin, sizeof(readin));
		}
		gettimeofday(&end, NULL);
		secs  = end.tv_sec  - start.tv_sec;
		usecs = end.tv_usec - start.tv_usec;
		mtime = ((secs) * 1000 + usecs/1000.0);
		printf("%f\n", mtime);
		send(sock, "end", 3, 0);
	}

	read(sock, readin, sizeof(readin));
	printf("%s\n", readin);
	return 0;
}
