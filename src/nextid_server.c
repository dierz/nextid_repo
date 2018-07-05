#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdio.h>
#include<memory.h>
#include<pthread.h>
#include<mysql.h>
void *connection_handler(void *);

int main(int argc , char *argv[])
{
	//Initialization of variables
   int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
    	//print the error message
        printf("Could not create socket");
    }
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {    
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
         //Creating thread for accepted client
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}

// handler for accepted connection socket
void *connection_handler(void *socket_desc)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    int d=0;
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char client_message[2000];
     
     conn= mysql_init(NULL);

    if (!mysql_real_connect(conn, "localhost","root", "", "counter", 0, NULL, 0)) 
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return NULL;
    }

    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        client_message[read_size]='\0';
        printf("%s\n",client_message);
    if(strcmp(client_message, "gen")==0){
        char q1[300],q2[300],q3[300];
        // 3-Step algorithm : Insert --> Select --> Delete
        snprintf(q1, 300, "insert into nextid() values() ");
        snprintf(q2, 300, "select * from nextid ");
        
        if (mysql_query(conn, q2)) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            return NULL;
        }
        res = mysql_use_result(conn);
        row = mysql_fetch_row(res);
        
        if(row==NULL){
            printf("row is null\n");
            return NULL;
        }

        d=atoi(row[0]);
        snprintf(q3, 300, "delete from nextid where id = %d", d);
        mysql_free_result(res);

        if (mysql_query(conn, q1)) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            return NULL;
        }

        res = mysql_use_result(conn);
        mysql_free_result(res);

        if (mysql_query(conn, q3)) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            return NULL;
        }
        res = mysql_use_result(conn);
        mysql_free_result(res);

        snprintf(client_message, 2000, "%d", d);

    }
    else{
    	// In case of wrong option printed
        snprintf(client_message, 2000, "Unknown command");
    }
        write(sock , client_message , strlen(client_message));
    }
     
    if(read_size == 0)
    {
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    //Free the socket pointer
    free(socket_desc);
    mysql_close(conn);
    return 0;
}
