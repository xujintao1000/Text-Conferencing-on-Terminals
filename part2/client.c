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
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <stdbool.h>

#define MAX_NAME 2000
#define MAX_DATA 2000

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

void error(const char *msg);
int init_client(const char* port, const char * address);
void read_from_server(int sockfd);
char * Encode(struct message client_message);
char* get_command(char * buffer, int index);

int main(int argc, char *argv[])
{
    bool is_login = false;
    char buffer[2000];
    int sockfd;
    bool admin = false;
    char client_name[200];
    while(1)
    {
        admin = false;
        bzero(buffer, 2000);
        
        fflush(stdin);
        printf("\n>>> ");
        fgets(buffer, 2000, stdin);
        //buffer[strlen(buffer)-1] = '\0';
        
        // command case 
        if(buffer[0] == '/'){
            printf("Receive Admin Command: \t");
            char* command = get_command(buffer, 1);
            // Init create socket  -> 0
            if(strncmp(command, "login", 5) == 0){
                admin = true;
                if(is_login == false){
                    is_login = true;
                    printf("Client is try to connection with server\n");
                    struct message client_message;
                    char* client_ID = get_command(buffer, 2);
                    char* password = get_command(buffer, 3);
                    char * server_IP = get_command(buffer, 4);
                    char * server_port = get_command(buffer, 5);
                    bzero(client_name, 200);
                    strncpy(client_name, client_ID, strlen(client_ID));
                    client_message.type = 0;
                    client_message.size = strlen(password);
                    strncpy(client_message.source, client_ID, strlen(client_ID));
                    if(client_message.source[strlen(client_ID)] != '\0') client_message.source[strlen(client_ID)] = '\0';
                    strncpy(client_message.data, password, strlen(password));

                    sockfd = init_client(server_port, server_IP);
                    char * encode_message = Encode(client_message);
                    //printf("Encode of message is (%s) \n", encode_message);
                    bzero(buffer, 2000);
                    strncpy(buffer, encode_message, strlen(encode_message));
                    
                    free(server_IP);
                    free(server_port);
                    free(client_ID);
                    free(password);
                }
                else{
                    printf("(%s) has already been login. \n", client_name);
                }
            }
            // Exit -> 1
            else if(strncmp(command, "quit", 4) == 0){
                if(is_login == true ){
                    bzero(buffer, 2000);
                    strcpy(buffer, "1");
                    return 0;
                }
            }
            else if(is_login == true && (strncmp(command, "EXIT", 4) == 0 || strncmp(command, "logout", 6) ==0 || strncmp(command, "Exit", 4)==0)){
                admin = true;
                printf("Receive message of close the socket\n");
                bzero(buffer, 2000);
                strcpy(buffer, "1");
                //buffer[0] = '1';
                
            }
            else if(is_login == true && ((strncmp(command, "QUERY", 5) == 0) || (strncmp(command, "list", 4) == 0) )){
                admin = true;
                printf("Receive message of show the clients and sessions\n");
                bzero(buffer, 2000);
                strcpy(buffer, "5");
            }
            else if(is_login == true && ((strncmp(command, "JOIN", 4) == 0) || ((strncmp(command, "joinsession", 11)) == 0))){
                char* conference_ID = get_command(buffer, 2);
                bzero(buffer, 2000);
                strcat(buffer, "2:");
                if(*(conference_ID+ (strlen(conference_ID)-1)) == '\n') conference_ID[strlen(conference_ID)-1] == '\0';
                if(*(conference_ID+ (strlen(conference_ID))) == '\n') conference_ID[strlen(conference_ID)-1] == '\0';
//                if(conference_ID[strlen(conference_ID)] == '\n') conference_ID[strlen(conference_ID)-1] == '\0';
                printf("Create Conference_ID is: (%s)", conference_ID);
                strcat(buffer, conference_ID);
                free(conference_ID);
                
            }
            else if(is_login == true && ((strncmp(command, "createsession", 13) == 0))){
                char* conference_ID = get_command(buffer, 2);
                bzero(buffer, 2000);
                strcat(buffer, "3:");
                if(*(conference_ID+ (strlen(conference_ID)-1)) == '\n') conference_ID[strlen(conference_ID)-1] == '\0';
                if(*(conference_ID+ (strlen(conference_ID))) == '\n') conference_ID[strlen(conference_ID)-1] == '\0';
                conference_ID[strlen(conference_ID)+1] == '\0';
                printf("Create Conference_ID is: (%s)", conference_ID);
                strcat(buffer, conference_ID);
                free(conference_ID);
                
            }
            else if(is_login == true && ((strncmp(command, "leavesession", 12) == 0))){
                char* conference_ID = get_command(buffer, 2);
                bzero(buffer, 2000);
                strcat(buffer, "4:");
                if(*(conference_ID+ (strlen(conference_ID)-1)) == '\n') conference_ID[strlen(conference_ID)-1] == '\0';
                if(*(conference_ID+ (strlen(conference_ID))) == '\n') conference_ID[strlen(conference_ID)-1] == '\0';
//                if(conference_ID[strlen(conference_ID)] == '\n') conference_ID[strlen(conference_ID)-1] == '\0';
                printf("Create Conference_ID is: (%s)", conference_ID);
                strcat(buffer, conference_ID);
                free(conference_ID);
            }
            else if((is_login == true) && (strncmp(command, "invite", 6) == 0)){
                char* name = get_command(buffer, 2);
                char* conference_name = get_command(buffer, 3);
                bzero(buffer, 2000);
                strcat(buffer, "6:");
                strcat(buffer, name);
                strcat(buffer, ":");
                strcat(buffer, conference_name);

                
                free(name);
                free(conference_name);
            }

            free(command);
        }

        if((is_login == true) && ((strncmp(buffer, "accept", 6) == 0) || (strncmp(buffer, "yes", 3) == 0))){
            bzero(buffer, 2000);
            strcat(buffer, "7:");
        }
        if((is_login == true) && ((strncmp(buffer, "reject", 6) == 0) || (strncmp(buffer, "no", 2) == 0))){
            bzero(buffer, 2000);
            strcat(buffer, "8:");
        }


        // 9->show current login status
        else if(buffer[0] == '9'){
            printf("---- Login Status ----\n");
            if(is_login == true){
                printf("  Status: (Yes) \n");
                printf("  Name: %s\n", client_name);
            }
            else{
                printf("  Status: No \n");
            }
            continue;
        }

        if(is_login){

            if(buffer[strlen(buffer)-1] == '\n') buffer[strlen(buffer)-1] = '\0';
            // if(buffer[strlen(buffer)] == '\n') buffer[strlen(buffer)] = '\0';
           printf("Client: Sending message (%s) \n", buffer);
            int n = write(sockfd, buffer, strlen(buffer));
            if(n < 0) printf("Error on writing\n");
            

            // check if exit 
            if(strncmp(buffer, "1", 1) == 0 && admin == true){
                is_login = false;
                close(sockfd);
            }

            pid_t childid;
            if(childid=fork()==0)// sub-system
            {
                read_from_server(sockfd);// listen on sub-system
                exit(0);
            }
        }
        else{
            printf("Not login yet, void input. \n");
        }
    }

    close(sockfd);
    return 0;
}


//---- Helping function -------//
//------------------------------------------------------------------------------------------------------------------//

char* get_command(char * buffer, int index){
    int i, count=0, start_index =1;
    char space = ' ';
    //printf("The size of buffer is: %d \n", (strlen(buffer)));
    bool last_word = false;
    for(i=1; i<=strlen(buffer); i++){
        if(i==strlen(buffer)){
            last_word = true;
            break;
        }
        //printf("[%c] ", buffer[i]);
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

    int size;
    if(index == 1) size = i-1;
    else if(last_word == true) size = i-start_index-1;
    else size = i-start_index;
    
    char temp[2000];
    bzero(temp, 2000);
    char* keyword = (char*) malloc(size*sizeof(char) + 100);
    
    bzero(temp, 2000);

    strncpy(temp, buffer+start_index, size);
    
    //printf("keyWord: (%s)\n", temp);
    strncpy(keyword, temp, strlen(temp));
    keyword[strlen(temp)] = '\0';
    //printf("keyWord: (%s)\n", keyword);
    return keyword;
 }

 void read_from_server(int sockfd){
    char buffer[2000];
    
    bzero(buffer, 2000);
    
    while(true){
        int n = read(sockfd, buffer, 2000);
        if(n<0){
            printf("error on reading close the Read Function from Server.\n >>>");
            exit(1);
        }
        printf("Server: %s\n", buffer);
        printf(">>> ");
        
        if(strncmp(buffer, "EXIT", 4) == 0 || strlen(buffer) <= 1){
            printf("Close the read function from Server.\n");
            close(sockfd);
            break;
        }
        bzero(buffer, 2000);

    }
}

int init_client(const char* port, const char * address){
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = atoi(port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) printf("ERROR opening socket");

    server = gethostbyname(address);
    if(server == NULL)
    {
        printf("Error, no such host");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr , (char*) &serv_addr.sin_addr.s_addr , server->h_length);
    serv_addr.sin_port = htons(portno);

    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
        printf("connection failed\n");

    return sockfd;
}

char * Encode(struct message client_message){
    char * output = (char*) malloc(sizeof(char) * 2000);
    //printf("Encode_Section: source: (%s) and Data: (%s). \n",  client_message.source, client_message.data);
    sprintf(output, "%d:%d:%s:%s", client_message.type, client_message.size, client_message.source, client_message.data);
    return output;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}