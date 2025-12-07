#include <stdlib.h>     // just for NULL constant
#include <stdio.h>      // for sprintf
#include "gamecore_.h"

#define MAX(x,y) (x)>=(y) ? (x) : (y)
#define MIN(x,y) (x)<=(y) ? (x) : (y)


// functions only defined here
void hit_player( Client player );
char* client_build_string(void* client);        // why do we need this one?


/*************************************************************************** 
 ***************************************************************************
 *                      Bot's related functions
 ***************************************************************************
****************************************************************************/
// Only defined here
Coord bot_move( Game game , Coord old ) {
    Coord new = random_move(old);

    List armmed = List_get( game.Bots , new ); 
    if( armmed != NULL )                                            // we ran into another bot
        new = old;                                                  // we should remain in the same place
    else if( (armmed = List_get( game.Clients , new )) != NULL ) {  // we ran into a player
        hit_player( (Client) armmed->data );
        new = old;                                                  // we should remain in the same place
    }
    else if( (armmed = List_get( game.Prizes , new )) != NULL ){
        new = old;
    }
    return new;                                                     // bots can be at the same place as prizes
}


void Bots_Move( Game game , List Bots ) {
    // move every bot on a valid direction
    for ( List actual = Bots ; actual != NULL ; actual = actual->next )
        actual->coord = bot_move( game , actual->coord );
}


/*************************************************************************** 
 ***************************************************************************
 *                      Prize's related functions
 ***************************************************************************
****************************************************************************/

void Prizes_Add( Game* game , int quantity ) {
    int *value;
    for ( int i = 0 ; i < quantity && game->nprizes < MAXPRIZES ; i++ ) {
        // new data copy
        value = (int*) malloc(sizeof(int));
        *value = '1' + rand() % MAXPRIZEVALUE;

        // store data copy
        game->Prizes = List_append( game->Prizes , random_coord(*game) , (void*) value );

        game->nprizes++;
    }
}

int Prize_Remove( Game* game , Coord coord ) {

    List actual = List_get( game->Prizes , coord );
    if(actual == NULL)
        return 0;

    game->nprizes--;   
    int value = *( (int*) actual->data );
    game->Prizes = List_remove( game->Prizes , coord , prize_delete_list );

    return value;             
}

// auxiliary function to make List struct delete allocated data. 
// In this case int* with the value of the prize.
void prize_delete_list(void* prize) { free( (int*) prize ); }


/*************************************************************************** 
 ***************************************************************************
 *                      Client's related functions
 ***************************************************************************
****************************************************************************/
// Only defined here
void hit_player( Client player ) {
    player->HP = MAX( MINHP , player->HP - 1 );
}

// Only defined here
char* client_build_string(void* client) { 
    char* aux = (char*) malloc( 5 * sizeof(char) );
    Client c = (Client) client;
    sprintf(aux,"%c %d",c->id,c->HP);
    return aux;
}


Client Client_Add( Game* game ) {
 
    Client new_client = (Client) malloc( sizeof(struct _client) );
    new_client->id = random_id(*game);
    new_client->HP = MAXHP;
    new_client->coord = random_coord(*game);

    game->Clients = List_append( game->Clients , new_client->coord , (void*) new_client );
    game->nclients++;
 
    return new_client;
}


void Client_Move( Game* game , Client client , int move ) {

    List Client_ = List_get(game->Clients, client->coord);
    Coord new_coord = valid_move ( client->coord , move );

    List rammed = List_get( game->Bots , new_coord );
    if( rammed == NULL ) { 

        // we did not run into another bot
        if( (rammed = List_get( game->Clients , new_coord )) == NULL ) {

            // we did not run into a player
            int prize = Prize_Remove( game , new_coord );
            if( prize != 0 )    // we ran into a prize
                client->HP = MIN( MAXHP , client->HP + prize - '0' );

            // we must update our coordinates
            client->coord = new_coord;
            Client_->coord = new_coord;

        } else {
            // we ran into a player
            if( ((Client) rammed->data)->HP != 0)
                hit_player( (Client) rammed->data );

            client->HP = MIN( MAXHP , client->HP + 1 );  
        } 
    }

    return;
}

    
void Client_Remove(Game* game, Client* player) {
    game->nclients--;
    game->Clients = List_remove( game->Clients, (*player)->coord , client_delete_list ); 
    *player = NULL;
}

void client_delete_list(void* client) { free( (Client) client ); }

