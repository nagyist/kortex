// ---------------------------------------------------------------------------
//
// This file is part of the <kortex> library suite
//
// Copyright (C) 2013 Engin Tola
//
// See licence.txt file for licence information.
//
// author: Engin Tola
// e-mail: engintola@gmail.com
//
// ---------------------------------------------------------------------------
#ifndef KORTEX_ROTATION_H
#define KORTEX_ROTATION_H

namespace kortex {

    /// computes the rotation matrix that rotates na to nb
    void rotate_normal_to_normal( const double* na, const double* nb, double* Rab );

    void  axisangle_to_quaternion( const double* aa, double* q );
    void quaternion_to_rotation  ( const double* q,  double* R );
    void  axisangle_to_rotation  ( const double* aa, double* R );

    void rotation_matrix_around_z( const double& angle_in_degrees, double R[9] );
    void euler_to_rotation( double theta, double phi, double psi, double * R );

    // Extracting Euler Angles from a Rotation Matrix - returns in degrees
    void rotation_to_euler( const  double* R, double& theta, double &phi, double& psi );

    // azimuth and elevation is in DEGREES
    // azimuth  : angle from the positive x-axis to the normal [ -180 , 180 ]
    // elevation: angle from the xy-plane to the normal        [ - 90 ,  90 ]
    void azel_to_cartesian( double az, double el, double n[3] );
    void cartesian_to_azel( const double n[3], double& az, double& el );

    void construct_local_coordinate_frame(const double* z_normal, double* new_u, double* new_v);
    void rotation_to_az_el_zeta( const double R[9], double& az, double& el, double& zeta );

    void rotation_matrix_frame_error( const double* R_ref, const double* R_obs,
                                      double& e_boresight, double& e_inplain );

}

#endif