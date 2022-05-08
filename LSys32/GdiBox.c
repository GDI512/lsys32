#include "GdiBox.h"
#include "GdiTurtle.h"
#include "System.h"
#include "WinUtil.h"

#define IDT_ANIMATION  1

#define TIMER_STEP     25

#define BWLP_HDC       0
#define BWLP_HBITMAP   BWLP_HDC      + sizeof(HDC)
#define BWLP_HTURTLE   BWLP_HBITMAP  + sizeof(HBITMAP)
#define BWLP_SYSID     BWLP_HTURTLE  + sizeof(HTURTLE)
#define BWLP_CXCLIENT  BWLP_SYSID    + sizeof(LONG_PTR)
#define BWLP_CYCLIENT  BWLP_CXCLIENT + sizeof(LONG_PTR)
#define BWLP_CXBITMAP  BWLP_CYCLIENT + sizeof(LONG_PTR)
#define BWLP_CYBITMAP  BWLP_CXBITMAP + sizeof(LONG_PTR)
#define BWLP_WNDEXTRA  BWLP_CYBITMAP + sizeof(LONG_PTR)

#define Box_GetMemoryDC(H)         (HDC)GetWindowLongPtr(H, BWLP_HDC)
#define Box_GetBitmap(H)           (HBITMAP)GetWindowLongPtr(H, BWLP_HBITMAP)
#define Box_GetTurtle(H)           (HTURTLE)GetWindowLongPtr(H, BWLP_HTURTLE)
#define Box_GetSystemID(H)         (LONG)GetWindowLongPtr(H, BWLP_SYSID)
#define Box_GetClientWidth(H)      (LONG)GetWindowLongPtr(H, BWLP_CXCLIENT)
#define Box_GetClientHeight(H)     (LONG)GetWindowLongPtr(H, BWLP_CYCLIENT)
#define Box_GetBitmapWidth(H)      (LONG)GetWindowLongPtr(H, BWLP_CXBITMAP)
#define Box_GetBitmapHeight(H)     (LONG)GetWindowLongPtr(H, BWLP_CYBITMAP)

#define Box_SetMemoryDC(H, X)      (HDC)SetWindowLongPtr(H, BWLP_HDC, X)
#define Box_SetBitmap(H, X)        (HBITMAP)SetWindowLongPtr(H, BWLP_HBITMAP, X)
#define Box_SetTurtle(H, X)        (HTURTLE)SetWindowLongPtr(H, BWLP_HTURTLE, X)
#define Box_SetSystemID(H, X)      (LONG)SetWindowLongPtr(H, BWLP_SYSID, X)
#define Box_SetClientWidth(H, X)   (LONG)SetWindowLongPtr(H, BWLP_CXCLIENT, X)
#define Box_SetClientHeight(H, X)  (LONG)SetWindowLongPtr(H, BWLP_CYCLIENT, X)
#define Box_SetBitmapWidth(H, X)   (LONG)SetWindowLongPtr(H, BWLP_CXBITMAP, X)
#define Box_SetBitmapHeight(H, X)  (LONG)SetWindowLongPtr(H, BWLP_CYBITMAP, X)

static HBITMAP CreateResizedBitmap(HWND hwnd, HBITMAP hOldBmp, LONG cxNew, LONG cyNew, LONG* cxOld, LONG* cyOld) {
    HDC     hWinDC;
    HBITMAP hBitmap;
    DeleteObject(hOldBmp);
    hWinDC  = GetDC(hwnd);
    hBitmap = CreateCompatibleBitmap(hWinDC, cxNew, cyNew);
    *cxOld  = cxNew;
    *cyOld  = cyNew;
    ReleaseDC(hwnd, hWinDC);
    return hBitmap;
}

static LRESULT Box_OnCreate(HWND hwnd) {
    HDC hWinDC = GetDC(hwnd);
    Box_SetMemoryDC(hwnd, CreateCompatibleDC(hWinDC));
    Box_SetTurtle(hwnd, CreateTurtle());
    ReleaseDC(hwnd, hWinDC);
    return 0;
}

static LRESULT Box_OnDestroy(HWND hwnd) {
    DeleteTurtle(Box_GetTurtle(hwnd));
    DeleteObject(Box_GetBitmap(hwnd));
    DeleteDC(Box_GetMemoryDC(hwnd));
    return 0;
}

static LRESULT Box_OnSize(HWND hwnd, LPARAM lParam) {
    Box_SetClientWidth(hwnd, LOWORD(lParam));
    Box_SetClientHeight(hwnd, HIWORD(lParam));
    return 0;
}

static LRESULT Box_OnPaint(HWND hwnd) {
    HDC         hMemDC   = Box_GetMemoryDC(hwnd);
    HBITMAP     hBitmap  = Box_GetBitmap(hwnd);
    LONG        cxClient = Box_GetClientWidth(hwnd);
    LONG        cyClient = Box_GetClientHeight(hwnd);
    LONG        cxBmp    = Box_GetBitmapWidth(hwnd);
    LONG        cyBmp    = Box_GetBitmapHeight(hwnd);
    PAINTSTRUCT paint;
    HDC         hWinDC;
    HGDIOBJ     hOldBmp;
    LONG        xDraw, yDraw;

    hWinDC = BeginPaint(hwnd, &paint);
    hOldBmp = SelectObject(hMemDC, hBitmap);

    GetCenterAlignedFramePoint(&xDraw, &yDraw, cxClient, cyClient, cxBmp, cyBmp);
    BitBlt(hWinDC, xDraw, yDraw, cxBmp, cyBmp, hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hOldBmp);
    EndPaint(hwnd, &paint);
    return 0;
}

static LRESULT Box_OnTimer(HWND hwnd) {
    HDC     hMemDC   = Box_GetMemoryDC(hwnd);
    HBITMAP hBitmap  = Box_GetBitmap(hwnd);
    HTURTLE hTurtle  = Box_GetTurtle(hwnd);
    LONG    cxClient = Box_GetClientWidth(hwnd);
    LONG    cyClient = Box_GetClientHeight(hwnd);
    LONG    cxBmp    = Box_GetBitmapWidth(hwnd);
    LONG    cyBmp    = Box_GetBitmapHeight(hwnd);
    HDC     hWinDC;
    HGDIOBJ hOldBmp;
    LONG    xWnd, yWnd;
    
    hWinDC  = GetDC(hwnd);
    hOldBmp = SelectObject(hMemDC, hBitmap);

    GetCenterAlignedFramePoint(&xWnd, &yWnd, cxClient, cyClient, cxBmp, cyBmp);
    if (!TurtleStep(hTurtle)) {
        KillTimer(hwnd, IDT_ANIMATION);
    }
    BitBlt(hWinDC, xWnd, yWnd, cxBmp, cyBmp, hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hOldBmp);
    ReleaseDC(hwnd, hWinDC);
    return 0;
}

static LRESULT Box_OnDraw(HWND hwnd) {
    HDC     hMemDC   = Box_GetMemoryDC(hwnd);
    HBITMAP hBitmap  = Box_GetBitmap(hwnd);
    HTURTLE hTurtle  = Box_GetTurtle(hwnd);
    LONG    iSysID   = Box_GetSystemID(hwnd);
    LONG    cxClient = Box_GetClientWidth(hwnd);
    LONG    cyClient = Box_GetClientHeight(hwnd);
    LONG    cxBmp    = Box_GetBitmapWidth(hwnd);
    LONG    cyBmp    = Box_GetBitmapHeight(hwnd);
    HDC     hWinDC;
    HBITMAP hOldBmp;
    LONG    xPos, yPos;
    LONG    xWnd, yWnd;
    LONG    cxBox, cyBox;

    KillTimer(hwnd, IDT_ANIMATION);
    GetBoundingBoxMetrics(SYSTEM_PRESET[iSysID], &cxBox, &cyBox, &xPos, &yPos);

    if (cxBmp < cxBox || cyBmp < cyBox) {
        hBitmap = CreateResizedBitmap(hwnd, hBitmap, cxBox, cyBox, &cxBmp, &cyBmp);
        Box_SetBitmap(hwnd, hBitmap);
        Box_SetBitmapWidth(hwnd, cxBox);
        Box_SetBitmapHeight(hwnd, cyBox);
    }

    hWinDC  = GetDC(hwnd);
    hOldBmp = SelectObject(hMemDC, hBitmap);

    GetCenterAlignedFramePoint(&xWnd, &yWnd, cxClient, cyClient, cxBmp, cyBmp);
    SetSystem(hTurtle, SYSTEM_PRESET[iSysID]);
    SetDC(hTurtle, hMemDC);
    MoveTurtle(hTurtle, xPos, yPos);
    BitBlt(hMemDC, 0, 0, cxBmp, cyBmp, hMemDC, 0, 0, WHITENESS);
    TurtleDraw(hTurtle);
    BitBlt(hWinDC, 0, 0, cxClient, cxClient, hWinDC, 0, 0, WHITENESS);
    BitBlt(hWinDC, xWnd, yWnd, cxBmp, cyBmp, hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hOldBmp);
    ReleaseDC(hwnd, hWinDC);
    return 0;
}

static LRESULT Box_OnAnimate(HWND hwnd) {
    HDC     hMemDC   = Box_GetMemoryDC(hwnd);
    HBITMAP hBitmap  = Box_GetBitmap(hwnd);
    HTURTLE hTurtle  = Box_GetTurtle(hwnd);
    LONG    iSysID   = Box_GetSystemID(hwnd);
    LONG    cxClient = Box_GetClientWidth(hwnd);
    LONG    cyClient = Box_GetClientHeight(hwnd);
    LONG    cxBmp    = Box_GetBitmapWidth(hwnd);
    LONG    cyBmp    = Box_GetBitmapHeight(hwnd);
    HDC     hWinDC;
    HGDIOBJ hOldBmp;
    LONG    cxBox, cyBox;
    LONG    xPos, yPos;

    KillTimer(hwnd, IDT_ANIMATION);
    GetBoundingBoxMetrics(SYSTEM_PRESET[iSysID], &cxBox, &cyBox, &xPos, &yPos);

    if (cxBmp < cxBox || cyBmp < cyBox) {
        hBitmap = CreateResizedBitmap(hwnd, hBitmap, cxBox, cyBox, &cxBmp, &cyBmp);
        Box_SetBitmap(hwnd, hBitmap);
        Box_SetBitmapWidth(hwnd, cxBox);
        Box_SetBitmapHeight(hwnd, cyBox);
    }
    
    hWinDC  = GetDC(hwnd);
    hOldBmp = SelectObject(hMemDC, hBitmap);
    BitBlt(hWinDC, 0, 0, cxClient, cyClient, hWinDC, 0, 0, WHITENESS);
    BitBlt(hMemDC, 0, 0, cxBmp, cyBmp, hMemDC, 0, 0, WHITENESS);
    SelectObject(hMemDC, hOldBmp);
    ReleaseDC(hwnd, hWinDC);

    SetSystem(hTurtle, SYSTEM_PRESET[iSysID]);
    SetDC(hTurtle, hMemDC);
    MoveTurtle(hTurtle, xPos, yPos);
    SetTimer(hwnd, IDT_ANIMATION, TIMER_STEP, NULL);
    return 0;
}

static LRESULT Box_OnSetSysID(HWND hwnd, WPARAM wParam) {
    Box_SetSystemID(hwnd, wParam);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        return Box_OnCreate(hwnd);
    case WM_DESTROY:
        return Box_OnDestroy(hwnd);
    case WM_SIZE:
        return Box_OnSize(hwnd, lParam);
    case WM_PAINT:
        return Box_OnPaint(hwnd);
    case WM_TIMER:
        return Box_OnTimer(hwnd);
    case GBM_DRAW:
        return Box_OnDraw(hwnd);
    case GBM_ANIMATE:
        return Box_OnAnimate(hwnd);
    case GBM_SETSYSID:
        return Box_OnSetSysID(hwnd, wParam);
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

ATOM RegisterBox(HINSTANCE hInstance) {
    WNDCLASS wndclass = {
        .style         = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc   = WndProc,
        .cbClsExtra    = 0,
        .cbWndExtra    = BWLP_WNDEXTRA,
        .hInstance     = hInstance,
        .hIcon         = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor       = LoadCursor(NULL, IDC_CROSS),
        .hbrBackground = GetStockObject(WHITE_BRUSH),
        .lpszMenuName  = NULL,
        .lpszClassName = GDIBOX_CLASS
    };
    return RegisterClass(&wndclass);
}