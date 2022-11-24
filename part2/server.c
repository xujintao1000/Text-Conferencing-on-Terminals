/*
/--------------Server & Client program are successful to run on below command-------------------/
/-----------------------------------------------------------------------------------------------/
Name                                                            Description 
/login <client ID> <Password> <server-IP> <server-port>         Initial with client, and try to make a connection with server. 
/logout                                                         Logout from the server, but not close the program
/joinsession <session ID>                                       Join with new session, if no found, create a new one.
/leavesession <session ID>                                      Leave the session
/createsession <session ID>                                     Create the session
/list                                                           Show the client list and conference list of current state on server.
/quit                                                           Close the client program
9                                                               Get the self client information
/invite <client name> <conference name>                         invite client to join with the session (yes)=>accept to join , (no)=>reject
/-----------------------------------------------------------------------------------------------/
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_NAME 2000
#define MAX_DATA 2000
#define MAX_CONNECT 10

// Initial Structure 
//-----------------------------------------------------------------------------------------------------------------//
struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

struct conference{
    bool fill;
    unsigned char name[MAX_NAME];
    int client_num;
    int connect_list[10];
};

struct client {
    bool fill;
    unsigned char * name;
    unsigned char * password;
    int socket_ID;
};

// Initial Helping function 
//-----------------------------------------------------------------------------------------------------------------//
void error(const char *message);
void str_echo(int newsockfd, int client_index);
int init_server(int argc, char * argv[]);
char * Encode(struct message client_message);
char* Decode_Word(char* buffer, int index);
struct message * Decode(char* input);
int accept_new_connection(int sockfd);
void init_global_variable();
char * find_client_name(int sockfd, struct client * client_list);
void create_conferece(int connect_list[], char * buffer, int current_socket, int client_num, struct client * client_list );
void init_variable(struct conference * conference_list, struct client * client_list);


int main(int argc, char * argv[])
{
    // Initial with server socket ID;
    int sockfd = init_server(argc, argv);
    struct conference conference_list [10];
    struct client client_list [10]; 
    char buffer[2000];
    struct client invite_client;
    char * invite_conference_name = (char*) malloc(sizeof(char) * 1000);
    invite_client.fill = false;
    invite_client.socket_ID = -1;

    // Initial variable (conference_list and client_list)   
    init_variable(conference_list, client_list);

    bzero(buffer, 2000);

    fd_set current_sockets, read_sockets;
    FD_ZERO(&current_sockets);
    FD_SET(sockfd, &current_sockets);

    


    while(1){
        read_sockets = current_sockets;
        
        if(select(FD_SETSIZE, &read_sockets, NULL, NULL, NULL) < 0){
            error("select error");
        }
        for(int i=0; i<FD_SETSIZE; i++){
            if(FD_ISSET(i, &read_sockets)){
                if(i == sockfd){
                    int newsockfd = accept_new_connection(sockfd);
                    FD_SET(newsockfd, &current_sockets);
                }
                else{
                    bzero(buffer, 2000);
                    int n = read(i, buffer, 2000);
                    if(n < 0) error("Error on reading");
                   
                    printf("Client %d: (%s) -->  " , i, buffer);
                    
                    // 0->initial connection with client
                    if(buffer[0] == '0'){           
                        // add client information to the database
                        struct message * client_message = Decode(buffer);
                        int j;
                        for(j=0; j<10; j++){
                            if(client_list[j].fill == false){
                                printf("<Initial Client>\n");
                                client_list[j].fill = true;
                                // bzero(client_list[j].name, 2000);
                                // bzero(client_list[j].password, 2000);
                               
                                client_list[j].password = client_message->data;
                                client_list[j].name = client_message->source;

                                printf("Length of client_message.data = %d, client_message.source = %d", strlen(client_message->data), strlen(client_message->source));
                                
                                // for(int k=0; k<strlen(client_message->data); k++){
                                //     client_list[j].password[k] = client_message->data[k];
                                // }
                                // for(int k=0; k<strlen(client_message->source); k++){
                                //     client_list[j].name[k] = client_message->source[k];
                                // }


                                // strncpy(client_list[j].password, client_message.data, strlen(client_message.data));
                                // strncpy(client_list[j].name, client_message.source,  strlen(client_message.source));

                                printf("Client Name: (%s), Password is (%s)\n", client_list[j].password, client_list[j].name);
                                printf("<Receive the name and password>\n");
                               // client_list[j].name = client_message.source;
                                client_list[j].socket_ID = i;
                                //client_list[j].password = client_message.data;
                                break;
                            }
                        }
                        bzero(buffer, 2000);
                        if(i==10) strncpy(buffer, "LO_NAK", 6);
                        else strncpy(buffer, "LO_ACK", 6);
                    }
                    else if(buffer[0] == '1'){
                        bzero(buffer, 2000);
                        printf("Clear the client socket %d", i);
                        for(int j=0; j<10; j++){
                            
                            if(client_list[j].fill == true && client_list[j].socket_ID == i){
                                client_list[j].fill = false;
                                // free(client_list[j].name);
                                // free(client_list[j].password);
                                
                                bzero(client_list[j].name, 2000);
                                bzero(client_list[j].password, 2000);
                                bzero(buffer, 2000);
                                strcpy(buffer, "EXIT");
                                FD_CLR(i, &current_sockets);
                                break;
                            }
                            
                        }
                    }
                    // Join conference -> 2
                    else if(buffer[0] == '2'){
                        
                        char * conference_ID = (char*) malloc(sizeof(char) * 1000);
                        memset(conference_ID, 0, 1000);

                        if(strlen(buffer) < 3){
                            strncpy(conference_ID, "NULL", 4);
                        }
                        else{
                            strncpy(conference_ID, buffer+2, strlen(buffer)-2);
                        }
                        
                        printf("Get conference ID: %s\n", conference_ID);
                        // check if exist
                        bool finish = false;
                        for(int j=0; j<10; j++){
                            //printf("Compare conference_list[j].name (%s) and conference_ID (%s).\n", conference_list[j].name,  conference_ID);
                            if(conference_list[j].fill == true && strncmp(conference_list[j].name, conference_ID, strlen(conference_ID)) == 0){
                                printf("find the exist conference ID");
                                int num = conference_list[j].client_num;
                                conference_list[j].connect_list[num] = i;
                                conference_list[j].client_num = num + 1;
                                finish = true;
                                break;
                            }
                        }


                        // If the conference name does not exist on conference list
                        if(finish == false){
                            for(int j=0; j<10; j++){
                                if(conference_list[j].fill == false){
                                    conference_list[j].fill = true;
                                    bzero(conference_list[j].name,2000);
                                    strcpy(conference_list[j].name, conference_ID);
                                    conference_list[j].connect_list[conference_list[j].client_num] = i;
                                    conference_list[j].client_num = conference_list[j].client_num + 1;
                                    break;
                                }
                            }
                        }

                        bzero(buffer, 2000);
                        if(finish == true){
                            sprintf(buffer, "JN_ACK, find conference(%s), and successfully join in.\n", conference_ID);
                        }
                        else {
                            sprintf(buffer, "JN_NAK, does not find conference(%s), but join with new conference.\n", conference_ID);
                        }
                        
                        free(conference_ID);
                    }
                    // Create new conference session
                    else if(buffer[0] == '3'){

                        char * conference_ID = (char*) malloc(sizeof(char) * 1000);
                        memset(conference_ID, 0, 1000);

                        if(strlen(buffer) < 3){
                            strncpy(conference_ID, "NULL", 4);
                        }
                        else{
                            strncpy(conference_ID, buffer+2, strlen(buffer)-2);
                        }
                        
                        printf("Get conference ID: %s\n", conference_ID);


                        for(int j=0; j<10; j++){
                            if(conference_list[j].fill == false){
                                conference_list[j].fill = true;
                                bzero(conference_list[j].name,2000);
                                strcpy(conference_list[j].name, conference_ID);
                                conference_list[j].connect_list[conference_list[j].client_num] = i;
                                conference_list[j].client_num = conference_list[j].client_num + 1;
                                break;
                            }
                        }

                        bzero(buffer, 2000);
                        sprintf(buffer, "NEW_SESS, Create new conference (%s)\n", conference_ID);
                    }
                     // Leave conference session -> 4
                    else if(buffer[0] == '4'){

                        char * conference_ID = (char*) malloc(sizeof(char) * 1000);
                        memset(conference_ID, 0, 1000);

                        if(strlen(buffer) < 3){
                            strncpy(conference_ID, "NULL", 4);
                        }
                        else{
                            strncpy(conference_ID, buffer+2, strlen(buffer)-2);
                        }
                        
                        printf("Get conference ID: %s\n", conference_ID);
                        bool founded = false;
                        for(int j=0; j<10; j++){
                            if(conference_list[j].fill == true){
                                for(int k=0; k<10; k++){
                                    //printf("Found socket ID is %d and i is %d", conference_list[j].connect_list[k], i);
                                    if(conference_list[j].connect_list[k] == i){
                                        //printf("Found socket ID is %d", conference_list[j].connect_list[k]);
                                        conference_list[j].connect_list[k] = 0;
                                        founded = true;
                                        conference_list[j].client_num=conference_list[j].client_num -1;
                                        
                                        break;
                                    }
                                }
                            }
                            if(founded == true) break;
                        }

                        bzero(buffer, 2000);
                        sprintf(buffer, "ACK, Remove new conference(%s)\n", conference_ID);
                    }


                    else if(buffer[0] == '5'){
                        bzero(buffer, 2000);
                        printf("\n------ Client List ------\n");
                        printf("No.\tSockID\tName\tPassword\t\n");
                        strcat(buffer, "\n------ Client List ------\n");
                        strcat(buffer, "No.\tSockID\tName\tPassword\t\n");
                        
                        for(int j=0; j<10; j++){
                            if(client_list[j].fill == true){
                                char temp [1000];
                                sprintf(temp,"%d\t%d\t%s\t%s\n", j, client_list[j].socket_ID, client_list[j].name, client_list[j].password);
                                printf("%s", temp);
                                strncat(buffer, temp, strlen(temp));
                            }
                        }
                        
                        strcat(buffer, "\n----------END-----------\n");
                        printf("\n------ Conference List ------\n");
                        printf("No.\tName\tClient_list\t\n");
                        strcat(buffer, "\n------ Conference List ------\n");
                        strcat(buffer, "No.\tName\tClient_list\t\n");
                        int count = 0;
                        for(int j=0; j<10; j++){
                            if(conference_list[j].fill == true){

                                count++;

                                char * temp  = (char *) malloc(sizeof(char) * 500);
                                memset(temp, 0, 500);
                                sprintf(temp,"%d\t%s\t", j, conference_list[j].name);
                                strncat(buffer, temp, strlen(temp));
                                
                                strcat(buffer,"\t");
                                for(int k=0; k<conference_list[j].client_num; k++){


                                    for(int m = 0;  m<10; m++){
                                        if(client_list[m].fill == true && client_list[m].socket_ID == conference_list[j].connect_list[k]){
                                            printf("%s\t", client_list[m].name);
                                            strncat(buffer, client_list[m].name, strlen(client_list[m].name));
                                            strcat(buffer,"\t");
                                        }
                                    }
                                }
                                strcat(buffer,"\n");
                                printf("%s", buffer);
                            }
                        }
                        if(count==0){
                            strcat(buffer, "Empty\n");
                        }
                        strcat(buffer, "\n----------END-----------\n");
                        printf("\n----------END-----------\n");
                    }
                    else if(buffer[0] == '6'){      // one client ask the other client to join with the conference
                        char * client_name = Decode_Word(buffer, 2);
                        int client_socket;
                        char * conference_name = Decode_Word(buffer, 3);
                        bzero(buffer, 2000);
                        //printf("Client: %s, invite to join Conference: %s. \n", client_name, conference_name);

                        char * sender_client_name;

                        for(int j=0; j<10; j++){
                            if(client_list[j].fill == true && strncmp(client_list[j].name, client_name, strlen(client_name))==0){
                                client_socket = client_list[j].socket_ID;
                            }
                        }

                        for(int j=0; j<10; j++){
                            if(client_list[j].fill == true && i == client_list[j].socket_ID){
                                sender_client_name = client_list[j].name;
                            }
                        }

                        sprintf(buffer, "Client(%s) Invite you to join with Conference(%s).\n", sender_client_name, conference_name);
                        printf("Client(%s) Invite you to join with Conference(%s).\n", sender_client_name, conference_name);
                        write(client_socket, buffer, 2000);

                        invite_client.fill = true;
                        invite_client.name = client_name;
                        invite_client.socket_ID = client_socket;
                        strcpy(invite_conference_name, conference_name);

                        bzero(buffer, 2000);
                        strcpy(buffer, "ACK");

                    }
                    else if(buffer[0] == '7'){      // the other client accept to join with the client 
                        if(invite_client.fill == true){
                            printf("Get conference ID: %s\n", invite_conference_name);
                            // check if exist
                            bool finish = false;
                            for(int j=0; j<10; j++){
                                //printf("Compare conference_list[j].name (%s) and invite_conference_name (%s).\n", conference_list[j].name,  invite_conference_name);
                                if(conference_list[j].fill == true && strncmp(conference_list[j].name, invite_conference_name, strlen(invite_conference_name)) == 0){
                                    printf("find the exist conference ID");
                                    int num = conference_list[j].client_num;
                                    conference_list[j].connect_list[num] = invite_client.socket_ID;
                                    conference_list[j].client_num = num + 1;
                                    finish = true;
                                    break;
                                }
                            }

                            invite_client.fill = false;
                            bzero(buffer, 2000);
                            if(finish == true){
                                sprintf(buffer, "JN_ACK, find conference(%s), and successfully join in.\n", invite_conference_name);
                            }
                            else {
                                sprintf(buffer, "JN_NAK, does not find conference(%s), but join with new conference.\n", invite_conference_name);
                            }
                        }
                        else{
                            printf("Does not invite you yet.\n");
                        }
                    }
                    else if(buffer[0] == '8'){      // the client reject with the client 

                    }


                   // ---------------------------------------------------------
                    printf("Sending Message is (%s)\n", buffer);
                    write(i, buffer, 2000);
                    printf("Successfully send the message to client \n");

                    for(int j=0; j<10; j++){
                        if(conference_list[j].fill == true && conference_list[j].client_num >=2){
                                // each variable indicate:  conference_members, message, me as a member, conference_number, given all the client list infor.
                                create_conferece(conference_list[j].connect_list, buffer, i, conference_list[j].client_num, client_list);
                        }
                    }
                    

                    bzero(buffer, 2000);
                }
            }
        }
    }
    close(sockfd);
    return 0;

}

//------Helping function ------//
//--------------------------------------------------------------------------------------------------------------//

void str_echo(int newsockfd, int client_index)
{
    int n;
    char buffer[255];
    bzero(buffer, 255);
    pid_t pid = getpid();

     while(1){
        n = read(newsockfd, buffer, 255);
        if(n < 0) error("Error on reading");
        printf("Client: %s" , buffer);
        bzero(buffer, 255);
        fgets(buffer, 255, stdin);

        n = write(newsockfd, buffer, strlen(buffer));
        if(n<0) error("Error on writing. ");
        int i = strncmp("Bye", buffer, 3);
        if(i==0){
            printf("child process: %d exited.\n",pid);
            break;
        } 
    }
}



int init_server(int argc, char * argv[]){
    if(argc < 2)
    {
        printf("Port No not provided. Program terminated");
        exit(1);
    }

    int sockfd, portno;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        error("Error opening socket.");
    }
    printf("Server: Socket id = %d\n",sockfd);

    bzero((char *) & serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]); // get the port number into int type

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    //printf("server ip =%s\n",inet_ntoa(serv_addr.sin_addr));// server  ip address
    
    int i;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(i));   // connect multiple socket with local address

    if(bind(sockfd, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0 )
    {
        error("binding failed.");
    }
    printf("Server: Bind success!\n");
    listen(sockfd, 5);
    return sockfd;
}

char* Decode_Word(char* buffer, int index){
    int i, count=0, start_index =0;
    char space = ':';

    for(i=0; i<=strlen(buffer); i++){
        if(i==strlen(buffer)){
            break;
        }
        if(buffer[i] == space){
            count++;
            if(count == index){
                break;
            }
            else{
                start_index = i+1;
            }
        }
    }
    
    char*  keyword  = (char*) malloc((i-start_index));
    strncpy(keyword, buffer+start_index, i-start_index);
    
    //printf("Keyword (%s), strlen(buffer): %d, i: %d, start_Index: %d", keyword, strlen(buffer), i,start_index);
    return keyword;
}

struct message * Decode(char* input){
    struct message * output = (struct message*)malloc(sizeof(struct message));

    char* type =  Decode_Word(input, 1);
    char* size =  Decode_Word(input, 2);
    unsigned char* source = Decode_Word(input, 3);
    unsigned char* data = Decode_Word(input, 4);
    output->size = atoi(size);
    output->type = atoi(type);
    
    strncpy(output->source, source, strlen(source));
    strcpy(output->data, data);

    //printf("Output Source: (%s), Data (%s)  \n", output.source, output.data);

    free(size);
    free(type);

    return output;
}

int accept_new_connection(int sockfd){
    socklen_t clilen; // socklen_t -- unsigned int
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) & cli_addr, & clilen);
    return newsockfd;
}



void error(const char *message)
{
    perror(message);
    exit(1);
}



char * find_client_name(int sockfd, struct client * client_list){
    char * client_name = (char*) malloc(sizeof(char)* 2000);
    memset(client_name, 0, 2000);
    for(int i=0; i<10; i++){
        if(client_list[i].socket_ID == sockfd){
            strncpy(client_name, client_list[i].name, strlen(client_list[i].name));
        }
    }
    return client_name;
}


void create_conferece(int connect_list[], char * buffer, int current_socket, int client_num, struct client * client_list ){
    char * new_buffer = (char * ) malloc(sizeof(char) * 2000);

    bool broadcast = false;
    // check if there exist current_socket on the current conference.
    for(int i=0; i<client_num; i++){
        if(connect_list[i] == current_socket)
            broadcast = true;
    }

    //
    if(broadcast == true){
        for(int i=0; i<client_num; i++){
            if(connect_list[i] != current_socket){
                memset(new_buffer, 0, 2000);
                char * client_name = find_client_name(connect_list[i], client_list);

                sprintf(new_buffer, "Client(%s): %s", client_name, buffer);
                printf("Sending (%s)\n", new_buffer);
                write(connect_list[i], new_buffer, strlen(new_buffer));
                free(client_name);
                
            }
        }
    }
    
    free(new_buffer);
}

void init_variable(struct conference * conference_list, struct client * client_list){
    for(int i=0; i<10; i++){
        client_list[i].fill = false;
        client_list[i].socket_ID = -1;
        conference_list[i].client_num = 0;
        conference_list[i].fill = false;
        for(int j=0 ;j<10; j++){
            conference_list[i].connect_list[j] = 0;
        }
    }
}
