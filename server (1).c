#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

typedef struct{
    char fileName[50];
    char file[1024];
    int sock=0;
}DATA;

typedef struct{
    pthread_mutex_t *mutex; //pour envoyer le résultat eu client, un par un
    int port;               //port du serveur esvlave
    int fd[2];              //pipe entre Main process et le Thread
}DATAServer;

void sending(int sock,int len, void* arg){   //sending function
    int h=send(sock , arg,len,0);
    printf("message sent : %d\n",h);
}

void *ThreadFunct(void* arg){
    //get data
    DATAServer *dServer = (DATAServer*)arg;
    /////

    //connecte to slave
    int PORT_slave = dServer->port, valread, valsend;
    int slave_server = 0;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};

	if ((slave_server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
        return NULL;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT_slave);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
	    return NULL;
	}
    
    if (connect(slave_server, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return NULL;
	}

    //slave server Connected
    DATA d;
    int *result=malloc(sizeof(int));
    while(read(dServer->fd[0], &d, sizeof(DATA)) > 0)  //waiting data from pipe
    {
        //send to slave
        sending(slave_server,sizeof(d),&d);

        //receive from slave
        valread = recv(slave_server , result, sizeof(int*),0);
        if(valread==0) break;                                  //if we failed to send data to slave(disconnected)
        printf("Thread. Receive : %d, Result from slave : %d\n",valread,*result);

        //send to client
        pthread_mutex_lock(dServer->mutex);
        sending(d.sock,sizeof(result),result);
        pthread_mutex_unlock(dServer->mutex);
    }    
    printf("Slave Server %d disconected.\n",PORT_slave);
    

    return NULL; 
}
int main(int argc, char *argv[])
{
    //mutex && thread 
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    //read slave servers file
    FILE *file  = fopen("LSlave.txt", "r");
    int nbServers, i;                        //number of servers
    if (file == NULL)                        // test for files not existing. 
    {   
        perror("file");
        exit(EXIT_FAILURE);
    } 
    fscanf(file,"%d\n",&nbServers);          //extract first line of file
    /////

    /////create thread and bind server data

    pthread_t *thr = malloc(nbServers*sizeof(pthread_t));     //liste des threads
    DATAServer *dServer =malloc(nbServers*sizeof(DATAServer)); //liste data envoyer au threads

    int Ports[nbServers];

    for(i=0;i<nbServers;i++){                                   //copy ports from file to table Ports[], we dont copy names (%*s)
        fscanf(file,"%*s %d\n",&P[i]);
    }
    fclose(file);

    for(i=0;i<nbServers;i++){                                   //Threads creation
        dServer[i].mutex = &mutex;
        dServer[i].port = Ports[i];
        pipe(dServer[i].fd);
        pthread_create(&thr[i],NULL,ThreadFunct,&dServer[i]);
    }
    pthread_mutex_destroy(&mutex);

    ///////////////////Create main server
    
	//int PORT = atoi(argv[1]);
	int PORT = 7001;

    int serverSocket, client_socket, valread;
    int opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
	char buffer[1024] = {0};

    // Creating socket file descriptor
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (serverSocket==0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    //Bind - Forcefully attaching socket to the port 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(serverSocket, (struct sockaddr *)&address, addrlen)==-1)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //Listen
    if (listen(serverSocket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //////Infinite LOOP
    while(1){
        printf("Waiting ! client\n");
        //accept
        if ((client_socket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
        printf("Client : %d accepted.\n",client_socket);

        //receive from client
        DATA d;
        valread = recv(client_socket , &d, sizeof(DATA),0);
        printf("file Name : %s, file : %s\n",d.fileName,d.file);
        printf("Main receive : %d\n",valread);

        //send to thread -> slave    
        char * token = strtok(d.file, "\n");
        int i = 0;
        // loop through the string to extract all commandes seperated by "\n"
        while( token != NULL ) {
            DATA d2;
            d2.sock = client_socket;            
            strcpy(d2.file, token);                     //copy just one commande
            write(dServer[i].fd[1], &d2,sizeof(d2));    //send data to thread i
            token = strtok(NULL,"\n");
            i++;
            i=i%nbServers;
        } 
    }
    for(i=0;i<nbServers;i++){  //test
        pthread_join(thr[i],NULL);
    }
    close(serverSocket);
    close(client_socket);
    return 0;
}