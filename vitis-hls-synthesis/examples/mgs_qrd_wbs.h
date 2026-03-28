/*
Header file for QR Decomposition Example
*/

#ifndef MGS_QRD_WBS_H_
#define MGS_QRD_WBS_H_

// Design configuration parameters
#define MAX_ROW 32
#define MAX_COL 32
#define UNFACT 4 // Unroll factor for parallelization

// Function prototypes
void mgs_qrd(float Qr_i[MAX_ROW*MAX_COL], float Qi_i[MAX_ROW*MAX_COL], int col, int row, float br_i[MAX_ROW], float bi_i[MAX_ROW], float xr_i[MAX_COL],float xi_i[MAX_COL]);
void cmult(float a, float b, float c, float d, float *R, float *I);
void cdiv(float a, float b, float c, float d, float *R, float *I);

#endif /* MGS_QRD_WBS_H_ */
