#include <stdio.h>
#include <stdlib.h>

extern int add(int, int);
extern int mul(int, int);

int main(int argc, char **argv)
{
    int a, b;

    if (argc < 3) {
        // must input 2 number args
        fprintf(stderr, "input 2 numbers for calc add or mul.\n");
        fprintf(stderr, "Usage) ./a.out 1 2\n");
        return -1;
    }

    a = atoi(argv[1]);
    b = atoi(argv[2]);

    if (a < b) {
        printf("the answer is a + b = %d\n", add(a, b));
    } else  {
        printf("the answer is a * b = %d\n", mul(a, b));
    }
  
    return 0;
}
