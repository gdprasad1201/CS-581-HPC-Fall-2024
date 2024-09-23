#include <stdio.h>

int main() {
  int a=2;
  int *p = &a;
  printf("*p=%d\n", *p);
  printf("&a=%p\n",&a);
  printf("p=%p \n", p);
 
}