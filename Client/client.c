#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include "../client_api.h"
#include<unistd.h>

#define IP "127.0.0.1"
#define PORT 54363 // entre 49152-65535

/*************************************************************************** 
 ***************************************************************************
 *                      Global client_addr Variables
 ***************************************************************************
****************************************************************************/
Connection client_addr = NULL;
int id;
Display disp;
bool running = true;
bool dead = false;

pthread_mutex_t disp_mtx = PTHREAD_MUTEX_INITIALIZER;

void waiting_ten_s(){

    Display_Questions(disp);
    for( int i = REVIVE_TIME ; i >= 0 ; i-- ) {
        Display_number(disp, i);
        sleep(1);
        if(!dead){
            clear_question_win(disp);
            Display_Online(disp);
            break;
        }
    }

    if (dead && running){
        dead = false;
        TCP_Delete(&client_addr);
        Display_delete(disp);
        printf("Client CLOSE\n");
        exit(0);
    }
    pthread_exit(NULL);
}

void Online_Screen( void* args ) {

    Message message;
    pthread_t wait;
    while(running) {
        
        // receive new message
        // se estiver morto nao read
        if (dead == false) {

            message = TCP_Read(client_addr);
            if(sizeof(message) <= 0)
                break;

            pthread_mutex_lock(&disp_mtx);
            // in case i took too long to revive (i'm out of the game)
            if( !TCP_Still_Connected(client_addr) )
                break;

            if ( message.type == FIELD_STATUS )
                Field_Status_Display( disp , message.field , id);

            else if( message.type == DEAD ) { 
                pthread_create( &wait , NULL , (void *)waiting_ten_s , &client_addr );
                dead = true;
            }
            else if( message.type == SERVER_CLOSED) {
                dead = false;

                TCP_Delete(&client_addr);
                Display_delete(disp);

                printf("Client CLOSE\n");
                exit(0);
            }
            pthread_mutex_unlock(&disp_mtx); 

        } 

    }

}


/*************************************************************************** 
 ***************************************************************************
 *                          Main Function
 ***************************************************************************
****************************************************************************/
int main(int argc, char ** argv) {

    if(argc>=3)
        client_addr = TCP_Connect( argv[1] , atoi(argv[2]) );
    else{
        client_addr = TCP_Connect(IP,PORT);
    }

    if( client_addr == NULL ) {
        printf("\nServer is Down! Please try later....\n");
        return 0;
    }

    // receive Ball_Info    (i actually don't need it)
    Message message = TCP_Read(client_addr);
    id = message.ball.id;
    if( message.type == SERVER_FULL ) {
        printf("\nServer is Full! Please try later....\n");
        TCP_Delete(&client_addr);
        return 0;
    }

    disp = Display_init();
    Display_Online(disp);
    // Init function to always keep screen updated
    pthread_t notify;
    pthread_create( &notify , NULL , (void *)Online_Screen , NULL );

    int key = -1;
    while(running) {

        key = Display_get_char(disp);
        pthread_mutex_lock(&disp_mtx);  // manda key e recebe menssagem de volta sincronizar isso
        if(dead){                       // in case Online_Screen received a dead message
            if (key == '1'){
                message.type = CONTINUE;
                TCP_Send(client_addr, message);
                dead = false;
            }
            else
                running = false;
        }
        else if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN) {

            // send movement message to server
            message.type = BALL_MOV;
            message.mov.mov = key;
            if( !TCP_Send(client_addr, message) ) {
                printf("\nServer is Down! Disconnecting...\n");
                running = false;
            }

        } else if(key == 'q'){
            message.type = CLOSE_SOCKET;
            TCP_Send(client_addr, message);
            running = false;
        }
        pthread_mutex_unlock(&disp_mtx);

    }

    pthread_cancel(notify);
    TCP_Delete(&client_addr);
    Display_delete(disp);
    printf("Client CLOSE\n");

    return 0;
}

