/*
Vitis HLS QR Decomposition (MGS) Example for Beamforming
Reference: AMD Vitis HLS Design Tutorials
*/

#include <math.h>
#include "mgs_qrd_wbs.h"

// Complex multiplication helper
void cmult(float a, float b, float c, float d, float *R, float *I) {
    *R = (a*c) - (b*d);
    *I = (a+b)*(c+d) -(a*c) - (b*d);
}

// Complex division helper
void cdiv(float a, float b, float c, float d, float *R, float *I) {
    *R = ((a*c) + (b*d))/(c*c + d*d);
    *I = ((b*c)-(a*d))/(c*c + d*d);
}

void mgs_qrd(float Qr_i[MAX_ROW*MAX_COL], float Qi_i[MAX_ROW*MAX_COL], int col, int row, float br_i[MAX_ROW], float bi_i[MAX_ROW], float xr_i[MAX_COL],float xi_i[MAX_COL])
{
    // Reshape input arrays for parallel access
    #pragma HLS ARRAY_RESHAPE variable=Qr_i complete dim=1
    #pragma HLS ARRAY_RESHAPE variable=Qi_i complete dim=1
    #pragma HLS ARRAY_RESHAPE variable=br_i complete dim=1
    #pragma HLS ARRAY_RESHAPE variable=bi_i complete dim=1
    #pragma HLS ARRAY_RESHAPE variable=xr_i complete dim=1
    #pragma HLS ARRAY_RESHAPE variable=xi_i complete dim=1

    float Qr[MAX_ROW][MAX_COL],  Qi[MAX_ROW][MAX_COL],  br[MAX_ROW], bi[MAX_ROW],  xr[MAX_COL], xi[MAX_COL];

    // Partition internal arrays for full parallel access
    #pragma HLS ARRAY_PARTITION variable=Qr complete dim=1
    #pragma HLS ARRAY_PARTITION variable=Qi complete dim=1
    #pragma HLS ARRAY_PARTITION variable=br complete dim=1
    #pragma HLS ARRAY_PARTITION variable=bi complete dim=1
    #pragma HLS ARRAY_PARTITION variable=xr complete dim=1
    #pragma HLS ARRAY_PARTITION variable=xi complete dim=1

    int i, j, k, ii, kk;
    float Rr[MAX_ROW][MAX_ROW], Ri[MAX_ROW][MAX_ROW], Qrtb[MAX_ROW], Qitb[MAX_ROW], Qcr[MAX_ROW], Qci[MAX_ROW];
    float rtr[UNFACT], rti[UNFACT], qrt[UNFACT], qti[UNFACT], qtr[UNFACT], tr[UNFACT], ti[UNFACT], nrmr[UNFACT], nrmi[UNFACT];

    // Partition temporary arrays for parallel access
    #pragma HLS ARRAY_PARTITION variable=Rr complete dim=1
    #pragma HLS ARRAY_PARTITION variable=Ri complete dim=1
    #pragma HLS ARRAY_PARTITION variable=Qrtb complete dim=1
    #pragma HLS ARRAY_PARTITION variable=Qitb complete dim=1
    #pragma HLS ARRAY_PARTITION variable=Qcr complete dim=1
    #pragma HLS ARRAY_PARTITION variable=Qci complete dim=1

    // Read input matrices
    read_loop: for (int i = 0; i < row*col; i++) {
        #pragma HLS PIPELINE II=1
        Qr[i/col][i%col] = Qr_i[i];
        Qi[i/col][i%col] = Qi_i[i];
    }

    // Modified Gram-Schmidt QRD main loop
    l3:for (ii=0; ii<col; ii++) {
        #pragma HLS loop_tripcount min=32 max=32

        l3a:for (kk=0; kk<row; kk++) {
            #pragma HLS loop_tripcount min=32 max=32
            #pragma HLS PIPELINE II=1
            #pragma HLS UNROLL factor=UNFACT
            Qcr[kk] = Qr[kk][ii];
            Qci[kk] = Qi[kk][ii];
        }

        l4:for (j=0; j<ii; j++) {
            #pragma HLS loop_tripcount min=0 max=31 avg=15

            // Initialize accumulators
            init_acc: for (int k=0; k<UNFACT; k++) {
                #pragma HLS UNROLL
                rtr[k] = 0;
                rti[k] = 0;
            }

            l5:for (kk=0; kk<row; kk++) {
                #pragma HLS loop_tripcount min=32 max=32
                #pragma HLS UNROLL factor=UNFACT
                #pragma HLS PIPELINE II=1
                cmult(Qr[kk][j], -Qi[kk][j], Qcr[kk], Qci[kk], &tr[kk%UNFACT], &ti[kk%UNFACT]);
                rtr[kk%UNFACT] += tr[kk%UNFACT];
                rti[kk%UNFACT] += ti[kk%UNFACT];
            }

            // Reduce accumulators
            reduce: for (int k=1; k<UNFACT; k++) {
                #pragma HLS UNROLL
                rtr[0] += rtr[k];
                rti[0] += rti[k];
            }

            Rr[j][ii] = rtr[0];
            Ri[j][ii] = rti[0];

            // Orthogonalize vectors
            l6:for (kk=0; kk<row; kk++) {
                #pragma HLS loop_tripcount min=32 max=32
                #pragma HLS PIPELINE II=1
                #pragma HLS UNROLL factor=UNFACT
                qtr[kk%UNFACT] = Qr[kk][j];
                qti[kk%UNFACT] = Qi[kk][j];
                cmult(qtr[kk%UNFACT], qti[kk%UNFACT], rtr[0], rti[0], &tr[kk%UNFACT], &ti[kk%UNFACT]);
                Qcr[kk] -= tr[kk%UNFACT];
                Qci[kk] -= ti[kk%UNFACT];
                nrmr[kk%UNFACT] = 0;
                nrmi[kk%UNFACT] = 0;
            }
        }

        // Compute norm
        norm_loop: for (kk=0; kk<row; kk++) {
            #pragma HLS loop_tripcount min=32 max=32
            #pragma HLS UNROLL factor=UNFACT
            #pragma HLS PIPELINE II=1
            nrmr[kk%UNFACT] += Qcr[kk] * Qcr[kk];
            nrmi[kk%UNFACT] += Qci[kk] * Qci[kk];
        }

        // Reduce norm values
        reduce_norm: for (int k=1; k<UNFACT; k++) {
            #pragma HLS UNROLL
            nrmr[0] += nrmr[k];
            nrmi[0] += nrmi[k];
        }

        Rr[ii][ii] = sqrt(nrmr[0] + nrmi[0]);
        Ri[ii][ii] = 0.0;

        // Normalize Q vector
        normalize: for (kk=0; kk<row; kk++) {
            #pragma HLS loop_tripcount min=32 max=32
            #pragma HLS UNROLL factor=UNFACT
            #pragma HLS PIPELINE II=1
            Qcr[kk] /= Rr[ii][ii];
            Qci[kk] /= Rr[ii][ii];
        }

        // Store back to Q matrix
        store_q: for (kk=0; kk<row; kk++) {
            #pragma HLS loop_tripcount min=32 max=32
            #pragma HLS PIPELINE II=1
            #pragma HLS UNROLL factor=UNFACT
            Qr[kk][ii] = Qcr[kk];
            Qi[kk][ii] = Qci[kk];
        }
    }

    // Back substitution to solve for X
    back_sub: for (j=col-1; j >= 0; j--) {
        #pragma HLS loop_tripcount min=32 max=32
        float tr = 0, ti = 0;

        l12:for (k=j+1; k<col; k++) {
            #pragma HLS loop_tripcount min=0 max=31 avg=15
            #pragma HLS PIPELINE II=1
            float acr, aci;
            cmult(Rr[j][k], Ri[j][k], xr[k], xi[k], &acr, &aci);
            tr += acr;
            ti += aci;
        }

        cdiv((Qrtb[j] - tr), (Qitb[j] - ti), Rr[j][j], Ri[j][j], &xr[j], &xi[j]);
    }

    // Write output results
    write_loop: for (ii=0; ii<col; ii++) {
        #pragma HLS loop_tripcount min=32 max=32
        #pragma HLS PIPELINE II=1
        xr_i[ii] = xr[ii];
        xi_i[ii] = xi[ii];
    }
}
