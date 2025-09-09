#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* print array */
int print_array(double **array, int r, int c) {
  for (int i = 0; i < r; i++) {
    for (int j = 0; j < c; j++) {
      // array[i][j] = createFloat(array[i][j],4);
      printf("%.4f ", array[i][j]);
    }
    printf("\n");
  }
  printf("\n");
  return 0;
}

/* to copy the array */
void swap(double ***dest, double ***src) {
  double **temp = *src;
  *src = *dest;
  *dest = temp;
}
/*to create arrays*/
double **create_array(int r, int c) {
  double **arr =
      (double **)malloc(r * sizeof(double *)); // Allocate memory for rows
  for (int i = 0; i < r; i++) {
    arr[i] = (double *)malloc(c * sizeof(double)); // memory for columns
    for (int j = 0; j < c; j++) {
      if ((i == 0) || (j == 0) || (j == c - 1) || (i == r - 1)) {
        arr[i][j] = 1.0;
        // createFloat(1.00,2);//createFloat(randomFloat(),3);
      } else {
        arr[i][j] = 0.0;
        // createFloat(0.00,2);//createFloat(randomFloat(),3);
      }
    }
  }
  return arr;
}
/* free(memory) */
void free_array(double **arr, int r) {
  for (int i = 0; i < r; i++) {
    free(arr[i]);
  }
  free(arr);
}
