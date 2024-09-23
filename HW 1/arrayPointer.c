#include <stdio.h>

int main() {
    int myNumbers[4] = {25, 50, 75, 100};
    int i;

    for (i = 0; i < 4; i++) {
        printf("%p\n", &myNumbers[i]);
    }

    // Get the size of the myNumbers array
    printf("%lu\n", sizeof(myNumbers));

    // Get the memory address of the myNumbers array
    printf("%p\n", myNumbers);

    // Get the memory address of the first array element
    printf("%p\n", &myNumbers[0]);

    //  Get the value of the first element in myNumbers
    printf("%d\n", *myNumbers);

    //  Get the value of the second element in myNumbers
    printf("%d\n", *(myNumbers + 1));

    //  Get the value of the forth element in myNumbers
    printf("%d\n", *(myNumbers + 3));

    //  Get the value of the outboundary element in myNumbers
    printf("%d\n", *(myNumbers + 4));

    // int *ptr = myNumbers;
    // for (i = 0; i < 4; i++) {
    //   printf("%d\n", *(ptr + i));
    // }

    // // Change the value of the first element to 13
    // *myNumbers = 13;

    // // Change the value of the second element to 17
    // *(myNumbers +1) = 17;

    // // Get the value of the first element
    // printf("%d\n", *myNumbers);

    // // Get the value of the second element
    // printf("%d\n", *(myNumbers + 1));

    return 0;
}