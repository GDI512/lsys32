#ifndef SYSTEM_H
#define SYSTEM_H

#include "GdiTurtle.h"

#define KOCH_CURVE            0
#define DRAGON_CURVE          1
#define SIERPINSKI_TRIANGLE   2
#define SIERPINSKI_ARROWHEAD  3
#define FRACTAL_PLANT         4

extern HSYSTEM SYSTEM_PRESET[];

void InitSystemPresets();

#endif
