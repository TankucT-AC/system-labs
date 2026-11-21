// STL
#include <stdio.h>

// calculator
#include <src/calc.h>

int main(void) {
    int a, b, operator;
    if (!scanf("%d %d %d", &a, &b, &operator)) {
        printf("error: uncorrect input!!!\n");
        return 1;
    }
    if (operator < 0 || operator > 4) {
        printf("error: uncorrect operator!!!\n");
        return 1;
    }
    if (operator == DIV && b == 0) {
        printf("error: division by zero!!!\n");
        return 1;
    }
    int result = op(a, b, operator);
    printf("Result: %d\n", result);

    return 0;
}
