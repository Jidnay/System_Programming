#include <sys/socket.h>
#include <netinet/in.h> // for struct sockaddr_in ???
#include <arpa/inet.h>  // for htons
// #include <sys/un.h>
#include <unistd.h>     // for close
#include <stdlib.h>     // for malloc
#include <string.h>     // for strcmp and memset
#include <errno.h>      // for error constants (detect connection failures)
#include <stdbool.h>    // for bool type
#include "api_.h"
#define MAX_STACK_SIZE 10

#include <stdio.h>

/***************************************************************************
 * 
 * 
 *                          Basic TCP Interface
 * 
 * 
****************************************************************************/
struct _conn {
    int sock_fd;
    struct sockaddr_in* otheraddr;
};

/***************************************************************************
 *                          Auxiliar Functions
****************************************************************************/
struct sockaddr_in* aux_addr_init( char* ADDRESS , int port ) {
    struct sockaddr_in* addr = (struct sockaddr_in*) malloc( sizeof(struct sockaddr_in) );
    memset(addr, 0, sizeof(*addr) );

    addr->sin_family = AF_INET;
    addr->sin_port = port;
    addr->sin_addr.s_addr = inet_addr(ADDRESS);

    return addr;
}

Connection TCP_Init( char* IP , int port ) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
        return NULL;

    Connection aux = (Connection) malloc( sizeof(struct _conn) );
    aux->sock_fd = sock_fd;
    aux->otheraddr = aux_addr_init(IP,port);
    
    return aux;
}


/***************************************************************************
 *                  Functions to manage TCP structures
****************************************************************************/


/******************************************************************************
 * TCP_Connect()
 * Arguments: 
 *          IP   - server IP
 *          port - server port      
 * Returns:         
 *          Connection, with TCP socket and struct addrinfo* created      
 * Side-Effects:    
 *          none 
 * Description:     
 *          Creates a Connection to the server.
 *****************************************************************************/
Connection TCP_Connect( char* IP , int port ) {
    Connection aux = TCP_Init(IP,port);
    if( aux == NULL )
        return NULL;

    if( connect(aux->sock_fd, (struct sockaddr*) aux->otheraddr , sizeof(struct sockaddr_in) ) != 0 ) {
        free( aux->otheraddr );
        free(aux);
        return NULL;
    }

    return aux;
}
/******************************************************************************
 * TCP_Listen()
 * Arguments: 
 *          IP   - server IP
 *          port - server port      
 * Returns:         
 *          Connection, with TCP socket and struct addrinfo* created      
 * Side-Effects:    
 *          none 
 * Description:     
 *          Creates a Connection, with a listener TCP socket and a 
 *          struct sockaddr_in*.
 *****************************************************************************/
Connection TCP_Listen( char* IP , int port ) {
    Connection aux = TCP_Init(IP,port);     // INADDR_ANY
    if( aux == NULL )
        return NULL;

    if ( bind(aux->sock_fd, (struct sockaddr*) (aux->otheraddr), sizeof(struct sockaddr_in) ) != 0 ) {
        free( aux->otheraddr );
        free(aux);
        return NULL;
    }

    if( listen(aux->sock_fd, MAX_STACK_SIZE) != 0 ) {
        free( aux->otheraddr );
        free(aux);
        return NULL;
    }

    return aux;
}

/******************************************************************************
 * TCP_Accept()
 * Arguments:       
 *          listener - listener Connection
 * Returns:         
 *          Connection with client's accepted info      
 * Side-Effects:    
 *          None
 * Description:     
 *          Accepts a TCP client.
 *****************************************************************************/
Connection TCP_Accept( Connection listener ) {

    // New Client connection
    Connection client = (Connection) malloc( sizeof(struct _conn) );
    client->otheraddr = (struct sockaddr_in*) malloc( sizeof(struct sockaddr_in) );
    socklen_t addr_len = sizeof(struct sockaddr_in);;

    client->sock_fd = accept( listener->sock_fd , (struct sockaddr*) (client->otheraddr) , &addr_len );

    if(client->sock_fd < 0) {
        free( client->otheraddr );
        free(client);
        return NULL;
    }

    return client;
}


/******************************************************************************
 * TCP_Delete()
 * Arguments:       
 *          con - pointer to Connection* to delete
 * Returns:         
 *          none     
 * Side-Effects:    
 *          none 
 * Description:     
 *          Deletes a Connection structure, setting its pointer to NULL.
 *****************************************************************************/
void TCP_Delete(Connection* con) {
    if(*con == NULL)
        return;
    close( (*con)->sock_fd );
    if( (*con)->otheraddr != NULL )
        free((*con)->otheraddr);
    free((*con));
    *con = NULL;
}


bool TCP_Still_Connected( Connection con ) {
    // Send a dummy packet (client wont read it)
    return !(send( con->sock_fd , NULL, 0, MSG_NOSIGNAL) == -1 );//&& errno == ENOTCONN);
}

/***************************************************************************
 *                  Functions to send and receive Messages
****************************************************************************/

/******************************************************************************
 * TCP_Send()
 * Arguments:       
 *          con - Connection to the receiver
 *          message - message to send
 * Returns:         
 *          ture if message was sent, false otherwise.      
 * Side-Effects:    
 *          none 
 * Description:     
 *          Sends a TCP message to the given receiver. 
 *****************************************************************************/
bool TCP_Send(Connection con,Message message) {
    ssize_t sent_bytes = 0;
    ssize_t total_bytes = sizeof(Message);
    const void *data = &message;
    
    if( !TCP_Still_Connected(con) )
        return false;

    while (sent_bytes < total_bytes) {  // to ensure everything is sent
        ssize_t bytes = send( con->sock_fd , data + sent_bytes , total_bytes - sent_bytes , 0 );
        if ( bytes < 0 || !TCP_Still_Connected(con) )
            return false;     // connection lost?
        sent_bytes += bytes;
    }

    return true;
}
/******************************************************************************
 * TCP_Read()
 * Arguments:       
 *          con - Connection to the sender
 * Returns:         
 *          Received message. If there was any connection problem, message.type = DEAD      
 * Side-Effects:    
 *          none 
 * Description:     
 *          Receives a UDP message. 
 *****************************************************************************/
Message TCP_Read(Connection con) {
    Message message_received = { .type = DEAD };
    ssize_t received_bytes = 0;
    ssize_t total_bytes = sizeof(Message);
    void *data = &message_received;

    if( !TCP_Still_Connected(con) )
        return message_received;

    while (received_bytes < total_bytes) {
        ssize_t bytes = recv( con->sock_fd , data + received_bytes , total_bytes - received_bytes , 0 );
        
        if ( bytes < 0 || !TCP_Still_Connected(con) ) {
            message_received.type = DEAD;
            return message_received;            // connection lost?
        }
        received_bytes += bytes;
    }

    return message_received;
}


/***************************************************************************
 *                  Client's Notification Auxiliary Function
****************************************************************************/
void Client_Con_delete( void* client ) {

    Client_Con client_ = (Client_Con) client;

    if( client_->con != NULL )
        TCP_Delete( &(client_->con) );   // will never happen

    free(client_);

}

