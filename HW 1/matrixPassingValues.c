#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

void printarray(int **a, int mrows, int ncols) {
  int i,j;
  
  for (i=0; i<mrows; i++) {
    for (j=0; j<ncols; j++)
      printf("%d ", a[i][j]);
    printf("\n");
  }
}
int **initarray(int **a, int mrows, int ncols, int value) {
  int i,j;

  for (i=0; i<mrows; i++)
    for (j=0; j<ncols; j++)
      // a[i][j] = drand48()*value;
      a[i][j] = value;
  
  return a;
}
int main() {

  int a[][2]={{2,2},{3,3},{4,4}};

  int nRows=sizeof(a)/sizeof(a[0]);
  int nCols=sizeof(a)/(nRows*sizeof(a[0][0]));
  printf("nRows=%d nCols=%d \n",nRows,nCols);
  
  nRows=3;
  nCols=5;
  int *b;
  b=(int *)malloc(nRows*nCols*sizeof(int));
  int **p;
  p=(int **)malloc(nCols*sizeof(int*));
  

  /* for row major storage */
  for (int i = 0; i < nRows; i++)
    p[i] = &b[i*nCols];
  
  // time_t sec;
  //srand48(54321);
  srand48(time(NULL));
  for(int i=0;i<10;i++)
    printf(" ---%d ",drand48()>0.5?1:0);

  printf("\n\n");
  // srand48(time(NULL));
  //printf("==%d\n",drand48()>0.5?1:0);
  srand48(time(NULL));
  for(int i=0;i<nRows;i++){
    for(int j=0;j<=nCols;j++){
      p[i][j]=drand48()>0.5?1:0;
    }
  }
  printarray(p,nRows,nCols);

  printf("%d\n",__LINE__);
  printf("%d",p[4][5]);
  printf("%d\n",__LINE__);
  
  
 return 0;
}