#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h
#include <arpa/inet.h>

typedef struct{
    char fileName[50];
    char file[1024];
    int sock;
}DATA;

int main(int argc, char *argv[])
{
	int PORT = atoi(argv[1]);
	//int PORT = 7007;

    int mySocket, Main_socket,valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof address;
    char buffer[1024] = {0};
    char *hello = "Hello from slave server";

    // Creating socket file descriptor
    mySocket = socket(PF_INET, SOCK_STREAM, 0);
    if (mySocket==0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // Forcefully attaching socket to the port 8080
    if (setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    //Bind - Forcefully attaching socket to the port 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(mySocket, (struct sockaddr *)&address, addrlen)==-1)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    //Listen
    if (listen(mySocket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //accept
    if ((Main_socket = accept(mySocket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    ///Server Connected
    DATA d;
	while(1){
        
        printf("_________________\n");

        //receive data
        valread = recv(Main_socket , &d, sizeof(DATA),0);
        printf("Receive : %d || file Name : %s, file : %s\n",valread,d.fileName,d.file);

        //process
        int *result;
        int check = system(d.file);
        result = &check;
        printf("resulat : %d\n",check);

        //send data
        valread=send(Main_socket , result ,sizeof(result),0);
        printf("message sent : %d\n",valread);
    } 
    close(mySocket);
    close(Main_socket);
}