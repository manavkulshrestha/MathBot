#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

#define BUFFER_LEN 128
#define FLAG_LEN 64

/*
    Arguments:
        char *ip - host ip to connect to.
        int port - port to connect to.

    Description:
        Opens a socket and connects to the server using the host ip and port specified.
        Note that the function exits if it can't create a socket, get host, or fails to
        connect.

    Returns:
        int fd - the file descriptor for the server.
*/
int open_client_fd(char *ip, int port) {
    int fd;
    struct hostent *hp;
    struct sockaddr_in serv_addr;

    // Create socket
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Error: couldn't create socket. errno = %i.\n", errno);
        exit(errno);
    }

    // Get server information
    if((hp = gethostbyname(ip)) == NULL) {
        fprintf(stderr, "Error: couldn't get hostname.\n");
        exit(-2);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)hp->h_addr_list[0], hp->h_length);

    serv_addr.sin_port = htons(port);

    // Connect to the server
    if(connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
       fprintf(stderr, "ERROR: failed to connect. errno = %i.\n", errno);
       exit(errno);
    }

    return fd;
}

/*
    Arguments:
        char *question -  character array containing the question (of specified form).

    Description:
        Solves the problem specified by the question.

    Returns:
        int answer - the solution to the problem question.
        -1 - couldn't parse question correctly.

*/
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
        default:
            answer = -1;
    }

    return answer;
}

int main(int argc, char **argv) {
    int fd, len, temp;
    char buf[BUFFER_LEN], flag[FLAG_LEN];

    // Checking for appropriate number of arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <id> <port> <host>\n", argv[0]);
        exit(argc);
    }

    // Getting file descriptor
    fd = open_client_fd(argv[3], atoi(argv[2]));

    // Initial identification message
    sprintf(buf, "cs230 HELLO %s\n", argv[0]);

    while(1) {
        // Send buffer
        len = strlen(buf);
        switch(temp = send(fd, buf, len, 0)) {
            case -1:
                fprintf(stderr, "Error: couldn't send string. errno = %i.\n", errno);
                exit(errno);
            default:
                if(temp != len) {
                    char *partial;
                    strncpy(partial, buf, temp);
                    fprintf(stderr, "Error: partial string sent: '%s'.\n", partial);
                    exit(temp);
                }
        }

        // Recieve to buffer
        switch(recv(fd, buf, BUFFER_LEN, 0)) {
            case -1:
                fprintf(stderr, "Error: couldn't recieve from server. errno = %i.\n", errno);
                exit(errno);
                break;
            case 0:
                fprintf(stderr, "Error: server terminated connection. Solution may be incorrect.\n");
                exit(0);
        }
        len = strlen(buf);
        buf[len] = '\0';

        // Break out of loop if it doesn't contain a problem (thus, must contain the flag)
        if(strstr(buf, "BYE"))
            break;

        // Solve the problem
        if((temp = solve(buf)) == -1) {
            fprintf(stderr, "Error: couldn't parse question.\n");
            exit(-1);
        }

        // Set buffer to the solution to the question
        sprintf(buf, "cs230 %i\n", temp);
    }

    // Close file descriptor
    close(fd);

    sscanf(buf, "cs230 %s BYE\n", flag);
    printf("Flag captured: %s\n", flag);
    // df65ed32e8c67d962bc96127e1f860a747479da0a8ef15ca3df86f83120f13c0

    return 0;
}