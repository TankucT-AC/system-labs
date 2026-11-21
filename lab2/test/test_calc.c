#include <src/calc.h>
#include <stdio.h>
#include <math.h>

static int tests_run = 0;
static int tests_failed = 0;


#define ASSERT_EQ(expected, actual) do { \
    tests_run++; \
    int _exp = (expected); \
    int _act = (actual); \
    if (_exp != _act) { \
        fprintf(stderr, "[FAIL] %s:%d (%s): %s != %s (ожидалось %d, получено %d)\n", \
                __FILE__, __LINE__, __func__, #expected, #actual, _exp, _act); \
        tests_failed++; \
    } \
} while (0)

#define RUN_TEST(fn) do { \
    int _before = tests_failed; \
    fn(); \
    if (tests_failed == _before) { \
        printf("[ OK ] %s\n", #fn); \
    } \
} while (0)
/* ---------------------------------- */

void test_add(void) {
    ASSERT_EQ(5, op(2, 3, ADD)); 
    ASSERT_EQ(-5, op(-2, -3, ADD));
    ASSERT_EQ(2, op(2, 0, ADD));
}

void test_mul(void) {
    ASSERT_EQ(500, op(5, 100, MUL));
    ASSERT_EQ(-67, op(-1, 67, MUL));
    ASSERT_EQ(0, op(10000, 0, MUL));
    ASSERT_EQ(2500000, op(50000, 50000, MUL)); // Переполнение int, тест упадет
}

void test_sub(void) {
    ASSERT_EQ(-11, op(1, 12, SUB));
    ASSERT_EQ(21, op(-10, -31, SUB));
    ASSERT_EQ(0, op(10, 10, SUB));
}

void test_div(void) {
    ASSERT_EQ(2, op(13, 5, DIV));
    ASSERT_EQ(10, op(-100, -10, DIV));
    ASSERT_EQ(0, op(0, 12, DIV));
    ASSERT_EQ(NAN, op(127, 0, DIV)); // Деление на ноль вернет NAN
}

int main(void) {
    RUN_TEST(test_add);
    RUN_TEST(test_mul);
    RUN_TEST(test_sub);
    RUN_TEST(test_div);

    printf("\nИтого проверок: %d | Провалено: %d\n", tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}