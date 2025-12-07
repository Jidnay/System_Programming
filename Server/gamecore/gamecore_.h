#ifndef _GAMECORE_INCLUDED
#define _GAMECORE_INCLUDED

#include <stdbool.h>
#include "../gamecore.h"

/*************************************************************************** 
 ***************************************************************************
 *                      Other Auxiliary functions
 *                          (utils.c)
 ***************************************************************************
****************************************************************************/

bool coord_equal( Coord c1 , Coord c2 );
Coord valid_move ( Coord coord , int way );
bool isFree( Game game , Coord coord );
int random_id(Game game);
int Prize_to_ASCII(int prize);

#endif

