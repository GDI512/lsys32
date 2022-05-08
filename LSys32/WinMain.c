#include "resource.h"
#include <windowsx.h>
#include "GdiTurtle.h"
#include "GdiBox.h"
#include "System.h"
#include "WinUtil.h"

static HWND hBox;

INT_PTR OnInitDialog(HWND hDlg) {
    HWND hCtl = GetDlgItem(hDlg, IDC_COMBO);
    ComboBox_AddString(hCtl, L"Koch Curve");
    ComboBox_AddString(hCtl, L"Dragon Curve");
    ComboBox_AddString(hCtl, L"Sierpinski Triangle");
    ComboBox_AddString(hCtl, L"Sierpinski Arrowhead");
    ComboBox_AddString(hCtl, L"Fractal Plant");
    ComboBox_SetCurSel(hCtl, 0);
    SendMessage(hBox, GBM_SETSYSID, KOCH_CURVE, 0);
    return TRUE;
}

INT_PTR OnClose(HWND hDlg) {
    DestroyWindow(hDlg);
    return TRUE;
}

INT_PTR OnDestroy() {
    DestroyWindow(hBox);
    PostQuitMessage(0);
    return TRUE;
}

INT_PTR OnCommand(HWND hDlg, WPARAM wParam) {
    HWND hCtl;
    LONG iSys;
    switch (LOWORD(wParam)) {
    case IDC_DRAW:
        hCtl = GetDlgItem(hDlg, IDC_COMBO);
        iSys = ComboBox_GetCurSel(hCtl);
        SendMessage(hBox, GBM_SETSYSID, iSys, 0);
        SendMessage(hBox, GBM_DRAW, 0, 0);
        break;
    case IDC_ANIMATE:
        hCtl = GetDlgItem(hDlg, IDC_COMBO);
        iSys = ComboBox_GetCurSel(hCtl);
        SendMessage(hBox, GBM_SETSYSID, iSys, 0);
        SendMessage(hBox, GBM_ANIMATE, 0, 0);
        break;
    }
    return TRUE;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        return OnInitDialog(hDlg);
    case WM_CLOSE:
        return OnClose(hDlg);
    case WM_DESTROY:
        return OnDestroy();
    case WM_COMMAND:
        return OnCommand(hDlg, wParam);
    case WM_CTLCOLORDLG:
        return GetStockObject(WHITE_BRUSH);
    default:
        return FALSE;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
    HWND hDlg;
    MSG  msg;
    SIZE size;

    if (!RegisterBox(hInstance)) {
        MessageBox(NULL, L"Failed to register the window class", L"Error", MB_ICONERROR);
        return 1;
    }

    InitSystemPresets();

    size = GetSizeForClient(768, 480, WS_OVERLAPPEDWINDOW, FALSE);
    hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgProc);
    hBox = CreateWindow(GDIBOX_CLASS, L"GDI Box", WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, size.cx, size.cy, NULL, NULL, hInstance, NULL);
    ShowWindow(hBox, iCmdShow);
    ShowWindow(hDlg, iCmdShow);
    UpdateWindow(hBox);

    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!IsDialogMessage (hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return msg.wParam;
}