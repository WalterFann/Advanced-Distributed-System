#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
int main(int argc, char const *argv[])
{
	struct sockaddr_in address;
	int sock, new_sock, valread;
	int addrlen = sizeof(address);
	char buffer[100000];
	char end[4] = "end";
	char *ACK = "ACK from server";
	char *END = "end one ite";
	int opt=1;
	sock = socket(AF_INET, SOCK_STREAM, 0);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(8080);

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		printf("setsockopt failed"); 
		return -1;
	}

	if(bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0){
		printf("\nBind Failed \n");
		return -1;
	}

	listen(sock, 3);

	//For Q1
	//new_sock = accept(sock, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	//while(1)
	//{
	//	valread = read(new_sock, buffer, sizeof(buffer));
	//	buffer[valread] = '\0';
	//	printf("%d\n",valread);
	//	if(buffer[valread-3] == 'e' &&buffer[valread-2] == 'n' &&buffer[valread-1] == 'd'){
	//		printf("end session");
	//		break;
	//	}
	//	send(new_sock, ACK, strlen(ACK), 0);
	//}

	//For Q2
	//new_sock = accept(sock, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	//int count = 0;
	//while(1)
	//{
	//	valread = read(new_sock, buffer, sizeof(buffer));
	//	count += valread;
	//	if(buffer[valread-3] == 'e' &&buffer[valread-2] == 'n' &&buffer[valread-1] == 'd'){
	//		printf("end session\n");
	//		send(new_sock, ACK, strlen(ACK), 0);
	//		break;
	//	}
	//	if(buffer[valread-3] == 'E' &&buffer[valread-2] == 'N' &&buffer[valread-1] == 'D'){
	//		printf("end one ite\n");
	//		send(new_sock, END, strlen(END), 0);
        //        }
	//}

	//For Q3
	while(1)
	{
		new_sock = accept(sock, (struct sockaddr *)&address, (socklen_t*)&addrlen);
		if(fork() == 0)
		{
			close(sock);
			while(1)
			{
				valread = read(new_sock, buffer, sizeof(buffer));
				if(buffer[valread-3] == 'e' &&buffer[valread-2] == 'n' &&buffer[valread-1] == 'd'){
                        		printf("end session\n");
                        		send(new_sock, ACK, strlen(ACK), 0);
                        		break;
                		}
				send(new_sock, ACK, strlen(ACK), 0);
			}
			close(new_sock);
			exit(0);
		}
		else{
			close(new_sock);
		}
	}
	return 0;
}
