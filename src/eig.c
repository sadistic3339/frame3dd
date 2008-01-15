/*
 ---------------------------------------------------------------------------
 FILE	eig.c - routines to solve the generalized eigenvalue problem
 ---------------------------------------------------------------------------
 Copyright (C) 1992-2007  Henri P. Gavin

 This program is free software; you may redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 http://www.fsf.org/copyleft/gpl.html

 You should have received a copy of the GNU General Public License, gpl.txt,
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ---------------------------------------------------------------------------
 Henri P. Gavin                                             hpgavin@duke.edu
 Department of Civil and Environmental Engineering
 Duke University, Box 90287
 Durham, NC  27708--0287
 ---------------------------------------------------------------------------
*/

#include <math.h>
#include <stdio.h>

/* ----------------------- double precision --------------------------------- */
#define float double
#define vector dvector
#define matrix dmatrix
#define free_vector free_dvector
#define free_matrix free_dmatrix
#define save_matrix save_dmatrix
#define show_matrix show_dmatrix
#define show_vector show_dvector
/* ----------------------- double precision --------------------------------- */

#ifndef PI
#define PI      3.141592653589793
#endif

/*-----------------------------------------------------------------------------
SUBSPACE - Find the lowest m eigen-values, w, and eigen-vectors, V, of the 
general eigen-problem  ...       K V = w M V using sub-space / Jacobi iteration
where
  K is an n by n  symmetric real (stiffness) matrix
  M is an n by n  symmetric positive definate real (mass) matrix
  w is a diagonal matrix of eigen-values
  V is a  rectangular matrix of eigen-vectors

 H.P. Gavin, Civil Engineering, Duke University, hpgavin@duke.edu  1 March 2007
 Bathe, Finite Element Procecures in Engineering Analysis, Prentice Hall, 1982
-----------------------------------------------------------------------------*/
void subspace( K, M, n, m, w, V, tol, shift, iter, ok )
float	**K, **M, *w, **V, tol, shift; 
int	n, m,			/* DoF and number of required modes	*/
	*iter,			/* sub-space iterations			*/
	*ok;			/* Sturm check result			*/
{
	float	**Kb, **Mb, **Xb, **Qb, *d, *u, *v, km, km_old,
		error=1.0, w_old = 0.0,
		*vector(),
		**matrix();
	int	i=0, j=0, k=0,
		modes,
		disp = 0,	/* display improvements to K X = M V	*/
		sturm(),	/* sturm eigenvalue check		*/
		*idx, *ivector();
	
	void	ldl_dcmp(),	/* LDL decomposition K = L D L'		*/
		ldl_mprove(),	/* improve the solution from ldl_dcmp	*/
		jacobi(),	/* Jacobi method to solve smaller prblm */
		eigsort(),	/* sort e-values and e-vectors		*/
		mat_mult1(),	/* multiply two matrices		*/
		mat_mult2(),	/* multiply two matrices		*/
		xtAx(),		/* quadratic matrix multiplication	*/
		exit(),
		show_matrix(), save_matrix(), free_matrix(), free_vector();

	if ( m > n ) {
		fprintf(stderr,"SUBSPACE: Number of eigen-values must be less");
		fprintf(stderr," than the problem dimension.\n");
		fprintf(stderr,"Desired number of eigen-values=%d \n", m);
		fprintf(stderr,"Dimension of the problem= %d \n", n);
		exit(1);
	}

	d  = vector(1,n);
	u  = vector(1,n);
	v  = vector(1,n);
	Kb = matrix(1,m,1,m);
	Mb = matrix(1,m,1,m);
	Xb = matrix(1,n,1,m);
	Qb = matrix(1,m,1,m);
	idx = ivector(1,m);

	for (i=1; i<=m; i++) {
	 idx[i] = 0;
	 for (j=i; j<=m; j++)
	  Kb[i][j]=Kb[j][i] = Mb[i][j]=Mb[j][i] = Qb[i][j]=Qb[j][i] = 0.0;
	}

	for (i=1; i<=n; i++) for (j=1; j<=m; j++) Xb[i][j] = V[i][j] = 0.0;

	modes = (int) ( (float)(0.5*m) > (float)(m-8.0) ? (int)(m/2.0) : m-8 );

					/* shift eigen-values by this much */
	for (i=1;i<=n;i++) for (j=i;j<=n;j++) K[i][j] += shift*M[i][j];


	ldl_dcmp ( K, n, u, v, v, 1, 0, ok );	/* use L D L' decomp  */

	for (i=1; i<=n; i++) {
		if ( M[i][i] <= 0.0 )  {
		 fprintf(stderr," subspace: M[%d][%d] = %e \n", i,i, M[i][i] );
		 exit(1);
		}
		d[i] = K[i][i] / M[i][i];
	}

	km_old = 0.0;
	for (k=1; k<=m; k++) {
	    km = d[1];
	    for (i=1; i<=n; i++) {
		if ( km_old <= d[i] && d[i] <= km ) {
			*ok = 1;
			for (j=1; j<=k-1; j++) if ( i == idx[j] ) *ok = 0;
			if (*ok) {
				km = d[i];
				idx[k] = i;
			}
		}
	    }
	    if ( idx[k] == 0 ) {
			i = idx[1];
			for ( j=1; j<k; j++ ) if ( i < idx[j] ) i = idx[j]; 
			idx[k] = i+1;
			km = d[i+1];
	    }
	    km_old = km;
	}

/*	for (k=1; k<=m; k++)    printf(" idx[%d] = %d \n", k, idx[k] );	*/
	for (k=1; k<=m; k++) {
		V[idx[k]][k] = 1.0;
		*ok = idx[k] % 6; 
	/*	printf(" idx[%3d] = %3d   ok = %d \n", k , idx[k], *ok);  */
		switch ( *ok ) {
			case 1:	i =  1;	j =  2;	break;
			case 2:	i = -1;	j =  1;	break;
			case 3:	i = -1;	j = -2;	break;
			case 4:	i =  1;	j =  2;	break;
			case 5:	i = -1;	j =  1;	break;
			case 0:	i = -1;	j = -2;	break;
		}
		V[idx[k]+i][k] = 0.2; V[idx[k]+j][k] = 0.2;
	}

/*	for (i=1; i<=n; i++)	V[i][1] = M[i][i];	/* diag(M)	*/


	
	*iter = 0;
	do { 					/* Begin sub-space iterations */

		for (k=1; k<=m; k++) {		/* K Xb = M V	(12.10) */
			mat_mult1 ( M, V, v, n, k );
			ldl_dcmp ( K, n, u, v, d, 0, 1, ok ); /* LDL bk-sub */

                                        /* improve the solution iteratively */
			if (disp) fprintf(stderr,"  RMS matrix error:");
			error = *ok = 1;
			do {
				ldl_mprove ( K, n, u, v, d, &error, ok );
				if (disp) fprintf(stderr,"%9.2e", error );
			} while ( *ok );
			if (disp) fprintf(stderr,"\n");

			for (i=1; i<=n; i++)	Xb[i][k] = d[i];
		}

		xtAx ( K, Xb, Kb, n,m );	/* Kb = Xb' K Xb (12.11) */
		xtAx ( M, Xb, Mb, n,m );	/* Mb = Xb' M Xb (12.12) */

		jacobi ( Kb, Mb, w, Qb, m );		/* (12.13) */

		mat_mult2 ( Xb, Qb, V, n,m,m );		/* V = Xb Qb (12.14) */

		eigsort ( w, V, n, m );

		if (w[modes] == 0.0) {
		 fprintf(stderr," subspace: Zero frequency found! \n");
		 fprintf(stderr," subspace: w[%d] = %e \n", modes, w[modes] );
		 exit(1);
		}
		error = fabs( w[modes] - w_old ) / w[modes];

		(*iter)++;
		if (disp) fprintf(stderr," iter = %d  w[%d] = %f error = %e\n",
						*iter, modes, w[modes], error );
		w_old = w[modes];

		if ( *iter > 1000 ) {
		    fprintf(stderr,"  subspace(): Iteration limit exceeded\n");
		    fprintf(stderr," rel. error = %e > %e\n", error, tol );
		    exit(1);
		}

	} while	( error > tol );		/* End   sub-space iterations */
			

	for (k=1; k<=m; k++) {			/* shift eigen-values */
	    if ( w[k] > shift )	w[k] = w[k] - shift;
	    else		w[k] = shift - w[k];
	}

	printf(" %4d sub-space iterations,   error: %.4e \n", *iter, error );
	for ( k=1; k<=m; k++ ) {
		printf("  mode: %2d\tDoF: %5d\t %9.4lf Hz\n",
				k, idx[k], sqrt(w[k])/(2.0*PI) );
	}

	*ok = sturm ( K, M, n, m, shift, w[modes]+tol ); 

	for (i=1;i<=n;i++) for (j=i;j<=n;j++) K[i][j] -= shift*M[i][j];

	free_matrix(Kb,1,m,1,m);
	free_matrix(Mb,1,m,1,m);
	free_matrix(Xb,1,n,1,m);
	free_matrix(Qb,1,m,1,m);

	return;
}


/*-----------------------------------------------------------------------------
 JACOBI - Find all eigen-values, E, and eigen-vectors, V,
 of the general eigen-problem  K V = E M V
 using Jacobi iteration, with efficient matrix rotations.  
 K is a symmetric real (stiffness) matrix
 M is a symmetric positive definate real (mass) matrix
 E is a diagonal matrix of eigen-values
 V is a  square  matrix of eigen-vectors

 H.P. Gavin, Civil Engineering, Duke University, hpgavin@duke.edu  1 March 2007
 Bathe, Finite Element Procecures in Engineering Analysis, Prentice Hall, 1982
-----------------------------------------------------------------------------*/
void jacobi ( K, M, E, V, n )
float	**K, **M, *E, **V;
int	n;
{
	int	iter,
		d,i,j,k;
	float	Kii, Kjj, Kij, Mii, Mjj, Mij, Vki, Vkj, 
		alpha, beta, gamma,
		s, tol=0.0;
	void	rotate(); 

	Kii = Kjj = Kij = Mii = Mjj = Mij = Vki = Vkj = 0.0;

	for (i=1; i<=n; i++) for (j=i+1; j<=n; j++)	V[i][j] = V[j][i] = 0.0;
	for (d=1; d<=n; d++)	V[d][d] = 1.0;

	for (iter=1; iter<=2*n; iter++) {	/* Begin Sweep Iteration */

	  tol = pow(0.01,(2*iter));
	  tol = 0.0;

	  for (d=1; d<=(n-1); d++) {	/* sweep along upper diagonals */
	    for (i=1; i<=(n-d); i++) {		/* row */
	      j = i+d;				/* column */

	      Kij = K[i][j];
	      Mij = M[i][j];

	      if ( Kij*Kij/(K[i][i]*K[j][j]) > tol ||
		   Mij*Mij/(M[i][i]*M[j][j]) > tol ) {      /* do a rotation */

		Kii = K[i][i] * Mij     -   Kij     * M[i][i];
		Kjj = K[j][j] * Mij     -   Kij     * M[j][j];
		s   = K[i][i] * M[j][j] -   K[j][j] * M[i][i];

		if  ( s >= 0.0 ) gamma = 0.5*s + sqrt( 0.25*s*s + Kii*Kjj );
		else		 gamma = 0.5*s - sqrt( 0.25*s*s + Kii*Kjj );
		
		alpha =  Kjj / gamma ;
		beta  = -Kii / gamma ;

		rotate(K,n,alpha,beta,i,j);		/* make Kij zero */
		rotate(M,n,alpha,beta,i,j);		/* make Mij zero */

		for (k=1; k<=n; k++) {	/*  update eigen-vectors  V = V * P */
			Vki = V[k][i];
			Vkj = V[k][j];
			V[k][i] = Vki + beta *Vkj;
			V[k][j] = Vkj + alpha*Vki;
		}
	      } 				/* rotations complete */
	    }					/* row */
	  }					/* diagonal */
	}					/* End Sweep Iteration */

	for (j=1; j<=n; j++) {			/* scale eigen-vectors */
		Mjj = sqrt(M[j][j]);
		for (i=1; i<=n; i++)	V[i][j] /= Mjj;
	}
			
	for (j=1; j<=n; j++)
		E[j] = K[j][j]/M[j][j];		/* eigen-values */

	return;
}


/*-----------------------------------------------------------------------------
ROTATE - rotate an n by n symmetric matrix A such that A[i][j] = A[j][i] = 0
     A = P' * A * P  where diag(P) = 1 and P[i][j] = alpha and P[j][i] = beta.
     Since P is sparse, this matrix multiplcation can be done efficiently.  
-----------------------------------------------------------------------------*/
void rotate ( A, n, alpha, beta, i, j )
float	**A, alpha, beta;
int	i, j, n;
{
	float	Aii, Ajj, Aij,			/* elements of A	*/
		*Ai, *Aj, *vector();		/* i-th and j-th rows of A */
	int	k;
	void	free_vector();


	Ai = vector(1,n);
	Aj = vector(1,n);

	for (k=1; k<=n; k++) {
		Ai[k] = A[i][k];
		Aj[k] = A[j][k];
	}

	Aii = A[i][i];	
	Ajj = A[j][j];	
	Aij = A[i][j];

	A[i][i] = Aii + 2*beta *Aij + beta *beta *Ajj ;
	A[j][j] = Ajj + 2*alpha*Aij + alpha*alpha*Aii ;

	for (k=1; k<=n; k++) {
		if ( k != i && k != j ) {
			A[k][i] = A[i][k] = Ai[k] + beta *Aj[k];
			A[k][j] = A[j][k] = Aj[k] + alpha*Ai[k];
		}
	}
	A[j][i] = A[i][j] = 0;

	free_vector(Ai,1,n);
	free_vector(Aj,1,n);

	return;
}


/*------------------------------------------------------------------------------
STODOLA  -  calculate the lowest m eigen-values and eigen-vectors of the
generalized eigen-problem, K v = w M v, using a matrix iteration approach
with shifting. 								15oct98

 H.P. Gavin, Civil Engineering, Duke University, hpgavin@duke.edu  12 Jul 2001
------------------------------------------------------------------------------*/
void stodola ( K, M, n, m, w, V, tol, shift, iter, ok )
float	**K, **M, *w, **V, tol, shift;	
int	n, m, *iter, *ok;	/* DoF and number of required modes	*/
{
	float	**D,		/* the dynamics matrix, D = K^(-1) M	*/
		d_min = 0.0,	/* minimum value of D[i][i]		*/
		d_max = 0.0,	/* maximum value of D[i][i]		*/
		d_old = 0.0,	/* previous extreme value of D[i][i]	*/
		*d,		/* columns of the D, M, and V matrices	*/
		*u, *v,		/* trial eigen-vector vectors		*/
		*c,		/* coefficients for lower mode purge	*/
		vMv,		/* factor for mass normalization	*/
		RQ, RQold=0.0,	/* Raliegh quotient			*/
		xAy1(),		/* quadratic form calculation		*/
		error = 1.0,
		*vector(),
		**matrix();

	int	i_ex,		/* location of minimum value of D[i][i]	*/
		modes,		/* number of desired modes		*/
		disp = 0,	/* 1: display convergence error; 0: dont*/
		i,j,k,
		sturm();	/* Sturm eigenvalue check		*/

	void	ldl_dcmp(),	/* L D L' decomposition K = L D L'	*/
		ldl_mprove(),	/* improve the solution from ldl_dcmp	*/
		eigsort(),
		exit(),
		save_matrix(), free_matrix(), free_vector();

	D  = matrix(1,n,1,n);
	d  = vector(1,n);
	u  = vector(1,n);
	v  = vector(1,n);
	c  = vector(1,m);

	modes = (int) ( (float)(0.5*m) > (float)(m-8) ? (int)(m/2.0) : m-8 );

					/* shift eigen-values by this much */
	for (i=1;i<=n;i++) for (j=i;j<=n;j++) K[i][j] += shift*M[i][j];

	ldl_dcmp ( K, n, u, v, v, 1, 0, ok );	/* use L D L' decomp	*/
	if (*ok<0) {
		fprintf(stderr," Make sure that all six");
		fprintf(stderr," rigid body translation are restrained.\n");
		exit(1); 
	}
						/* calculate  D = K^(-1) M */
	for (j=1; j<=n; j++) {
		for (i=1; i<=n; i++)	v[i] = M[i][j];

		ldl_dcmp ( K, n, u, v, d, 0, 1, ok );	/* L D L' bk-sub */

					/* improve the solution iteratively */
		if (disp) fprintf(stderr,"  RMS matrix error:");
		error = *ok = 1;
		do {
			ldl_mprove ( K, n, u, v, d, &error, ok );
			if (disp) fprintf(stderr,"%9.2e", error );
		} while ( *ok );
		if (disp) fprintf(stderr,"\n");

		for (i=1; i<=n; i++)	D[i][j] = d[i];
	}

/*	save_matrix ( n, n, D, "D" );		/* save dynamics matrix */

	*iter = 0;
	for (i=1; i<=n; i++) if ( D[i][i] > d_max )	d_max = D[i][i];
	d_old = d_min = d_max;
	for (i=1; i<=n; i++) if ( D[i][i] < d_min )	d_min = D[i][i];

	for (k=1; k<=m; k++) {			/* loop over lowest m modes */

	    d_max = d_min;
	    for (i=1; i<=n; i++) {			/* initial guess */
		u[i] = 0.0;
		if ( D[i][i] < d_old && D[i][i] > d_max ) {
			d_max = D[i][i];
			i_ex = i;
		}
	    }
	    u[i_ex] = 1.0;  u[i_ex+1] = 1.e-4; 
	    d_old = d_max;

	    vMv = xAy1 ( u, M, u, n, d );		/* mass-normalize */
	    for (i=1; i<=n; i++)	u[i] /= sqrt ( vMv ); 

	    for (j=1; j<k; j++) {			/* purge lower modes */
		for (i=1; i<=n; i++)	v[i] = V[i][j];
		c[j] = xAy1 ( v, M, u, n, d );
	    }
	    for (j=1; j<k; j++)
		for (i=1; i<=n; i++)	u[i] -= c[j] * V[i][j];
			
	    vMv = xAy1 ( u, M, u, n, d );		/* mass-normalize */
	    for (i=1; i<=n; i++)	u[i] /= sqrt ( vMv ); 
	    RQ  = xAy1 ( u, K, u, n, d );		/* Raleigh quotient */

	    do {					/* iterate	*/
		for (i=1; i<=n; i++) {			/* v = D u	*/
			v[i] = 0.0;
			for (j=1; j<=n; j++)	v[i] += D[i][j] * u[j];
		}

		vMv = xAy1 ( v, M, v, n, d );		/* mass-normalize */
		for (i=1; i<=n; i++)	v[i] /= sqrt ( vMv ); 

		for (j=1; j<k; j++) {			/* purge lower modes */
			for (i=1; i<=n; i++)	u[i] = V[i][j];
			c[j] = xAy1 ( u, M, v, n, d );
		}
		for (j=1; j<k; j++)
			for (i=1; i<=n; i++)	v[i] -= c[j] * V[i][j];

		vMv = xAy1 ( v, M, v,  n, d );		/* mass-normalize */
		for (i=1; i<=n; i++)	u[i] = v[i] / sqrt ( vMv ); 

		RQold = RQ;
		RQ = xAy1 ( u, K, u, n, d );		/* Raleigh quotient */
		(*iter)++;

		if ( *iter > 1000 ) {
		    fprintf(stderr,"  stodola(): Iteration limit exceeded\n");
		    fprintf(stderr," rel. error = %e > %e\n",
						(fabs(RQ - RQold)/RQ) , tol );
		    exit(1);
		}

	    } while ( (fabs(RQ - RQold)/RQ) > tol );

	    for (i=1; i<=n; i++)	V[i][k] = v[i];

	    w[k] = xAy1 ( u, K, u, n, d );
	    if ( w[k] > shift )	w[k] = w[k] - shift;
	    else		w[k] = shift - w[k];

	    printf("  mode: %2d\tDoF: %5d\t", k, i_ex );
	    printf(" %9.4f Hz\t iter: %4d   error: %.4e \n",
		sqrt(w[k])/(2.0*PI), *iter, (fabs(RQ - RQold)/RQ) );
	}

	eigsort ( w, V, n, m );

	*ok = sturm ( K, M, n, m, shift, w[modes]+tol );

/*	save_matrix ( n, m, V, "V" );	/* save mode shape matrix */

	free_matrix(D,1,n,1,n);
	free_vector(d,1,n);
	free_vector(u,1,n);
	free_vector(v,1,n);
	free_vector(c,1,m);

	return;
}


/*------------------------------------------------------------------------------
xAy1  -  carry out vector-matrix-vector multiplication for symmetric A	7apr94
------------------------------------------------------------------------------*/
float xAy1 ( x, A, y, n, d )
float	*x, **A, *y, *d;
int	n;
{
	float	xtAy = 0.0;
	int	i,j;

	for (i=1; i<=n; i++) {				/* d = A y	*/
		d[i]  = 0.0;
		for (j=1; j<=n; j++) {		/* A in upper triangle only */
			if ( i <= j )	d[i] += A[i][j] * y[j];
			else		d[i] += A[j][i] * y[j];
		}
	}
	for (i=1; i<=n; i++)	xtAy += x[i] * d[i];	/* xAy = x' A y	*/
	return ( xtAy );
}


/*------------------------------------------------------------------------------
xtAx -  carry out matrix-matrix-matrix multiplication for symmetric A	7nov02
	C = X' A X     C is J by J	X is N by J	A is N by N	 
------------------------------------------------------------------------------*/
void	xtAx ( A, X, C, N, J )
float	**A, **X, **C;
int	N, J;
{
	float	**AX, **matrix();
	int	i,j,k,l;
	void	free_matrix();

	AX = matrix(1,N,1,J);

	for (i=1; i<=J; i++)	for (j=1; j<=J; j++)	 C[i][j] = 0.0;
	for (i=1; i<=N; i++)	for (j=1; j<=J; j++)	AX[i][j] = 0.0;

	for (i=1; i<=N; i++) {		/* use upper triangle of A */
		for (j=1; j<=J; j++) {
			for (k=1; k<=N; k++) {
				if ( i <= k )	AX[i][j] += A[i][k] * X[k][j];
				else		AX[i][j] += A[k][i] * X[k][j];
			}
		}
	}

	for (i=1; i<=J; i++) 
		for (j=1; j<=J; j++) 
			for (k=1; k<=N; k++) 
				C[i][j] += X[k][i] * AX[k][j];

        for (i=1; i<=J; i++)            /* make  C  symmetric */
                for (j=i; j<=J; j++) 
                        C[i][j] = C[j][i] = 0.5 * ( C[i][j] + C[j][i] );

	free_matrix(AX,1,N,1,J);
	return;
}


/*------------------------------------------------------------------------------
mat_mult1  -  matrix-matrix multiplication for symmetric A		27apr01
------------------------------------------------------------------------------*/
void	mat_mult1 ( A, B, u, n, j )       /*      u = A * B(:,j)      */
float   **A, **B, *u;
int     n, j;
{
        int     i, k;

        for (i=1; i<=n; i++)	u[i] = 0.0;

        for (i=1; i<=n; i++) {
                for (k=1; k<=n; k++) {
                        if ( i <= k )	u[i] += A[i][k]*B[k][j];
                        else		u[i] += A[k][i]*B[k][j];
		}
        }
	return;
}


/*------------------------------------------------------------------------------
mat_mult2  -  matrix-matrix multiplication 	C = A * B		27apr01
------------------------------------------------------------------------------*/
void	mat_mult2 ( A, B, C, I,J,K )
float   **A, **B, **C;
int     I, J, K;
{
	int     i, j, k;

	for (i=1; i<=I; i++)  
		for (k=1; k<=K; k++)  
			C[i][k] = 0.0;

	for (i=1; i<=I; i++) 
		for (k=1; k<=K; k++) 
			for (j=1; j<=J; j++) 
				C[i][k] += A[i][j]*B[j][k];
	return;
}



/*------------------------------------------------------------------------------
EIGSORT  -  Given the eigenvallues e[1..m] and eigenvectors v[1..n][1..m],
this routine sorts the eigenvalues into ascending order, and rearranges
the columns of v correspondingly.  The method is straight insertion.
Adapted from Numerical Recipes in C, Ch 11
------------------------------------------------------------------------------*/
void eigsort ( e, v, n, m )
float   *e, **v;
int	n, m;				   /* n ~ DoF, m ~ modes in V */
{
	int	k,j,i;
	float   p=0;

	for (i=1;i<m;i++) {
		k=i;
		p=e[k];
		for (j=i+1;j<=m;j++)
			if ( e[j] <= p )
				p=e[k=j];	/* find smallest eigen-value */
		if (k != i) {
			e[k]=e[i];		/* swap eigen-values	*/
			e[i]=p;
			for (j=1;j<=n;j++) {	/* swap eigen-vectors	*/
				p=v[j][i];
				v[j][i]=v[j][k];
				v[j][k]=p;
			}
		}
	}
	return;
}

/*-----------------------------------------------------------------------------
STURM  -  Determine the number of eigenvalues, w, of the general eigen-problem
  K V = w M V which are below the value ws,  
  K is an n by n  symmetric real (stiffness) matrix
  M is an n by n  symmetric positive definate real (mass) matrix
  w is a diagonal matrix of eigen-values
  ws is the limit

 H.P. Gavin, Civil Engineering, Duke University, hpgavin@duke.edu  30 Aug 2001
 Bathe, Finite Element Procecures in Engineering Analysis, Prentice Hall, 1982
-----------------------------------------------------------------------------*/
int sturm ( K, M, n, m, shift, ws )
float	**K, **M, shift, ws;
int	n, m;			/* DoF and number of required modes	*/
{
	float	ws_shift, *d, *vector();
	int	ok=0, i,j, modes;
	
	void	ldl_dcmp(),	/* L D L' decomposition of [ K - ws M ]	*/
		free_vector();

	d  = vector(1,n);

	modes = (int) ( (float)(0.5*m) > (float)(m-8.0) ? (int)(m/2.0) : m-8 );

	ws_shift = ws + shift;			/* shift [K]	*/
	for (i=1; i<=n; i++) for (j=i; j<=n; j++) K[i][j] -= ws_shift*M[i][j];

	ldl_dcmp ( K, n, d, d, d, 1, 0, &ok );

	fprintf(stderr,"  There are %d modes below %f Hz.",
						-ok, sqrt(ws)/(2.0*PI) );
	if ( -ok > modes ) {
		fprintf(stderr," ... %d modes were not found.\n", -ok-modes );
		fprintf(stderr," Try increasing the number of modes in \n");
		fprintf(stderr," order to get the missing modes below %f Hz.\n",
							sqrt(ws)/(2.0*PI) );
	} else  fprintf(stderr," ... all %d modes were found.\n", modes );

	for (i=1; i<=n; i++) for (j=i; j<=n; j++) K[i][j] += ws_shift*M[i][j];

	free_vector(d,1,n);

	return ok;
}

/*----------------------------------------------------------------------------
CHECK_NON_NEGATIVE -  checks that a value is non-negative
-----------------------------------------------------------------------------*/
void check_non_negative(x,i)
float	x;
int	i;
{
	if ( x <= 1.0e-100 )  {
		printf(" value %e is less than or equal to zero ", x );
		printf(" i = %d \n", i );
	} else {
		return;
	}
}
