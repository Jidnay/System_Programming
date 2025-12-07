#ifndef CLIENT_API_INCLUDED
#define CLIENT_API_INCLUDED

#include <stdbool.h>

#define REVIVE_TIME 10
#define FIELD_SIZE 400

/*************************************************************************** 
 ***************************************************************************
 *  To Use the given library "client_api.a", flag "-lncurses" must be 
 *  included in executable dependency's statement
 ***************************************************************************
****************************************************************************/

/*************************************************************************** 
 ***************************************************************************
 *                      Game Message's Interface
 ***************************************************************************
****************************************************************************/
typedef enum _types { CONTINUE , BALL_MOV , BALL_INFO , FIELD_STATUS , SERVER_FULL , DEAD, CLOSE_SOCKET, SERVER_CLOSED } Message_Types;

typedef struct Ball_information {int id;} Ball_Info;
typedef struct ball_move {int mov;} Ball_movement;
typedef struct _field {
    int field_status[4][FIELD_SIZE];
                /* row 0 -> id */
                /* row 1 -> HP when applicable*/
                /* row 2 -> x -*/
                /* row 3 -> y -*/
} Field_Status;

typedef struct _message {
    Message_Types type;
    Ball_Info ball;
    Ball_movement mov;
    Field_Status field;
} Message;


/*************************************************************************** 
 ***************************************************************************
 *                          Basic TCP Interface
 ***************************************************************************
****************************************************************************/
typedef struct _conn* Connection;

Connection TCP_Connect( char* IP , int port );
void TCP_Delete(Connection* con);
bool TCP_Send(Connection con, Message message);     // returns true if message was sent
Message TCP_Read(Connection con);                   // do not block in case of a connection failure
bool TCP_Still_Connected( Connection con );

/*************************************************************************** 
 ***************************************************************************
 *              High Level Interface to display the game
 *                              (chasethem.c)
 ***************************************************************************
****************************************************************************/
typedef struct api_ncurses* Display;

Display Display_init();
void Display_delete(Display disp);
int Display_get_char(Display disp);

// functions for client to personalize it's question window
void Display_Questions(Display disp);
void Display_number(Display disp, int number);  // present a countdown
void clear_question_win(Display disp);
void Display_Online(Display disp);

// for client to update his display
void Field_Status_Display( Display disp , Field_Status field , int id);

#endif