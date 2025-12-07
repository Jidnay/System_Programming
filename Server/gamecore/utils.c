#include <ncurses.h>    // for KEY_UP , KEY_DOWN , KEY_LEFT , KEY_RIGHT
#include <stdlib.h>     // for rand
#include "gamecore_.h"

/*************************************************************************** 
 ***************************************************************************
 *                          Coord functions
 ***************************************************************************
****************************************************************************/
bool coord_equal( Coord c1 , Coord c2 ) {
    return c1.x == c2.x && c1.y == c2.y;
}

Coord valid_move ( Coord coord , int way ) {
    Coord new = coord;
    // Pairs ( Direction , limit of the field in that direction )
    if (way == KEY_UP && coord.y != 1 ){
        new.x = coord.x;
        new.y = coord.y-1; 
    } else if (way == KEY_DOWN && coord.y != WINDOW_SIZE-2 ) {
        new.x = coord.x;
        new.y = coord.y+1; 
    } else if (way == KEY_LEFT && coord.x != 1 ) {
        new.x = coord.x-1;
        new.y = coord.y; 
    } else if (way == KEY_RIGHT && coord.x != WINDOW_SIZE-2 ) {
        new.x = coord.x+1;
        new.y = coord.y; 
    }
    return new;
}

Coord random_coord(Game game) {
    Coord coord;
    while(true) {
        coord.y = 1 + rand() % (WINDOW_SIZE-2);
        coord.x = 1 + rand() % (WINDOW_SIZE-2);
        if(isFree(game,coord))
            break;
    }   
    return coord;
}

int DIRECTIONS[4] = { KEY_UP , KEY_DOWN , KEY_LEFT , KEY_RIGHT };
Coord random_move(Coord old) {
    return valid_move( old , DIRECTIONS[ (int) rand()%4 ] );
}

bool isFree( Game game , Coord coord ) {
    List armmed = List_get( game.Bots , coord );
    if( armmed == NULL )                                            // we did not run into a bot
        if( (armmed = List_get(game.Clients,coord)) == NULL )      // we did not run into a Player
            return List_get(game.Prizes,coord) == NULL;   // we did not run into a Prize
    return false;
}


/*************************************************************************** 
 ***************************************************************************
 *                      Auxiliary functions
 ***************************************************************************
****************************************************************************/
// only defined here
List get_client_node(Game game, int id){
    Client player;
    for (List actual = game.Clients; actual != NULL; actual = actual->next){
        player = (Client) actual->data;
        if (player != NULL && player->id == id)
            return actual;
    }
    return NULL;
}
Client get_client(Game game, int id){
    List aux = get_client_node(game, id);
    return (aux == NULL) ? (NULL) : ( (Client) aux->data);
}
bool id_available(Game game, int id){
    return get_client_node(game, id) == NULL;
}
// bool id_available(Game game, int id) {
//     Client player;

//     for( List actual = game.Clients ; actual != NULL ; actual = actual->next ){
//         player = (Client) actual->data;
//         if( player != NULL && player->id == id )
//             return true;
//     }

//     return false;
// }

int random_id(Game game) {
    int id;
    if(game.nclients <26)
        while(true) {       
            id = 'A' + rand() % 26;
            if(id_available(game, id))// verify if this character is already used
                break;
        }
    else
        while(true) {       
            id = 'a' + rand() % 26;
            if(id_available(game, id))// verify if this character is already used
                break;
        }
    return id;
}

int Prize_to_ASCII(int prize) {
    //change to ascii value
    if (1 <= prize && prize <= 5)
        return 49 + prize;
    return ' ';
}


