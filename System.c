#include "System.h"

#define RAD(X) (X * 3.14159265359) / 180.0

HSYSTEM SYSTEM_PRESET[5];

void InitSystemPresets() {
    SYSTEM_PRESET[KOCH_CURVE] = CreateSystem(4, 7.67, RAD(90.0), L"F", 1, L'F', L"F+F-F-F+F");
    SYSTEM_PRESET[DRAGON_CURVE] = CreateSystem(13, 3.67, RAD(90.0), L"F", 2, L'F', L"F+G", L'G', L"F-G");
    SYSTEM_PRESET[SIERPINSKI_TRIANGLE] = CreateSystem(6, 8.25, RAD(120.0), L"F-G-G", 2, L'F', L"F-G+F+G-F", L'G', L"GG");
    SYSTEM_PRESET[SIERPINSKI_ARROWHEAD] = CreateSystem(6, 7.67, RAD(60.0), L"A", 2, L'A', L"B-A-B", L'B', L"A+B+A");
    SYSTEM_PRESET[FRACTAL_PLANT] = CreateSystem(6, 3.85, RAD(25.0), L"++X", 2, L'X', L"F+[[X]-X]-F[-FX]+X", L'F', L"FF");
}