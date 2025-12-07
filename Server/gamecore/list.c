#include <stdlib.h>     // just for NULL constant
#include "gamecore_.h"

/***************************************************************************
 * 
 *                          List functions
 * 
****************************************************************************/
List List_init() {
    List node;
    node = (List) malloc( sizeof(List) );
    if (node == NULL) {
        exit(-1);
    }
    node->next = NULL;
    node->data = NULL;
    return node;
}


List List_append( List head, Coord coord, void* data ) {
    List last = List_init();
    last->coord = coord;
    last->data = data;
    List aux = head;
    if (head == NULL){
        head = last;
        head->next = NULL;
    }
    else{
        while(aux->next != NULL)
            aux = aux->next;
        aux->next = last;
        last->next = NULL;       
    }
    return head;
}


List List_get( List head, Coord coord ) {
    if (head == NULL)
        return NULL;
    for ( List actual = head ; actual != NULL ; actual = actual->next ){

        if( coord_equal( actual->coord , coord ) ){
            return actual;
        }
    }
    return NULL;
}


List List_remove( List head, Coord coord , void (*delete_data) (void*) ) {
    // first case
    if (head == NULL)
        return NULL;
    
    if( coord_equal( head->coord , coord ) ) {
        List aux_head = head->next;
        delete_data(head->data);
        free(head);
        return aux_head;
    }

    for ( List actual = head , son = head->next ; son != NULL ; actual = son , son = son->next )
        if( coord_equal( son->coord , coord ) ) {
            actual->next = son->next;
            delete_data(son->data);
            free(son);
            return head;
        }
    return NULL;
}


void List_delete(List head , void (*delete_data) (void*) ) {
    // first case
    if( head == NULL)
        return;
    List actual = head;
    for ( List son = head->next ; son != NULL ; actual = son , son = son->next ) {
            delete_data(actual->data);
            free(actual);
        }
    // delete Last one
    delete_data(actual->data);
    free(actual);
    return;
}

