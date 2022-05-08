#include "WinUtil.h"

void GetCenterAlignedFramePoint(int* x, int* y, int cxTarget, int cyTarget, int cxFrame, int cyFrame) {
    *x = (cxTarget - cxFrame) / 2;
    *y = (cyTarget - cyFrame) / 2;
}

SIZE GetSizeForClient(int cx, int cy, DWORD dwStyle, BOOL fMenu) {
    RECT rect = { 0, 0, cx, cy };
    SIZE size;
    AdjustWindowRect(&rect, dwStyle, fMenu);
    size.cx = rect.right - rect.left;
    size.cy = rect.bottom - rect.top;
    return size;
}