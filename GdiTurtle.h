#ifndef GDITURTLE_H
#define GDITURTLE_H

#include <Windows.h>

typedef void* HSYSTEM;

HSYSTEM CreateSystem(int nIter, double dStep, double dRoll, PCWSTR szAxiom, int nRules, ...);

void DeleteSystem(HSYSTEM hSystem);

void GetBoundingBoxMetrics(HSYSTEM hSystem, LONG* cxBox, LONG* cyBox, LONG* xInit, LONG* yInit);

void SetStep(HSYSTEM hSystem, double dStep);

void SetAngle(HSYSTEM hSystem, double dRoll);

typedef void* HTURTLE;

HTURTLE CreateTurtle();

void DeleteTurtle(HTURTLE hTurtle);

void SetSystem(HTURTLE hTurtle, HSYSTEM hSystem);

void SetDC(HTURTLE hTurtle, HDC hdc);

void MoveTurtle(HTURTLE hTurtle, int xPos, int yPos);

void RotateTurtle(HTURTLE hTurtle, double nAngle);

BOOL TurtleStep(HTURTLE hTurtle);

void TurtleDraw(HTURTLE hTurtle);

#endif
