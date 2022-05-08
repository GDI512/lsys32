#ifndef GDIBOX_H
#define GDIBOX_H

#include <Windows.h>

#define GDIBOX_CLASS  L"GDI Box"
#define GBM_DRAW      WM_USER + 0x0000
#define GBM_ANIMATE   WM_USER + 0x0001
#define GBM_SETSYSID  WM_USER + 0x0002

ATOM RegisterBox(HINSTANCE hInstance);

#endif
