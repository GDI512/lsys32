#ifndef WINUTIL_H
#define WINUTIL_H

#include <Windows.h>

void GetCenterAlignedFramePoint(int* x, int* y, int cxTarget, int cyTarget, int cxFrame, int cyFrame);

SIZE GetSizeForClient(int cx, int cy, DWORD dwStyle, BOOL fMenu);

#endif
