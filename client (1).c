// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
    char fileName[50];
    char file[1024];
    int sock;
}DATA;

int main(int argc, char *argv[])
{
	//int PORT = atoi(argv[1]);
	int PORT = 7001;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	//read data from file
	char* content = NULL;
	FILE* fp = fopen("command.txt","r");
	if(fp==NULL) return -1;
	size_t len;
    ssize_t bytes_read = getdelim( &content, &len, '\0', fp);
    if ( bytes_read == -1) return -1;
        
	
	//send data to server
	DATA d = {
        .fileName = "File Name",
		.sock=0
    };
	strcpy(d.file,content);   //copy "content" to file  , (pointer to table)
	int i;

	printf("file Name : %s, file : %s\n",d.fileName,d.file);
    
	//send data to server
	valread=send(sock , &d ,sizeof(d),0);
	printf("send : %d\n",valread);

	int *result;
	//receive from server
	while(valread = recv(sock , result, sizeof(result),0)){
		printf("receive : %d, result: %d\n",valread,*result);
	}
	close(sock);
	return 0;
}
