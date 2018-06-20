#include <cmath>
#include <iostream>
#include <complex>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include "common.h"
#include "hfwfn.h"
#include "solver.h"
#include "tei.h"
#include "util.h"
#include "evalm.h"

/*
 * Notes for Solver:
 *
 *   - KT 06/2018 Added ahm_exp here because it makes use of the 
 *     Eigenvalues package. 
 *   - Find a real convergence criteria for ahm_exp based on repreated
 *     mutliplication
 *
 * */

void canort( Eigen::Ref<Eigen::MatrixXf> const s, Eigen::Ref<Eigen::MatrixXcf> xs , int& dim) {
  /*  
 *    Find an orthogonal transformation using canonical orthogonalization for a real matrix
 *    */

  int indx=-1 ;
  float thresh = 1e-7 ;
  std::complex<float> eig ;
  Eigen::EigenSolver<Eigen::MatrixXf> s_diag( s, true) ;
  Eigen::MatrixXcf s_U ;
  Eigen::VectorXcf s_eig ;

  /* Allocate some space to build our transformation matrix. */

  s_eig.resize( dim) ;
  s_U.resize( dim, dim) ;

  s_eig = s_diag.eigenvalues() ;

  s_U = s_diag.eigenvectors() ;

  xs = s_U.householderQr().householderQ() ;

  for (int i = 0; i < dim; i++ ) {
    eig = s_eig(i) ;
    if ( std::real(eig) <= thresh && std::imag(eig) <= thresh ) {
      std::cout << "Linear dependency found in canort.  Reducing basis." << std::endl ;
      std::cout << s_eig(i) << " is less than the threshhold" << std::endl ;
      std::cout << "Reducing basis" << std::endl ;
    } else {
      indx ++ ;
    }
    xs.col(indx) = xs.col(i)/std::sqrt(eig) ;
  }
 
  if ( indx != dim-1 ){
    xs.resize(dim,indx) ;
  }

  dim = indx + 1 ;
 
  /* Deallocate  */

  s_eig.resize( 0) ;
  s_U.resize( 0, 0) ;

  return ;

} ;

void canort( Eigen::Ref<Eigen::MatrixXcf> const s, Eigen::Ref<Eigen::MatrixXcf> xs , int& dim) {
  /*  
 *    Find an orthogonal transformation using canonical orthogonalization for a complex matrix
 *    */

  int indx=-1 ;
  float thresh = 1e-7 ;
  std::complex<float> eig ;
  Eigen::ComplexEigenSolver<Eigen::MatrixXcf> s_diag( s, true) ;
  Eigen::MatrixXcf s_U ;
  Eigen::VectorXcf s_eig ;

  /* Allocate some space to build our transformation matrix. */

  s_eig.resize( dim) ;
  s_U.resize( dim, dim) ;

  s_eig = s_diag.eigenvalues() ;

  s_U = s_diag.eigenvectors() ;

  xs = s_U.householderQr().householderQ() ;

  for (int i = 0; i < dim; i++ ) {
    eig = s_eig(i) ;
    if ( abs(std::real(eig)) <= thresh && abs(std::imag(eig)) <= thresh ) {
      std::cout << "Linear dependency found in canort.  Reducing basis." << std::endl ;
      std::cout << s_eig(i) << " is less than the threshhold" << std::endl ;
      std::cout << "Reducing basis" << std::endl ;
    } else {
      indx ++ ;
    }
    xs.col(indx) = xs.col(i)/std::sqrt(eig) ;
  }
 
//  if ( indx != dim-1 ){
//    xs.resize(4,indx) ;
//  }

  dim = indx + 1 ;
 
  /* Deallocate  */

  s_eig.resize( 0) ;
  s_U.resize( 0, 0) ;

  return ;

} ;

void ahm_exp( Eigen::Ref<Eigen::MatrixXcf> x, Eigen::Ref<Eigen::MatrixXcf> u, int dim,  int opt) {
/*
 *  Anti-Hermetian Matrix EXPonential
 *
 *  Given an anti-hermetian matrix in an exponential, build a unitary
 *  matrix. 
 *
 *  This is done two ways. The first is by diagonalizing the 
 *  anti-hermetian matrix
 *
 *     X = iVdVt     VtV = 1
 *
 *  U = exp(X) = exp(iVdVt) = V*exp(id)*Vt
 *
 *  The other method is by the definition of a matrix exponential
 *
 *  U = exp(X) = 1 +  sum_{n=1}^{infty} X^{n}/n!
 *
 *  where 1 is the unit matrix.
 *
 *
 * */ 
  int n = 0 ;
  float denom=1.0 ;
  Eigen::ComplexEigenSolver<Eigen::MatrixXcf> xs ;
  Eigen::MatrixXcf d ;
  Eigen::MatrixXcf tmp ;

  tmp.resize( dim, dim) ;
  d.resize( dim, dim) ;
  d.setIdentity() ;

  if ( opt == 0 ) {
    /* Default behavior.  Get the eigenvalues and vectors of x */
    xs.compute( x) ;

    /* Build d */
    for ( int i = 0; i < dim; i++ ){
      d( i, i) = std::exp(xs.eigenvalues()[i]) ;
    }

    /* Build U by Vtexp(idelta)V */
    tmp = d*xs.eigenvectors() ;
    u = xs.eigenvectors().adjoint()*tmp ;

  } else if ( opt == 1 ) {
    /* Find U by repeated multiplication */
      u.setZero() ;

      /* What should the convergence criteria be? */
      while( n < 15 ) {
        u += d ;
        n ++ ;
        tmp = d*x ;
        denom = 1.0/static_cast<float>(n) ;
        d = denom*tmp ;
      }

  } else {

    std::cout << "Unrecgonized opt in ahm_exp" << std::endl ;
    std::cout << "Returning Unit matrix" << std::endl ;
    u.setIdentity() ;

  }

  return ;

} ;

void trci( common& com, std::vector<hfwfn>& det, Eigen::Ref<Eigen::MatrixXcf> H, std::vector<tei>& intarr) {

/*
 * This routine accepts N determinants into the vector. These determinants
 * are expanded in the "Time Reversal" Basis of Complex Conjugation(K),
 * Spin Flip(F), and Time Reversal(T).  This will generate 4N
 * determinants.
 * */

  Eigen::MatrixXcf CI_s ;
  Eigen::MatrixXcf CI_h ;
  Eigen::MatrixXcf tmo ;
  hfwfn bra ;
  hfwfn ket ;
  int CI_d ;
  int idt_lb ;
  int jdt_lb ;
  int idt_ty ;
  int jdt_ty ;

  CI_d = 4*det.size() ;

  CI_s.resize( CI_d, CI_d) ;
  CI_h.resize( CI_d, CI_d) ;

  /* Build a hfwfn container for calculations */
  tmo.resize( 2*com.nbas(), 2*com.nbas()) ;
  tmo.setIdentity() ;
  bra.fil_mos( com.nbas(), tmo, 6) ;
  ket.fil_mos( com.nbas(), tmo, 6) ;

  for ( int i=0; i < CI_d; i++ ) {
    idt_lb = i/4 ;
    idt_ty = i % 4 ;

    if ( idt_ty == 0 ){
    /* |Phi> */
      det[idt_lb].get_mos( tmo) ;
      bra.set_mos( tmo) ;

    } else if ( idt_ty == 1 ){
    /* K|Phi> */
      K_op( det[idt_lb], tmo, com.nbas()) ;
      bra.set_mos( tmo) ;

    } else if ( idt_ty == 2 ){
    /* F|Phi> */
      F_op( det[idt_lb], tmo, com.nbas()) ;
      bra.set_mos( tmo) ;

    } else if ( idt_ty == 3 ){
    /* T|Phi> */
      T_op( det[idt_lb], tmo, com.nbas()) ;
      bra.set_mos( tmo) ;

    }

    for ( int j=0; j < CI_d; j++ ) {
      jdt_lb = j/4 ;
      jdt_ty = j % 4 ;

      if ( jdt_ty == 0 ){
      /* |Phi> */
        det[jdt_lb].get_mos( tmo) ;
        ket.set_mos( tmo) ;
 
      } else if ( jdt_ty == 1 ){ 
      /* K|Phi> */
        K_op( det[jdt_lb], tmo, com.nbas()) ;
        ket.set_mos( tmo) ;

      } else if ( jdt_ty == 2 ){
      /* F|Phi> */
        F_op( det[jdt_lb], tmo, com.nbas()) ;
        ket.set_mos( tmo) ;

      } else if ( jdt_ty == 3 ){
      /* T|Phi> */
        T_op( det[jdt_lb], tmo, com.nbas()) ;
        ket.set_mos( tmo) ;

      }
      /* Build <phi|H|psi> and <phi|psi> */
        CI_h( i, j) = fockop( com, H, intarr, bra, ket, CI_s( i, j)) ;
      }
    }

  std::cout << " CI_H " << std::endl << CI_h << std::endl ;
  std::cout << " CI_S " << std::endl << CI_s << std::endl ;

  tmo.resize( 0, 0) ;
  CI_h.resize( 0, 0) ;
  CI_s.resize( 0, 0) ;

  return ;

} ;
