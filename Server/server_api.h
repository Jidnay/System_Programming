#ifndef SERVER_API_INCLUDED
#define SERVER_API_INCLUDED

#include <pthread.h>
#include "gamecore.h"
#include "../client_api.h"

/*************************************************************************** 
 ***************************************************************************
 *                      Basic TCP Interface for server
 ***************************************************************************
****************************************************************************/
Connection TCP_Listen( char* IP , int port );
Connection TCP_Accept( Connection listener );

// for server to update his display and send Field_Status to every client
void Display_Game( Display disp , Game game , List connections );
void Display_string(Display disp, char* string);

/*************************************************************************** 
 ***************************************************************************
 *                  Structure for Clients Notification
 ***************************************************************************
****************************************************************************/
typedef struct _client_con {
    Connection con;
    bool in;
    bool dead_state; // state waits if client wants to Conitnue to play
    int id;
}* Client_Con;

void Client_Con_delete( void* client );


#endif