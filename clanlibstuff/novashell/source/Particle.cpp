
/*
Object: Particle
Information about a certain particle type that is shared between effects.

Group: Member Functions

func: SetColor
(code)
nil SetColor(Color color)
(end)
Sets the initial color of the particle.

Usage:
(code)
particle:SetColor(Color(255,110,60,255));

(end)


func: SetColoring1
(code)
nil SetColoring1(Color color1, number startTimeMS)
(end)
Particle will fade to this color.

Parameters:

color1 - A <Color> object holding the color we want to change to interpolate to.
startTimeMS - The fade will start at this time, 0 to start changing right away.


func: SetColoring2
(code)
nil SetColoring2(Color color1, Color color2, number startTimeMS)
(end)
Like <Particle::SetColoring1> except it allows you to fade to three colors total.

Parameters:

color1 - A <Color> object holding the color we want to change to interpolate to.
color2 - The second <Color> object holding the color we want to change to interpolate to.
startTimeMS - The fade to color2 will start at this time, 0 to start changing right away.

func: GetMotionController
(code)
MotionController GetMotionController()
(end)

Returns:

The <MotionController> associated with this particle template.  (created if one doesn't exist)






*/
