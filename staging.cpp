#include "basis.h"
#include "common.h"
#include "constants.h"
#include <Eigen/Dense>
#include "hubbard.h"
#include <iostream>
#include "nbodyint.h"
#include "obarasaika.h"
#include "qtzcntrl.h"
#include "r12.h"
#include "solver.h"
#include "staging.h"
#include "tei.h"
#include "util.h"
#include <vector>

void hubbard_hamiltonian( common& com){
/*
  Build everything we need to do Hubbard Hamiltonian calculations.

  Let's Start with a dimension of nx1 periodic
*/
  int nx = com.hub_n( 0) ;
  Eigen::MatrixXd T( nx, nx) ;
  tei g ;
  std::vector<tei> intarr ;

/*
  Set the Core Hamiltonian
*/

  T.setZero() ;

  for ( int i=0; i < nx-1; i++){
    T( i, i+1) = -d1 ;
    T( i+1, i) = -d1 ;
    } ;

  T( 0, nx-1) = -d1 ;
  T( nx-1, 0) = -d1 ;

  com.setH( T) ;
  g.set( 0, 0, 0, 0, com.getU()) ;
  intarr.push_back( g) ;
  com.setr12( intarr) ;

  T.resize( 0, 0) ;

  return ;

} ;

void molecular_hamiltonian( common& com){
/*
  Build everything we need to do molecular Hamiltonian calculations.
*/
  int nbas, natm = com.natm() ;
  basis_set b ;
  Eigen::MatrixXd c ( natm, 3), ns ( natm, 3) ;
  Eigen::VectorXd a ( natm) ;
  Eigen::MatrixXd S, T, V ;
  Eigen::MatrixXcd cV, h_fc ;
  std::vector<tei> intarr ;

  nnrep( com, natm, c, a) ;
  b = build_basis( com, a, c) ;
  nbas = b.nbas ;
  com.nbas( nbas) ;
  S.resize( nbas, nbas) ;
  T.resize( nbas, nbas) ;
  V.resize( nbas, nbas) ;
  cV.resize( nbas, nbas) ;
  h_fc.resize( 2*nbas, 2*nbas) ;
  ao_overlap( natm, b, S) ;
  com.setS( S) ;
  /*
    I also have cannonical orthogonalization
  */
  symort( S, cV) ;
  T = cV.real() ;
  com.setXS( T) ;
  T.setZero() ;
  V.setZero() ;
  ao_kinetic( natm, b, T) ;
  ao_eN_V( natm, b, c, a, V) ;
  S = T + V ;
  com.setH( S) ;
  /*
    Generate our FC terms
  */
  ns = com.getNS() ;
  fc_hamiltonian ( h_fc, b, c, ns) ;
  com.setFC( h_fc) ;
  /*
    Currently this only supports a linear symmetrized list of tei
  */
  list_ao_tei( com, b, intarr) ;
  com.setr12( intarr) ;

  h_fc.resize( 0, 0) ;
  cV.resize( 0, 0) ;
  V.resize( 0, 0) ;
  T.resize( 0, 0) ;
  S.resize( 0, 0) ;
  a.resize( 0) ;
  ns.resize( 0, 0) ;
  c.resize( 0, 0) ;

  return ;

} ;

void nnrep( common& com) {
/*
  Get the nuclear-nuclear repulsion
*/
  int i, j, natm = com.natm() ;
  double cx = d0, cy = d0, cz = d0 ;
  double r2 = d0, n_rep = d0 ;
  Eigen::MatrixXd c ( natm, 3) ;
  Eigen::VectorXd a ( natm) ;
  c = com.getC() ;
  a = com.getA() ;

  for ( i=0; i < natm; i++){
    for ( j=i+1; j < natm; j++){
      cx = c( j, 0) - c( i, 0) ;
      cy = c( j, 1) - c( i, 1) ;
      cz = c( j, 2) - c( i, 2) ;
      r2 = std::pow( cx, d2) + std::pow( cy, d2) + std::pow( cz, d2) ;
      n_rep += a(i)*a(j)/std::sqrt( r2) ;
      }
    }
 
  com.nrep( n_rep ) ;

  std::cout << " Nuclear repulsion is " << n_rep << std::endl ;

  a.resize( 0) ;
  c.resize( 0, 0) ;

  return ;

} ;

void nnrep( common& com, int& natm, Eigen::Ref<Eigen::MatrixXd> c, Eigen::Ref<Eigen::VectorXd> a ) {
/*
  Get the nuclear-nuclear repulsion
*/
  int i, j ;
  double cx = d0, cy = d0, cz = d0 ;
  double r2 = d0, n_rep = d0 ;
  c = com.getC() ;
  a = com.getA() ;

  for ( i=0; i < natm; i++){
    for ( j=i+1; j < natm; j++){
      cx = c( j, 0) - c( i, 0) ;
      cy = c( j, 1) - c( i, 1) ;
      cz = c( j, 2) - c( i, 2) ;
      r2 = std::pow( cx, d2) + std::pow( cy, d2) + std::pow( cz, d2) ;
      n_rep += a(i)*a(j)/std::sqrt( r2) ;
      }
    }
 
  com.nrep( n_rep ) ;

  std::cout << " Nuclear repulsion is " << n_rep << std::endl ;

  return ;

} ;

void pairing_hamiltonian( common& com){
/*
  Build everything we need to do pairing Hamiltonian calculations.
*/
  int nbas = com.nbas() ;
  Eigen::MatrixXd T( nbas, nbas) ;
  tei g ;
  std::vector<tei> intarr ;
/*
  Set the Core Hamiltonian
*/
  T.setZero() ;
  for ( int i=0; i < nbas; i++){
    T( i, i) = i+1 ;
    } ;
  com.setH( T) ;
  g.set( 0, 0, 0, 0, com.getU()) ;
  intarr.push_back( g) ;
  com.setr12( intarr) ;

  T.resize( 0, 0) ;

  return ;

} ;
