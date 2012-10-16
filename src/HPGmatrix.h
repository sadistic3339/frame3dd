/*
 * ==========================================================================
 *
 *       Filename:  matrix.h
 *
 *    Description:  header files for functions in matrix.c
 *
 *        Version:  1.0
 *        Created:  12/30/11 18:09:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Henri P. Gavin (hpgavin), h p gavin ~at~ duke ~dot~ e d v
 *        Company:  Duke Univ.
 *
 * ==========================================================================
 */

/* 
 * GAUSSJ
 * Linear equation solution by Gauss-Jordan elimination, [A][X]=[B] above. A[1..n][1..n]
 * is the input matrix. B[1..n][1..m] is input containing the m right-hand side vectors. On
 * output, a is replaced by its matrix inverse, and B is replaced by the corresponding set of solution
 * vectors. 
 */
void gaussj ( float **A, int n, float **B, int m );

/* 
 * LU_DCMP
 * Solves [A]{x} = {b}, simply and efficiently, by performing an 
 * LU-decomposition of matrix [A]. No pivoting is performed.
 * @param A is a diagonally dominant matrix of dimension [1..n][1..n]. 
 * @param b is a r.h.s. vector of dimension [1..n].
 *
 * {b} is updated using [LU] and then back-substitution is done to obtain {x}.  
 * {b} is replaced by {x} and [A] is replaced by the LU-reduction of itself.
 */
void lu_dcmp (
	double **A, /**< the system matrix, and its LU-reduction */
	int n,      /**< the dimension of the matrix */	
	double *b,  /**< the right hand side vector, and the solution vector */
	int reduce, /**< 1: do a forward reduction; 0: don't	*/
	int solve,  /**< 1: do a back substitution for {x};  0: don't */
	int *pd     /**< 1: positive diagonal  and  successful LU decomp'n */
);


/*
 * LDL_DCMP  
 * Solves [A]{x} = {b} simply and efficiently by performing an 
 * L D L' - decomposition of [A].  No pivoting is performed.  
 * [A] is a symmetric diagonally-dominant matrix of dimension [1..n][1..n].
 * {b} is a r.h.s. vector of dimension [1..n].
 * {b} is updated using L D L' and then back-substitution is done to obtain {x}. 
 * {b} is returned unchanged.  ldl_dcmp(A,n,d,x,x,1,1,&pd) is valid.  
 *     The lower triangle of [A] is replaced by the lower triangle L of the 
 *     L D L' reduction.  The diagonal of D is returned in the vector {d}
 */
void ldl_dcmp (
	double **A,	/**< the system matrix, and L of the L D L' decomp.*/
	int n,		/**< the dimension of the matrix		*/	
	double *d,	/**< diagonal of D in the  L D L' - decomp'n	*/
	double *b,	/**< the right hand side vector			*/
	double *x,	/**< the solution vector			*/
	int reduce,	/**< 1: do a forward reduction of A; 0: don't	*/
	int solve,	/**< 1: do a back substitution for {x}; 0: don't */
	int *pd		/**< 1: definite matrix and successful L D L' decomp'n*/
);

/* ldl_mprove
 * Improves a solution vector x[1..n] of the linear set of equations
 *	[A]{x} = {b}. 
 * The matrix A[1..n][1..n], and the vectors b[1..n] and x[1..n]
 * are input, as is the dimension n.   The matrix [A] is the L D L'
 * decomposition of the original system matrix, as returned by ldl_dcmp().
 * Also input is the diagonal vector, {d} of [D] of the L D L' decompositon.
 * On output, only {x} is modified to an improved set of values.
 */
void ldl_mprove(
	double **A, 
	int n, double *d, double *b, double *x,
	double *rms_resid,	/**< the RMS error of the solution residual */
	int *ok
);




/* ----------------------------------------------------------------------------
 LDL_DCMP_PM  -  Solves partitioned matrix equations
 [A_qq]{x_q} + [A_qr]{x_r} = {b_q}
 [A_rq]{x_q} + [A_rr]{x_r} = {b_r}
 where {b_q} and {x_r} are known and {x_q} and {b_r} are unknown
 via L D L' - decomposition of [A].  No pivoting is performed.  
 [A] is a symmetric diagonally-dominant matrix of dimension [1..n][1..n]. 
 {b} is a r.h.s. vector of dimension [1..n].
 {b} is updated using L D L' and then back-substitution is done to obtain {x}. 
 {b} is returned unchanged.  ldl_dcmp(A,n,d,x,x,1,1,&pd) is valid.  
 {q} is a vector of the indexes of known values {b_q}
 {r} is a vector of the indexes of known values {x_r}
 The lower triangle of [A_qq] is replaced by the lower triangle L of its 
 L D L' reduction.  The diagonal of D is returned in the vector {d}
 
 usage: double **A, *d, *b, *x;
 int   n, reduce, solve, pd;
 ldl_dcmp ( A, n, d, b, x, reduce, solve, &pd );
 
 H.P. Gavin, Civil Engineering, Duke University, 9 Oct 2001
 Bathe, Finite Element Procecures in Engineering Analysis, Prentice Hall, 1982
-----------------------------------------------------------------------------*/
void ldl_dcmp_pm (
        double **A,     /**< the system matrix, and L of the L D L' decomp.*/
        int n,          /**< the dimension of the matrix                */
        double *d,      /**< diagonal of D in the  L D L' - decomp'n    */
        double *b,      /**< the right hand side vector                 */
        double *x,      /**< the solution vector                        */
        int *q,         /**< q[j]=1 if  b[j] is known; q[j]=0 otherwise */
        int *r,         /**< r[j]=1 if  x[j] is known; r[j]=0 otherwise */
        int reduce,     /**< 1: do a forward reduction of A; 0: don't   */
        int solve,      /**< 1: do a back substitution for {x}; 0: don't */
        int *pd );      /**< 1: definite matrix and successful L D L' decomp'n*/
        
        
/* ----------------------------------------------------------------------------
 LDL_MPROVE_PM 
 Improves a solution vector x[1..n] of the linear set of equations

 	[A_qq]{x_q} + [A_qr]{x_r} = {b_q}
 	[A_rq]{x_q} + [A_rr]{x_r} = {b_r}
 	where {b_q} and {x_r} are known and {x_q} and {b_r} are unknown

 The matrix A[1..n][1..n], and the vectors b[1..n] and x[1..n]
 are input, as is the dimension n.   The matrix [A] is the L D L'
 decomposition of the original system matrix, as returned by ldl_dcmp().
 Also input is the diagonal vector, {d} of [D] of the L D L' decompositon.
 On output, only {x} is modified to an improved set of values.
 
 usage: double **A, *d, *b, *x, err;
 	int   n, ok;
 	ldl_mprove ( A, n, d, b, x, &err, &ok );
 
 H.P. Gavin, Civil Engineering, Duke University, 4 May 2001
 -----------------------------------------------------------------------------*/
void ldl_mprove_pm (
        double **A, int n, double *d, double *b, double *x,
        int *q, int*r,
	double *rms_resid,	/**< the RMS error of the solution residual */
	int *ok
);



/**
	calculate the pseudo-inverse of A ,
	Ai = inv ( A'*A + beta * trace(A'*A) * I ) * A' 
       	beta is a regularization factor, which should be small (1e-10)
	A is m by n      Ai is m by n  
*/
void pseudo_inv(
	double **A,	/**< an n-by-m matrix				*/
	double **Ai,	/**< the pseudo-inverse of A			*/
	int n, int m,	/**< matrix dimensions				*/
	double beta,	/**< regularization factor			*/
	int verbose	/**< 1: copious output to screen; 0: none	*/
);

/*  ----------------------------------------------------------------------------
 * PRODABj -  matrix-matrix multiplication for symmetric A            27apr01
 *               u = A * B(:,j)
 * --------------------------------------------------------------------------*/
void prodABj ( double **A, double **B, double *u, int n, int j );

/* ----------------------------------------------------------------------------
 * prodAB - matrix-matrix multiplication      C = A * B                 27apr01
 * --------------------------------------------------------------------------*/
void prodAB ( double **A, double **B, double **C, int nI, int nJ, int nK );

/** 
	INVAB  -  calculate product inv(A) * B  
       	   A is n by n      B is n by m                               6jun07
*/
void invAB(
        double **A, double **B,
        int n, int m, double **AiB,
        int *ok, int verbose );

/** 
  XTAIY  -  calculate quadratic form with inverse matrix   X' * inv(A) * Y   
   A is n by n    X is n by m     Y is n by m               15sep01
*/
void xtinvAy(
        double **X, double **A, double **Y, int n, int m, double **Ac, int verbose );

/* ----------------------------------------------------------------------------
 * xtAx - carry out matrix-matrix-matrix multiplication for symmetric A  7nov02
 *       C = X' A X     C is J by J      X is N by J     A is N by N      
 * --------------------------------------------------------------------------*/
void xtAx(double **A, double **X, double **C, int N, int J);

/* ----------------------------------------------------------------------------
 * xAy1 - carry out vector-matrix-vector multiplication for symmetric A  7apr94
 * ------------------------------------------------------------------------- */
double xtAy(double *x, double **A, double *y, int n, double *d);

/*  ---------------------------------------------------------------------------
 * invAXinvA -  calculate quadratic form with inverse matrix 
 *         replace X with inv(A) * X * inv(A) 
 *         A is n by n and symmetric   X is n by n and symmetric    15sep01
 * --------------------------------------------------------------------------*/
void invAXinvA ( double **A, double **X, int n, int verbose );

/** 
REL_NORM -  compute the relative 2-norm between two vectors       26dec01 
     compute the relative 2-norm between two vectors N and D
        return  ( sqrt(sum(N[i]*N[i]) / sqrt(D[i]*D[i]) )

*/
double rel_norm( double *N, double *D, int n );

/* Legendre 
 * compute matrices of the Legendre polynomials and its first two derivitives
 */
void Legendre( int order, float *t, int n, float **P, float **Pp, float **Ppp );

