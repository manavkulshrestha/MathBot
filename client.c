#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>

#define MAX_QUESTIONS 2000
#define MAX_QUESTION_LEN 50
#define BYE_LEN 3

int open_client_fd(char *host, int port) {
	int fd;

	struct sockaddr_in server_addr;
	struct addrinfo hints, *addr;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
  	hints.ai_socktype = SOCK_STREAM;
  	hints.ai_protocol = 0;
  
 	if((errno = getaddrinfo(host, NULL, &hints, &addr)) != 0) {
 		fprintf(stderr, "Failed to get host information. %s.\n", gai_strerror(errno));
		exit(errno);
 	}

  	if((fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1) {
		fprintf(stderr, "Failed to create socket. errno = %i.\n", errno);
		exit(errno);
	}

	memset(&server_addr, 0, sizeof(server_addr));

  	server_addr.sin_family = addr->ai_family;
    server_addr.sin_addr.s_addr = &(((struct sockaddr_in *)addr->ai_addr)->sin_addr); //what
    server_addr.sin_port = htons(port);

    if(connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
    	fprintf(stderr, "Failed to connect. errno = %i.\n", errno);
    	exit(errno);
    }	

	freeaddrinfo(addr);
	addr = NULL;

	return fd;
    }
}

int solve(char *question) {
	int num1, num2, answer;
	char operator;

	sscanf(question, "cs230 STATUS %i %c %i\n", &num1, &operator, &num2);

	switch(operator) {
		case '+':
			answer = num1+num2;
			break;
		case '-':
			answer = num1-num2;
			break;
		case '*':
			answer = num1*num2;
			break;
		case '/':
			answer = num1/num2;
			break;
		case '%':
			answer = num1%num2;
			break;
		default:
			answer = -1;
	} 

	return answer;
}

int main(int argc, char **argv) {
	if(argc != 4) {
		fprintf(stderr, "Usage: %s <host> <port>.\n", argv[0]);
		exit(argc);
	}

	char *host = argv[0];
	int port = atoi(argv[1]);
	char *ip = argv[2];

	int len, temp;
	char *buf = (char *) malloc(MAX_QUESTION_LEN*sizeof(char));
	sprintf(buf, "cs230 HELLO %s\n", host);

	int client_fd = open_client_fd(host, port);

	while(1) {
		len = strlen(buf)+1;
		switch(temp = send(client_fd, buf, len, 0)) {
			case -1:
				fprintf(stderr, "Error sending string. errno = %i.\n", errno);
				exit(errno);
			default:
				if(temp != len) {
					char *partial;
					strncpy(partial, buf, temp);
					fprintf(stderr, "Partial string sent: '%s'.\n", partial);
					exit(temp);
				}
		}

		switch(recv(client_fd, buf, MAX_QUESTION_LEN, 0)) {
			case -1:
				fprintf(stderr, "Error recieving from server. errno = %i.\n", errno);
				exit(errno);
				break;
			case 0:
				fprintf(stderr, "Server terminated connection. Solution may be incorrect.\n");
				exit(0);
		}
		len = strlen(buf);
		buf[len] = '\0';

		if(strcmp(buf+len-BYE_LEN, "BYE") == 0)//can be done betetr with sscanf maybe
			break;

		if((temp = solve(buf)) == -1) {
			fprintf(stderr, "Operator not recognized.\n");
			exit(-1);
		}

		sprintf(buf, "cs230 %i\n", temp);
	}

	close(client_fd);

	printf("Flag captured: '%s'.\n", buf);//do

	free(buf);
	buf = NULL;

	return 0;
}