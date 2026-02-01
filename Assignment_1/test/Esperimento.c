#include <stdio.h>

int main() {
    int x = 5;
    int a, b = 3, c;
    
    int sum = x + 0;
    int prod = x * 1;
    int mult = 15 * x;
    int div = x / 8;
    a = b + 1;
    c = a - 1;

    printf("x + 0 = %d\n", sum);
    printf("x * 1 = %d\n", prod);
    printf("15 * x = %d\n", mult);
    printf("x / 8 = %d\n", div);
    printf("a = b + 1 = %d\n", a);
    printf("c = a - 1 = %d\n", c);

    return 0;

}
