#include "GdiTurtle.h"

#define MAX_STACK     64
#define MIN_CAPACITY  8

#include <Windows.h>
#include <stdarg.h>
#include <math.h>

typedef struct {
    PWSTR psCmd;
    int   ccCmd;
} CMD;

typedef CMD RULE;

typedef struct {
    CMD*   arrCmd;
    RULE*  arrRules;
    double dStep;
    double dRoll;
    int    iNextGen;
    int    iCmdArrCap;
} SYSTEM;

typedef struct {
    double dX;
    double dY;
    double dRoll;
} TURTLEPOS;

typedef struct {
    HDC           hdc;
    const SYSTEM* pSystem;
    int           iCmd;
    TURTLEPOS     pos;
    int           iSaved;
    TURTLEPOS*    arrSaved;
} TURTLE;

static BOOL IsRuleChar(WCHAR c) {
    return c >= L'A' && c <= L'Z';
}

static void StrCpy(PWSTR psDest, int ccSrc, PCWSTR psSrc) {
    while (ccSrc --> 0) {
        *psDest++ = *psSrc++;
    }
}

static CMD CreateCommand(PCWSTR szAxiom) {
    CMD cmd;
    cmd.ccCmd = wcslen(szAxiom);
    cmd.psCmd = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, cmd.ccCmd * 2);
    StrCpy(cmd.psCmd, cmd.ccCmd, szAxiom);
    return cmd;
}

static void DeleteCommand(CMD cmd) {
    HeapFree(GetProcessHeap(), 0, cmd.psCmd);
}

static CMD CreateIteratedCommand(RULE* arrRules, CMD cmdOld) {
    CMD  cmdNew;
    RULE rule;

    cmdNew.psCmd = NULL;
    cmdNew.ccCmd = 0;

    for (int iSym = 0; iSym < cmdOld.ccCmd; iSym++) {
        if (IsRuleChar(cmdOld.psCmd[iSym])) {
            rule = arrRules[cmdOld.psCmd[iSym] - L'A'];
            cmdNew.ccCmd += rule.ccCmd;
        } else {
            cmdNew.ccCmd += 1;
        }
    }

    cmdNew.psCmd = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, cmdNew.ccCmd * 2);

    for (int iSym = 0, iOff = 0; iSym < cmdOld.ccCmd; iSym++) {
        if (IsRuleChar(cmdOld.psCmd[iSym])) {
            rule = arrRules[cmdOld.psCmd[iSym] - L'A'];
            StrCpy(cmdNew.psCmd + iOff, rule.ccCmd, rule.psCmd);
            iOff += rule.ccCmd;
        } else {
            cmdNew.psCmd[iOff] = cmdOld.psCmd[iSym];
            iOff += 1;
        }
    }

    return cmdNew;
}

void IterateSystem(SYSTEM* pSystem, int iGen) {
    if (pSystem->iNextGen <= iGen) {
        if (pSystem->iCmdArrCap <= iGen) {
            pSystem->iCmdArrCap = pSystem->iCmdArrCap * 2 > iGen + 1 ? pSystem->iCmdArrCap * 2 : iGen + 1;
            pSystem->arrCmd = HeapReAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, pSystem->arrCmd, pSystem->iCmdArrCap * sizeof(CMD));
        }
        while (pSystem->iNextGen <= iGen) {
            pSystem->arrCmd[pSystem->iNextGen] = CreateIteratedCommand(pSystem->arrRules, pSystem->arrCmd[pSystem->iNextGen - 1]);
            pSystem->iNextGen++;
        }
    }
}

HSYSTEM CreateSystem(int nIter, double dStep, double dRoll, PCWSTR szAxiom, int nRules, ...) {
    SYSTEM* pSystem;
    va_list pArg;

    pSystem             = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(SYSTEM));
    pSystem->arrCmd     = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, MIN_CAPACITY * sizeof(CMD));
    pSystem->arrCmd[0]  = CreateCommand(szAxiom);
    pSystem->arrRules   = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, sizeof(RULE[26]));
    pSystem->dStep      = dStep;
    pSystem->dRoll      = dRoll;
    pSystem->iNextGen   = 1;
    pSystem->iCmdArrCap = MIN_CAPACITY;

    va_start(pArg, nRules);
    for (int iRule = 0; iRule < nRules; iRule++) {
        WCHAR  cPred = va_arg(pArg, WCHAR);
        PCWSTR szCmd = va_arg(pArg, PCWSTR);
        pSystem->arrRules[cPred - L'A'] = CreateCommand(szCmd);
    }
    va_end(pArg);

    IterateSystem(pSystem, nIter);

    return pSystem;
}

void DeleteSystem(HSYSTEM hSystem) {
    SYSTEM* pSystem = hSystem;
    for (int iCmd = 0; iCmd < 26; iCmd++) {
        DeleteCommand(pSystem->arrRules[iCmd]);
    }
    for (int iCmd = 0; iCmd < pSystem->iNextGen; iCmd++) {
        DeleteCommand(pSystem->arrCmd[iCmd]);
    }
    HeapFree(GetProcessHeap(), 0, pSystem->arrRules);
    HeapFree(GetProcessHeap(), 0, pSystem->arrCmd);
    HeapFree(GetProcessHeap(), 0, pSystem);
}

void GetBoundingBoxMetrics(HSYSTEM hSystem, LONG* cxBox, LONG* cyBox, LONG* xInit, LONG* yInit) {
    SYSTEM*    pSystem = hSystem;
    int        iSaved;
    TURTLEPOS  arrSaved[MAX_STACK];
    TURTLEPOS  pos;
    CMD        currCmd;
    double     dXMin, dYMin, dXMax, dYMax; 

    iSaved    = 0;
    pos.dX    = 0.0;
    pos.dY    = 0.0;
    pos.dRoll = 0.0;

    currCmd = pSystem->arrCmd[pSystem->iNextGen - 1];
    dXMin   = 0.0;
    dYMin   = 0.0;
    dXMax   = 0.0;
    dYMax   = 0.0;

    for (int iSym = 0; iSym < currCmd.ccCmd; iSym++) {
        switch (currCmd.psCmd[iSym]) {
        case '+':
            pos.dRoll -= pSystem->dRoll;
            break;
        case '-':
            pos.dRoll += pSystem->dRoll;
            break;
        case '[':
            arrSaved[iSaved].dX    = pos.dX;
            arrSaved[iSaved].dY    = pos.dY;
            arrSaved[iSaved].dRoll = pos.dRoll;
            iSaved++;
            break;
        case ']':
            iSaved--;
            pos.dX    = arrSaved[iSaved].dX;
            pos.dY    = arrSaved[iSaved].dY;
            pos.dRoll = arrSaved[iSaved].dRoll;
            break;
        default:
            pos.dX += pSystem->dStep * cos(pos.dRoll);
            pos.dY += pSystem->dStep * sin(pos.dRoll);
            if (pos.dX > dXMax) {
                dXMax = pos.dX;
            } else if (pos.dX < dXMin) {
                dXMin = pos.dX;
            }
            if (pos.dY > dYMax) {
                dYMax = pos.dY;
            } else if (pos.dY < dYMin) {
                dYMin = pos.dY;
            }
            break;
        }
    }

    *cxBox = ceil(dXMax - dXMin + 1.0);
    *cyBox = ceil(dYMax - dYMin + 1.0);
    *xInit = -dXMin;
    *yInit = -dYMin;
}

void SetStep(HSYSTEM hSystem, double dStep) {
    SYSTEM* pSystem = hSystem;
    pSystem->dStep  = dStep;   
}

void SetAngle(HSYSTEM hSystem, double dRoll) {
    SYSTEM* pSystem = hSystem;
    pSystem->dRoll  = dRoll;
}

HTURTLE CreateTurtle() {
    TURTLE* pTurtle = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(TURTLE));
    pTurtle->hdc       = NULL;
    pTurtle->pSystem   = NULL;
    pTurtle->iCmd      = 0;
    pTurtle->pos.dX    = 0.0;
    pTurtle->pos.dY    = 0.0;
    pTurtle->pos.dRoll = 0.0;
    pTurtle->iSaved    = 0;
    pTurtle->arrSaved  = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, MAX_STACK * sizeof(TURTLEPOS));
    return pTurtle;
}

void DeleteTurtle(HTURTLE hTurtle) {
    TURTLE* pTurtle = hTurtle;
    HeapFree(GetProcessHeap(), 0, pTurtle->arrSaved);
    HeapFree(GetProcessHeap(), 0, pTurtle);
}

void SetSystem(HTURTLE hTurtle, HSYSTEM hSystem) {
    TURTLE* pTurtle  = hTurtle;
    pTurtle->pSystem = hSystem;
    pTurtle->iCmd    = 0;
}

void SetDC(HTURTLE hTurtle, HDC hdc) {
    TURTLE* pTurtle    = hTurtle;
    pTurtle->hdc       = hdc;
    pTurtle->pos.dX    = 0.0;
    pTurtle->pos.dY    = 0.0;
    pTurtle->pos.dRoll = 0.0;
}

void MoveTurtle(HTURTLE hTurtle, int dX, int dY) {
    TURTLE* pTurtle = hTurtle;
    pTurtle->pos.dX = dX;
    pTurtle->pos.dY = dY;
    MoveToEx(pTurtle->hdc, dX, dY, NULL);
}

void RotateTurtle(HTURTLE hTurtle, double dRoll) {
    TURTLE* pTurtle = hTurtle;
    pTurtle->pos.dRoll = dRoll;
}

void SaveTurtle(HTURTLE hTurtle) {
    TURTLE* pTurtle = hTurtle;
    if (pTurtle->iSaved < MAX_STACK) {
        pTurtle->arrSaved[pTurtle->iSaved].dX    = pTurtle->pos.dX;
        pTurtle->arrSaved[pTurtle->iSaved].dY    = pTurtle->pos.dY;
        pTurtle->arrSaved[pTurtle->iSaved].dRoll = pTurtle->pos.dRoll;
        pTurtle->iSaved++;
    }
}

void RestoreTurtle(HTURTLE hTurtle) {
    TURTLE* pTurtle = hTurtle;
    if (pTurtle->iSaved > 0) {
        pTurtle->iSaved--;
        pTurtle->pos.dX    = pTurtle->arrSaved[pTurtle->iSaved].dX;
        pTurtle->pos.dY    = pTurtle->arrSaved[pTurtle->iSaved].dY;
        pTurtle->pos.dRoll = pTurtle->arrSaved[pTurtle->iSaved].dRoll;
        MoveToEx(pTurtle->hdc, pTurtle->pos.dX, pTurtle->pos.dY, NULL);
    }
}

void TurtleLine(HTURTLE hTurtle) {
    TURTLE* pTurtle = hTurtle;
    pTurtle->pos.dX += pTurtle->pSystem->dStep * cos(pTurtle->pos.dRoll);
    pTurtle->pos.dY += pTurtle->pSystem->dStep * sin(pTurtle->pos.dRoll);
    LineTo(pTurtle->hdc, ceil(pTurtle->pos.dX), ceil(pTurtle->pos.dY));
}

BOOL TurtleStep(HTURTLE hTurtle) {
    TURTLE* pTurtle = hTurtle;
    CMD cmd = pTurtle->pSystem->arrCmd[pTurtle->pSystem->iNextGen - 1];
    while (pTurtle->iCmd < cmd.ccCmd && !IsRuleChar(cmd.psCmd[pTurtle->iCmd])) {
        switch (cmd.psCmd[pTurtle->iCmd++]) {
        case '+':
            pTurtle->pos.dRoll -= pTurtle->pSystem->dRoll;
            break;
        case '-':
            pTurtle->pos.dRoll += pTurtle->pSystem->dRoll;
            break;
        case '[':
            SaveTurtle(hTurtle);
            break;
        case ']':
            RestoreTurtle(hTurtle);
            break;
        }
    }

    if (pTurtle->iCmd < cmd.ccCmd) {
        switch (cmd.psCmd[pTurtle->iCmd++]) {
        case '+':
            pTurtle->pos.dRoll -= pTurtle->pSystem->dRoll;
            break;
        case '-':
            pTurtle->pos.dRoll += pTurtle->pSystem->dRoll;
            break;
        case '[':
            SaveTurtle(hTurtle);
            break;
        case ']':
            RestoreTurtle(hTurtle);
            break;
        default:
            TurtleLine(hTurtle);
            break;
        }
        return TRUE;
    }
    return FALSE;
}

void TurtleDraw(HTURTLE hTurtle) {
    TURTLE* pTurtle = hTurtle;
    CMD cmd = pTurtle->pSystem->arrCmd[pTurtle->pSystem->iNextGen - 1];
    while (pTurtle->iCmd < cmd.ccCmd) {
        switch (cmd.psCmd[pTurtle->iCmd]) {
        case '+':
            pTurtle->pos.dRoll -= pTurtle->pSystem->dRoll;
            break;
        case '-':
            pTurtle->pos.dRoll += pTurtle->pSystem->dRoll;
            break;
        case '[':
            SaveTurtle(hTurtle);
            break;
        case ']':
            RestoreTurtle(hTurtle);
            break;
        default:
            TurtleLine(hTurtle);
            break;
        }
        pTurtle->iCmd++;
    }
}