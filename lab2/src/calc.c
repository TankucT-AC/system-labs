#include "calc.h"
#include <math.h>

int add(int a, int b) {
    return a + b;
}

int mul(int a, int b) {
    return a * b;
}

int div(int a, int b) {
    if (b == 0) return NAN;
    return a / b;
}

int sub(int a, int b) {
    return a - b;
}

int op(int a, int b, int calc_func) {
    switch (calc_func) {
    case ADD: return add(a, b);
    case MUL: return mul(a, b);
    case DIV: return div(a, b);
    case SUB: return sub(a, b);
    default: return 0;
    }
}

