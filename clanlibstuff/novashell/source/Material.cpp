#include "AppPrecomp.h"

/*
Object: DrawManager
For drawing primitives like rectangles and lines.

About:

Being able to draw a rectangle, line, or single pixel from any place is useful for debugging or things like fading the screen out.

You can specify which layer to draw over and whether or not to use screen or world coordinates.


Usage:

(code)
//draw a black line on the screen, on top of everything else
GetDrawManager:DrawLine(Vector2(0,0), Vector2(100,100), Color(0,0,0,255), C_LAYER_TOP, C_DRAW_SCREEN_COORDS);
(end)


Group: Member Functions
*/

/*
func: DrawLine
(code)
nil (Vector2 a, Vector2 b, Color col, number layerID, number drawType) 
(end)
Draws a line of the specified color at the specified layerID.

Parameters:

a - A <Vector2> object containing the line's starting position
b - A <Vector2> object containing the line's ending position
col - A <Color> object that specifies the line's color and opacity
layerID - Onto which layer we should draw to, use C_LAYER_TOP for on top of everything
drawType - One of the <C_DRAW_TYPE_CONSTANTS>.


Section: Related Constants

Group: C_DRAW_TYPE_CONSTANTS
Used with the <DrawManager>

constant: C_DRAW_SCREEN_COORDS
The numbers passed in are in screen coordinates, so 0,0 would always mean the upper-left most dot on the screen.

constant: C_DRAW_WORLD_COORDS
The numbers passed in are in world coordinates, drawing a dot at an entities position would draw it over wherever the entity is, possibly offscreen.

*/
