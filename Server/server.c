#include <stdlib.h>     // for atoi function
#include <time.h>       // for time_t data type
#include <stdlib.h>     // for srand function
#include <unistd.h>     // for sleep function
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include "gamecore.h"
#include "server_api.h"

#define TIME_TO_DIE 14          // 4 -> TCP delay + display delay + safe issue
#define BOTDELAY 3
#define PRIZEDELAY 5
#define MINIMUMFREESPACE 50     // (WINDOW_SIZE*WINDOW_SIZE)/8 <=> pelo menos um oitavo da tela livre
#define AVAILABLESPACE(game) WINDOW_SIZE * WINDOW_SIZE - game.nprizes - game.nbots - game.nclients

#define IP "127.0.0.1"
#define PORT 54363              // entre 49152-65535


/*************************************************************************** 
 ***************************************************************************
 *                      Global Server Variables
 ***************************************************************************
****************************************************************************/
Game game = { .Clients = NULL , . Bots = NULL , .Prizes = NULL , .threads=NULL, .nbots = 10, .nprizes = 0 , .nclients = 0 , .running = true };
Display display;
List connections = NULL;   // list of type Client_Con (so that every client receives the most updated field)

pthread_t thread_bots;
pthread_t thread_prizes;
pthread_mutex_t game_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t connection_mtx = PTHREAD_MUTEX_INITIALIZER;


/*************************************************************************** 
 ***************************************************************************
 *                      Mutex Auxiliary functions
 ***************************************************************************
****************************************************************************/
void lock_mutex(pthread_mutex_t* mtx){
    int s;
    s = pthread_mutex_lock(mtx);
    if(s != 0){
        perror("mutex");
        exit(0);
    }
}

void unlock_mutex(pthread_mutex_t* mtx){
    int s;
    s = pthread_mutex_unlock(mtx);
    if(s != 0){
        perror("mutex");
        exit(0);
    }
}


/*************************************************************************** 
 ***************************************************************************
 *                      Thread Auxiliary functions
 ***************************************************************************
****************************************************************************/
pthread_t* Thread_New( void (func) (void*) , void* data ) {
    Coord useless = { .x = -1 , .y = -1 };

    pthread_t *thread;
    // new data copy
    thread = (pthread_t*) malloc(sizeof(pthread_t));
    // create thread
    pthread_create( thread , NULL , (void*) func , data );
    // store data copy
    game.threads = List_append( game.threads , useless , (void*) thread );

    return thread;
}

void Thread_Delete(void* thread) { 
    pthread_cancel(*(pthread_t*)thread);
    free( (pthread_t*) thread );
}


/*************************************************************************** 
 ***************************************************************************
 *                  connection List Auxiliary functions
 ***************************************************************************
****************************************************************************/
List connection_remove(List head, int id){

    if(head == NULL)
        return NULL;

    if( ((Client_Con)head->data)->id == id) {
        List aux_head = head->next;
        free( ((Client_Con)head->data)->con);
        free(head);
        return aux_head;
    }

    for ( List actual = head , son = head->next ; son != NULL ; actual = son , son = son->next )
        if( ((Client_Con)son->data)->id == id ) {
            actual->next = son->next;
            free( ((Client_Con)son->data)->con);
            free(son);

            return head;
        }

    return head;
}


List connection_get(List head, int id) {
    for ( List actual = head ; actual != NULL ; actual = actual->next )
        if( ((Client_Con)actual->data)->id == id)
            return actual;
    return NULL;
}


/*************************************************************************** 
 ***************************************************************************
 *              Auxiliary functions to deal with Player's Death
 ***************************************************************************
****************************************************************************/
void No_Time_to_Die(void* data) {

    int id = ((Client)data)->id;
    List actual_con = connection_get(connections, id);

    // Sending a Death message to the rammed player
    Message message;
    message.type = DEAD;
    if( !TCP_Send( ((Client_Con)actual_con->data)->con, message) )
        return;

    // wait for his answer (His thread will receive an answer)
    sleep(TIME_TO_DIE);

    // check if client choose to revive, to die, or if did not choose at all 
    lock_mutex(&connection_mtx);
    actual_con = connection_get(connections, id);
    if(actual_con == NULL || !((Client_Con)actual_con->data)->dead_state) {
        unlock_mutex(&connection_mtx);
        return;
    }
    ((Client_Con)actual_con->data)->in = false;

    unlock_mutex(&connection_mtx);

}

void verify_health_zero_node(List actual) {

    List aux = connection_get( connections , ((Client)actual->data)->id );
    Client_Con client_aux = (Client_Con)aux->data;

    lock_mutex(&connection_mtx);
    if( client_aux->dead_state == false )               // case is not waiting for answer
    {
        client_aux->dead_state = true;
        Thread_New( No_Time_to_Die , actual->data );
    }
    unlock_mutex(&connection_mtx);

}

void verify_health_zero() {
    
    if ( game.Clients == NULL )
        return;

    for ( List actual = game.Clients ; actual != NULL ; actual = actual->next )
        if( ((Client)actual->data)->HP == MINHP )
            verify_health_zero_node(actual);

}

/*************************************************************************** 
 ***************************************************************************
 *                      Main Function to Deal With Clients
 ***************************************************************************
****************************************************************************/
// Side Effects: Changes Game.Clients and players HP. Also updates server Display.
// Deletes every data allocated. Ends connection with client but and delete
// it's info from connections list.
void Client_Controller( void* client_con_ ) {

    Client_Con client_con = (Client_Con) client_con_;
    lock_mutex(&game_mtx);
    Client me = Client_Add( &game );
    client_con->id = me->id;
    unlock_mutex(&game_mtx);

    // create and send a Ball_Info Message
    Ball_Info new_ball = {.id = me->id };
    Message message = { .type = BALL_INFO , .ball = new_ball };

    if( !TCP_Send( client_con->con , message ) ) {
        TCP_Delete(&(client_con->con));               // there was some connection error
        Client_Remove( &game , &me );   // aborting client
        return;
    }
    while(game.running && client_con->in) {   // validar se o client não terminou abruptamente a connection

        // receive new message
        message = TCP_Read(client_con->con);

        // Mandou uma mensagem inválida/saiu
        if( message.type == CLOSE_SOCKET || !TCP_Still_Connected(client_con->con))
            break;
        
        // proccess client's message
        lock_mutex(&game_mtx);
        if( message.type == BALL_MOV && me->HP != 0 && game.running) {

            Ball_movement move = message.mov;
            Client_Move( &game , me , move.mov );
            verify_health_zero();  
            // Update server display and automatically send field status to every client  
            Display_Game( display , game , connections );

        }
        else if(message.type == CONTINUE && client_con->dead_state == true ) {

            me->HP = MAXHP;                             // return to play
            Display_Game( display, game, connections);
            lock_mutex(&connection_mtx);
            client_con->dead_state = false;             // not dead anymore
            unlock_mutex(&connection_mtx);

        } else {

            lock_mutex(&connection_mtx);
            client_con->dead_state = true;              // Client wants to leave
            client_con->in = false;
            unlock_mutex(&connection_mtx);

        }

        unlock_mutex(&game_mtx);

    }

    // It's my responsability to end connection
    lock_mutex(&connection_mtx);
    TCP_Delete( &(client_con->con) );
    connections = connection_remove(connections, me->id);
    unlock_mutex(&connection_mtx);

    lock_mutex(&game_mtx);
    Client_Remove( &game , &me ); 
    Display_Game( display , game , connections );
    unlock_mutex(&game_mtx);

}



/*************************************************************************** 
 ***************************************************************************
 *                      Main Function to Deal With Bots
 ***************************************************************************
****************************************************************************/
// Side Effects: Changes Game.Bots and players HP. Also updates server Display.
// Deletes every data allocated. On Game.Bots, List->data contains nothing.
void Bot_Controller( void* nothing ) {

    time_t t;
    srand((unsigned) time(&t));
    // Init Bots with a random position (there's no need to store any data on the list)
    for ( int i = 0 ; i < game.nbots && i < MAXBOTS ; i++ )
        game.Bots = List_append( game.Bots , random_coord(game) , NULL );

    while(true) {
        sleep(BOTDELAY);
        lock_mutex(&game_mtx);
        if(!game.running){
            unlock_mutex(&game_mtx);
            return;
        }
           
        Bots_Move( game , game.Bots );
        Display_Game( display , game , connections );
        verify_health_zero(); 

        unlock_mutex(&game_mtx);
    }
}

void bot_delete_list(void* bot) {}

/*************************************************************************** 
 ***************************************************************************
 *                      Main Function to Deal With Prizes
 ***************************************************************************
****************************************************************************/
// Side Effects: Changes Game.Prizes and players HP. Also updates server Display.
// On Game.Prizes, List->data contains structures of type int*.
void Prize_Controller( void* nothing ) {

    time_t t;
    srand((unsigned) time(&t));
    // Init Prizes with a random position and random value
    Prizes_Add(&game,5);
    
    while(true) {
        sleep(PRIZEDELAY);
        lock_mutex(&game_mtx);
        if(!game.running){
            unlock_mutex(&game_mtx);
            return;
        }

        Prizes_Add(&game,1);
        Display_Game( display , game , connections );

        unlock_mutex(&game_mtx);
    }

}

/*************************************************************************** 
 ***************************************************************************
 *                      Actual Server Main Function
 ***************************************************************************
****************************************************************************/
void actual_server(void* con) {
    
    Connection me = (Connection) con;
    Message message;
    Coord useless = { .x = -1 , .y = -1 };

    //Init "bot's controller"
    pthread_create( &thread_bots , NULL , (void *)Bot_Controller , NULL );

    // Init "prizes's controller"
    pthread_create( &thread_prizes , NULL , (void *)Prize_Controller , NULL );

    while(game.running) {
        // accept new client
        Connection new_client = TCP_Accept(me);
        if ( AVAILABLESPACE(game) <= MINIMUMFREESPACE || game.nclients > MAXPLAYERS) {

            message.type = SERVER_FULL;
            TCP_Send( new_client , message );   // its not my problem if he does not receive the message
            TCP_Delete( &new_client );
        } else {

            // store new connection in a list, so that it is possible for every existing thread to notify this client
            lock_mutex(&connection_mtx);
            Client_Con client_con = (Client_Con) malloc( sizeof(struct _client_con) );
            client_con->in = true;
            client_con->con = new_client;
            client_con->dead_state = false;
            connections = List_append( connections , useless , (void *) client_con );
            unlock_mutex(&connection_mtx);
  	    printw("New Client\n");
            Thread_New((void *)Client_Controller, (void*)client_con);
        }
    }
}


void disconnect_allClients(List con) {
    Message message;
    message.type = SERVER_CLOSED;
    for ( List actual_con = con ; actual_con != NULL ; actual_con = actual_con->next ) {
        TCP_Send( ((Client_Con)actual_con->data)->con , message );
        ((Client_Con)actual_con->data)->in = false;
    }
}

/*************************************************************************** 
 ***************************************************************************
 *                          Main Function
 ***************************************************************************
****************************************************************************/
int main(int argc, char ** argv) {

    Connection me;
    if(argc>=3)
        me = TCP_Listen( argv[1] , atoi(argv[2]) );
    else
        me = TCP_Listen(IP,PORT);
    if (me == NULL){
        printf("IP OR PORT not available\n");
        exit(0);
    }

    // init display
    display = Display_init();
    
    // Init actual main server function
    pthread_t server;
    pthread_create( &server , NULL , (void *)actual_server , me );
    Display_string(display, "Press Q to Close!");

    int key;
    while(1) {

        key = Display_get_char(display);
        
        if(key == 'q') {

            lock_mutex(&game_mtx);
            game.running = false;
            unlock_mutex(&game_mtx);

            lock_mutex(&connection_mtx);
            disconnect_allClients(connections);
            unlock_mutex(&connection_mtx);

            break;
        }

    }

    Display_string(display, "Server Closing...");
    pthread_join(thread_bots, NULL);
    pthread_join(thread_prizes, NULL);
    pthread_cancel(server);

    lock_mutex(&game_mtx);
    lock_mutex(&connection_mtx);

    List_delete(game.threads, Thread_Delete);
    List_delete( connections , Client_Con_delete );
    List_delete(game.Clients , client_delete_list );
    List_delete( game.Prizes , prize_delete_list );
    List_delete( game.Bots , bot_delete_list );

    unlock_mutex(&connection_mtx);
    unlock_mutex(&game_mtx);

    TCP_Delete(&me);
    Display_delete(display);


    printf("Server CLOSED\n");
    return 0;
}


