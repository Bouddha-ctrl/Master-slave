#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>

typedef struct{
    char fileName[50];
    char *file;
    int sock;
}DATA;


int main(){
    

    //read data from file
	char* content = NULL;
	FILE* fp = fopen("command.txt","r");
	if(fp==NULL) return -1;
	size_t len;
    ssize_t bytes_read = getdelim( &content, &len, '\0', fp);
    if ( bytes_read == -1) return -1;
        
    DATA d = {
        .fileName = "File Name",
        .file = content,
		.sock=0
    };
    printf("%s\n",d.file);



    return 0;
}