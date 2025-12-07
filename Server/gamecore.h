#ifndef GAMECORE_INCLUDED
#define GAMECORE_INCLUDED

#include <stdbool.h>

#define WINDOW_SIZE 20
#define MAXHP 10
#define MINHP 0
#define MAXPLAYERS 52   // available number of characters
#define MAXBOTS 10
#define MAXPRIZES 10
#define MAXPRIZEVALUE 5

/*************************************************************************** 
 ***************************************************************************
 *                      Coord data type and functions
 *                                  (utils.c)
 ***************************************************************************
****************************************************************************/
typedef struct _coord{
    int x;
    int y;
} Coord;

/*************************************************************************** 
 ***************************************************************************
 *                      List data type and functions
 *                              (list.c)
 ***************************************************************************
****************************************************************************/
typedef struct _list {
    void* data;
    Coord coord;
    struct _list* next;
}*List;

List List_init();
List List_append( List head, Coord coord, void* data );
List List_get( List head, Coord coord );
List List_remove( List head, Coord coord , void (*delete_data) (void*) );
void List_delete(List head , void (*delete_data) (void*) );

/*************************************************************************** 
 ***************************************************************************
 *                  Game data type and related functions
 *                              (chasethem.c)
 ***************************************************************************
****************************************************************************/
typedef struct game {

    List Clients;           // List->data contains structures of type Client
    List Bots;              // List->data contains nothing
    List Prizes;            // List->data contains structures of type int*
    List threads;
    int nbots;
    int nprizes;
    int nclients;
    bool running;

} Game;

// void Display_show( Display disp , Game game);
// void Display_show_client(Display disp , int field_status[4][30]);

/*************************************************************************** 
 ***************************************************************************
 *                  Client data type and related functions
 *                          (worlds_dynamic.c)
 ***************************************************************************
****************************************************************************/
typedef struct _client {
    int id;
    int HP;
    Coord coord;
}* Client;

Client Client_Add( Game* game );
void Client_Move( Game* game , Client client , int move );
void Client_Remove(Game* game, Client* player);
void client_delete_list(void* client);

/*************************************************************************** 
 ***************************************************************************
 *                      Bot's related functions
 *                          (worlds_dynamic.c)
 ***************************************************************************
****************************************************************************/
void Bots_Move( Game game , List Bots );

/*************************************************************************** 
 ***************************************************************************
 *                      Prize related functions
 *                          (worlds_dynamic.c)
 ***************************************************************************
****************************************************************************/
void Prizes_Add( Game* game , int quantity );
int Prize_Remove( Game* game , Coord coord );
void prize_delete_list(void* prize);

/*************************************************************************** 
 ***************************************************************************
 *                      Other Auxiliary functions
 *                          (utils.c)
 ***************************************************************************
****************************************************************************/
Coord random_coord(Game game);
Coord random_move(Coord old);

#endif

