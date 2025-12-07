#include <stdlib.h>     // for malloc
#include <ncurses.h>
#include "api_.h"

// only defined here
bool isClient(int id){ return (id >= 'A' && id <= 'Z') || (id >= 'a' && id <= 'z') ; }
bool isBot(int id){ return id=='*';}
bool isPrize(int id){ return id > '0' && id < '6';}
/*************************************************************************** 
 ***************************************************************************
 *                  High Level Interface to display the game
 ***************************************************************************
****************************************************************************/
struct api_ncurses {
    WINDOW* my_win;
    WINDOW* message_win;
    WINDOW* amount_win;
    WINDOW* question_win;
};

/***************************************************************************
 *                          Auxiliar Functions
****************************************************************************/
WINDOW* init_amount_window() {
    /* creates a Movement window and draws a border */
    WINDOW* my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
    keypad(my_win, true);
    wrefresh(my_win);
    return my_win;
}

WINDOW* init_message_window() {
    /* creates a Message window */
    WINDOW * message_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, WINDOW_SIZE);
    box(message_win, 0 , 0);
    
    wrefresh(message_win);
    return message_win;
}

WINDOW* init_amount_window_border() {
    /* creates a Movement window border */
    WINDOW* amount_win = newwin(9, WINDOW_SIZE, WINDOW_SIZE, WINDOW_SIZE);
    box(amount_win, 0 , 0);	
    wrefresh(amount_win);
    return amount_win;
}
WINDOW* init_question_window(){
    WINDOW* question_win =newwin(9, WINDOW_SIZE, WINDOW_SIZE, 0); 
    box(question_win, 0 , 0);	
    wrefresh(question_win);
    return question_win;
}

/* print clients HP and id */
void print_MessageWindow(WINDOW* win, List clients) {
    int count = 1;
    werase(win);
    box(win, 0 , 0);	

    // char* aux;
    for ( List actual = clients ; actual != NULL ; actual = actual->next , count ++ ){
        if(actual->data != NULL) {

            // aux = string_builder(actual->data);
            // mvwprintw(win, count, 1, "%s", aux );
            Client c = (Client) actual->data;
            mvwprintw(win, count, 1, "%c %d",c->id,c->HP );
            // free(aux);
        }
    }
    wrefresh(win);

}



/***************************************************************************
 *                  Functions to manage Display structure
****************************************************************************/
Display Display_init() {

    static int already_init = true;
    if (already_init){
        initscr();			    /* Start curses mode 		        */
        cbreak();				/* Line buffering disabled	        */
        keypad(stdscr, TRUE);   /* We get F1, F2 etc..		        */
        noecho();			    /* Don't echo() while we do getch   */
        curs_set(0);            /* Makes curser invisible           */
        already_init = false;
    }

    Display me = (Display) malloc( sizeof(struct api_ncurses) );
    me->my_win = init_amount_window();
    me->message_win = init_message_window();
    me->amount_win = init_amount_window_border();
    me->question_win = init_question_window();
    return me;
}

void Display_delete(Display disp) { 
    endwin();
    free(disp); 
}

void Display_erase(WINDOW* win){
    werase(win);
    box(win, 0 , 0);
}

void clear_question_win(Display disp){
    werase(disp->question_win);
    box(disp->question_win, 0 , 0);
}

void Display_update_symbol(Display api, Coord coord, int symbol ) {
    wmove(api->my_win, coord.y, coord.x);
    waddch(api->my_win, symbol);
}

int Display_get_char(Display disp) {
    return wgetch(disp->my_win);
}

void Display_Questions(Display disp){
    Display_erase(disp->question_win);
    mvwprintw(disp->question_win, 1, 1,"You just died...");
    mvwprintw(disp->question_win, 3, 1,"Continue to play?");
    mvwprintw(disp->question_win, 4, 1,"1-Yes");
    mvwprintw(disp->question_win, 5, 1,"other keys-Leave");
    mvwprintw(disp->question_win, 7, 1,"Remain Time:");
    wrefresh(disp->question_win);
}

void Display_number(Display disp, int number){
    mvwprintw(disp->question_win, 7, 14,"%d", number);
    if(number != 10)
        mvwprintw(disp->question_win, 7, 15," ");
    wrefresh(disp->question_win);
}

void Display_Online(Display disp){
    Display_erase(disp->question_win);
    mvwprintw(disp->question_win, 1, 4,"%s", "Chase Them");
    mvwprintw(disp->question_win, 2, 3,"%s", "  Online!!!");

    //SMILE
    mvwprintw(disp->question_win, 4, 2,"   (4)   (4)");
    mvwprintw(disp->question_win, 5, 2,"       L");
    mvwprintw(disp->question_win, 6, 2,"  *         *");
    mvwprintw(disp->question_win, 7, 2,"    *******");
    
    wrefresh(disp->question_win);

}
void Display_string(Display disp, char* string){
    Display_erase(disp->question_win);
    mvwprintw(disp->question_win, 4, 1,"%s", string);
    wrefresh(disp->question_win);
}

/*************************************************************************** 
 ***************************************************************************
 *              Field_Status related functions
 ***************************************************************************
****************************************************************************/

// only defined here
void Field_Status_Message(Game game, Message* message) {

    // Initialization
    int i=0;
    for(i=0; i< FIELD_SIZE;i++){
        message->field.field_status[0][i]=-1;
        message->field.field_status[1][i]=-1;
        message->field.field_status[2][i]=-1;
        message->field.field_status[3][i]=-1;
    }

    // Add Clients in their respective place
    i=0;
    for( List C = game.Clients; C != NULL; C=C->next){
        message->field.field_status[0][i] = ((Client)C->data)->id;
        message->field.field_status[1][i] = ((Client)C->data)->HP;
        message->field.field_status[2][i] = ((Client)C->data)->coord.x;
        message->field.field_status[3][i] = ((Client)C->data)->coord.y;
        i++;
    }

    // Add Bots in their respective place
    for( List C = game.Bots; C != NULL; C=C->next){
        message->field.field_status[0][i] = '*';
        // message->field.field_status[1][i] = ((Bot*)C->data)->nbots;
        message->field.field_status[2][i] = C->coord.x;
        message->field.field_status[3][i] = C->coord.y;
        i++;
    }

    // Add Prizes in their respective place
    for( List C = game.Prizes; C != NULL; C=C->next) {
        message->field.field_status[0][i] = *((int*)C->data);
        message->field.field_status[2][i] = C->coord.x;
        message->field.field_status[3][i] = C->coord.y;
        i++;
    }
}


// for server to update his display and send Field_Status messages to every client
void Display_Game( Display disp , Game game , List connections ) {

    // create Field_Status
    Message message;
    Field_Status_Message( game , &message );
    message.type = FIELD_STATUS;

    // send Field_Status to every client

    for ( List actual = connections ; actual != NULL ; actual = actual->next ){
        if( ((Client_Con) actual->data)->in) 
            // its not my problem if he does not receive the message
            TCP_Send( ((Client_Con) actual->data)->con , message );
    }
    // update server display
    Field_Status_Display( disp , message.field, false );

}


// for client to update his display
void Field_Status_Display( Display disp , Field_Status field, int id) {
    int count = 1;
    int next_column=1;
    // Clear current display
    Display_erase(disp->my_win); 
    Display_erase(disp->message_win);
    Display_erase(disp->amount_win);

    //
    int total_clients = 0;
    int total_bot = 0;
    int total_prize = 0;

    for(int i=0; i< FIELD_SIZE; i++) {

        int id = field.field_status[0][i];
        if ( id != -1 ){
            Coord coord;
            coord.x = field.field_status[2][i];
            coord.y = field.field_status[3][i];
            Display_update_symbol(disp, coord, id);
            if( isClient(id) ) {
                int HP = field.field_status[1][i];
                mvwprintw(disp->message_win, count, next_column, "%c %d", id, HP);
                if(count%18 == 0){      // case the coloum is full move to next coloum
                    next_column += 6;
                    count = 1;
                }
                else
                    count++;
                total_clients++;
            }         
            if(isBot(id))
                total_bot++;
            if(isPrize(id))
                total_prize++;
        } 
    }

    mvwprintw(disp->amount_win, 1, 1, "Clients  %3d", total_clients );
    mvwprintw(disp->amount_win, 2, 1, "Prizes   %3d", total_prize );
    mvwprintw(disp->amount_win, 3, 1, "Bots     %3d", total_bot );
    if(id)
        mvwprintw(disp->amount_win, 4, 1, "You        %c", id);
    mvwprintw(disp->amount_win, 7, 1,"%s", "IST");
    mvwprintw(disp->amount_win, 7, 9,"%s", "By: GF YJ");
    wrefresh(disp->amount_win);
    wrefresh(disp->my_win);
    wrefresh(disp->message_win);
}


