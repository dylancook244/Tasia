// runner.c
#include <stdio.h>

// The function from our Tasia program
extern double calculate(void);

int main(void) {
    double result = calculate();
    printf("Result: %.1f\n", result);
    return 0;
}