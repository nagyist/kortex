// ---------------------------------------------------------------------------
//
// This file is part of the <kortex> library suite
//
// Copyright (C) 2013 Engin Tola
//
// See LICENSE file for license information.
//
// author: Engin Tola
// e-mail: engintola@gmail.com
// web   : http://www.engintola.com
//
// ---------------------------------------------------------------------------

#include <kortex/kmatrix.h>
#include <kortex/matrix.h>
#include <kortex/fileio.h>
#include <kortex/check.h>
#include <kortex/message.h>
#include <kortex/math.h>

#include <cstring>
#include <iomanip>

using std::endl;

namespace kortex {

    void KMatrix::init_() {
        m_ro_data = NULL;
        m_data    = NULL;
        m_wrapper = false;
        m_const   = false;
        nr        = 0;
        nc        = 0;
    }

    KMatrix::KMatrix() {
        init_();
    }

    KMatrix::KMatrix( const double* data, int h, int w ) {
        init_();
        m_ro_data = data;
        m_wrapper = true;
        m_const   = true;
        nr = h;
        nc = w;
    }

    KMatrix::KMatrix( const KMatrix& rhs ) {
        release();
        copy( rhs );
    }

    KMatrix::KMatrix( double* data, int h, int w ) {
        init_();
        m_data    = data;
        m_wrapper = true;
        m_const   = false;
        nr        = h;
        nc        = w;
    }

    KMatrix::KMatrix( int h, int w ) {
        init_();
        init( h, w );
    }

    void KMatrix::copy( const KMatrix& rhs ) {
        passert_statement( !is_const(), "cannot copy onto const matrix" );
        resize( rhs.h(), rhs.w() );
        memcpy( m_data, rhs.get_const_pointer(), sizeof(*m_data)*rhs.size() );
    }

    void KMatrix::clone( const KMatrix& rhs ) {
        passert_statement( !is_wrapper(), "cannot clone onto wrapper matrix" );
        if( rhs.is_wrapper() ) {
            m_data    = rhs.m_data;
            m_ro_data = rhs.m_ro_data;
            m_const   = rhs.m_const;
            nr        = rhs.h();
            nc        = rhs.w();
            m_wrapper = true;
        } else {
            this->copy( rhs );
        }
    }

    void KMatrix::init( int h, int w ) {
        passert_statement( !is_wrapper(), "call release() on the wrapped matrix first" );
        m_memory.resize( req_mem(h,w) );
        m_ro_data = NULL;
        m_data    = (double*)m_memory.get_buffer();
        m_wrapper = false;
        m_const   = false;
        nr        = h;
        nc        = w;
    }

    void KMatrix::reshape( int h, int w ) {
        passert_statement( h*w == size(), "matrix size should not change" );
        nr = h;
        nc = w;
    }

    void KMatrix::resize( int h, int w ) {
        if( is_wrapper() ) {
            passert_statement( h*w == this->size(), "wrapped matrix has insufficient space" );
            nr = h;
            nc = w;
        } else {
            init( h, w );
        }
    }

    void KMatrix::expand( int h, int w ) {
        passert_statement( is_wrapper(), "cannot expand wrapped matrix" );
        m_memory.expand( req_mem(h,w) );
        m_ro_data = NULL;
        m_data = (double*)m_memory.get_buffer();
        m_wrapper = false;
        m_const   = false;
        nr        = h;
        nc        = w;
    }

    void KMatrix::reset() {
        init_();
    }

    void KMatrix::release() {
        m_memory.deallocate();
        init_();
    }

    KMatrix::~KMatrix() {
        release();
    }

    void KMatrix::set( const double* data, int dsz ) {
        passert_statement( !is_const(), "cannot modify const matrix" );
        passert_statement( dsz == size(), "data size should be equal to the matrix size" );
        assert_pointer( data );
        memcpy( m_data, data, sizeof(*m_data)*dsz );
    }

    void KMatrix::set_row( int rid, const double* rdata, int rdsz ) {
        assert_boundary( rid, 0, nr );
        assert_pointer( rdata );
        assert_statement( rdsz == nc, "rdsz should be same with matrix size" );
        mat_set_row( get_pointer(), nr, nc, rid, rdata, rdsz );

    }
    void KMatrix::set_row( int r0, int c0, const double* row_data, int csz ) {
        mat_set_row_from_point( get_pointer(), nr, nc, r0, c0, row_data, csz );
    }
    void KMatrix::set_row( int r, const double* rdata, int rsz, double alpha ) {
        mat_set_row( get_pointer(), nr, nc, r, alpha, rdata, rsz );
    }

    void KMatrix::set_col( int cid, const double* cdata, int csz ) {
        mat_set_col( get_pointer(), nr, nc, cid, cdata, csz );
    }
    void KMatrix::set_col( int r0, int c0, const double* cdata, int csz ) {
        mat_set_col_from_point( get_pointer(), nr, nc, r0, c0, cdata, csz );
    }
    void KMatrix::set_col( int c, const double* cdata, int csz, double alpha ) {
        mat_set_col( get_pointer(), nr, nc, c, alpha, cdata, csz );
    }


    double* KMatrix::get_pointer() {
        passert_statement( !is_const(), "cannot return non-const pointer for const matrix" );
        assert_statement( size() != 0, "matrix has not been initialized" );
        return m_data;
    }

    const double* KMatrix::get_const_pointer() const {
        assert_statement( size() != 0, "matrix has not been initialized" );
        switch( m_const ) {
        case true : return m_ro_data;
        case false: return m_data;
        }
    }

    double* KMatrix::get_row( int rid ) {
        assert_boundary( rid, 0, nr );
        return get_pointer() + rid * nc;
    }
    double* KMatrix::get_col( int cid ) {
        assert_boundary( cid, 0, nc );
        return get_pointer() + cid;
    }

    const double* KMatrix::get_row( int rid ) const {
        assert_boundary( rid, 0, nr );
        return get_const_pointer() + rid * nc;
    }
    const double* KMatrix::get_col( int cid ) const {
        assert_boundary( cid, 0, nc );
        return get_const_pointer() + cid;
    }

    /// copies  this[ dr:dr+srsz, dc:dc+srsz ] =  rhs[ sr:sr+srsz, sc:sc+scsz ]
    void KMatrix::copy( const KMatrix& rhs, int sr, int sc, int srsz, int scsz, int dr, int dc ) {
        assert_statement( sr+srsz <= rhs.h(), "size oob" );
        assert_statement( sc+scsz <= rhs.w(), "size oob" );
        assert_statement( dr+srsz <= h(), "size oob" );
        assert_statement( dc+scsz <= w(), "size oob" );
        for( int r=0; r<srsz; r++ ) {
            const double* srow =   rhs.get_row( sr+r );
            double      * drow = this->get_row( dr+r );
            memcpy( drow+dc, srow+sc, sizeof(*drow)*scsz );
        }
    }

    void KMatrix::copy( const KMatrix& rhs, int dr, int dc ) {
        assert_statement( dr+rhs.h() <= h(), "size oob" );
        assert_statement( dc+rhs.w() <= w(), "size oob" );
        for( int r=0; r<rhs.h(); r++ ) {
            const double* srow =   rhs.get_row( r    );
            double      * drow = this->get_row( dr+r );
            memcpy( drow+dc, srow, sizeof(*drow)*rhs.w() );
        }
    }

    void KMatrix::identity() {
        passert_statement( !is_const(), "cannot modify const matrix" );
        assert_statement( size() != 0, "matrix has not been initialized" );
        mat_identity( m_data, nr, nc );
    }

    void KMatrix::zero() {
        passert_statement( !is_const(), "cannot modify const wrapped matrix" );
        assert_statement( size() != 0, "matrix has not been initialized" );
        mat_zero( m_data, nr, nc );
    }

    void KMatrix::negate() {
        assert_statement( size() != 0, "matrix has not been initialized" );
        mat_negate( get_pointer(), nr, nc );
    }

    double KMatrix::normalize() {
        assert_statement( size() != 0, "matrix has not been initialized" );
        return mat_normalize( get_pointer(), nr, nc );
    }

    double KMatrix::trace() const {
        return mat_trace( get_const_pointer(), nr, nc );
    }

    double KMatrix::det3() const {
        passert_statement( nr == nc && nr == 3, "invalid matrix size" );
        return mat_det_3( get_const_pointer(), nr );
    }

    void KMatrix::transpose() {
        passert_statement( !is_const(), "cannot modify const matrix" );
        if( nr == nc ) {
            for( int y=0; y<nr; y++ ) {
                double* row = get_row(y);
                double* col = get_col(y);
                for( int x=y+1; x<nc; x++ ) {
                    std::swap( row[x], col[x*nc] );
                }
            }
        } else {
            KMatrix tmp( nc, nr );
            mat_transpose( get_const_pointer(), nr, nc, tmp.get_pointer(), tmp.size() );
            this->copy( tmp );
        }

    }

    void KMatrix::scale_row( int rid, double alpha ) {
        assert_boundary( rid, 0, nr );
        assert_statement( !is_const(), "cannot modify const matrix" );
        double* row = get_row(rid);
        for( int i=0; i<nc; i++ )
            row[i] *= alpha;
    }

    void KMatrix::scale( double s ) {
        assert_statement( !is_const(), "cannot modify const matrix" );
        double* data = get_pointer();
        for( int i=0; i<size(); i++ )
            data[i] *= s;
    }

    void KMatrix::scale_col( int cid, double alpha ) {
        assert_boundary( cid, 0, nc );
        assert_statement( !is_const(), "cannot modify const matrix" );
        double* col = get_col(cid);
        for( int i=0; i<nr; i++ )
            col[i*nc] *= alpha;
    }

    // frobenius norm
    double KMatrix::norm() const {
        return mat_norm( get_const_pointer(), nr, nc );
    }

    double KMatrix::norm_sq() const {
        return mat_norm_sq( get_const_pointer(), nr, nc );
    }

    void KMatrix::print( const string& str ) const {
        if( size() == 0 ) {
            printf( "%s = []\n", str.c_str() );
            return;
        }
        matrix_print( str.c_str(), get_const_pointer(), nr, nc, false, true );
    }

    void KMatrix::save( const string& file ) const {
        ofstream fout;
        open_or_fail( file, fout );
        save( fout, "X" );
        fout.close();
    }
    void KMatrix::save( ofstream& fout, const string& mat_name ) const {
        fout << mat_name.c_str() << " " << nr << " " << nc << " ";
        for( int r=0; r<nr; r++ ) {
            const double* row = get_row( r );
            for( int c=0; c<nc; c++ ) {
                fout << std::setprecision(24);
                fout << row[c] << " ";
            }
        }
        fout << endl;
        check_file_stream_error( fout );
    }

    void KMatrix::save_binary( ofstream& fout ) const {
        write_bparam( fout, nr );
        write_bparam( fout, nc );
        write_bparam( fout, get_const_pointer(), size() );
    }

    void KMatrix::load_binary( ifstream& fin ) {
        passert_statement( !is_const(), "cannot modify const matrix" );

        int th, tw;
        read_bparam( fin, th );
        read_bparam( fin, tw );

        if( m_wrapper ) {
            if( th*tw != size() )
                logman_fatal( "wrapped matrix has an invalid size" );
            nr = th;
            nc = tw;
            read_bparam( fin, get_pointer(), th*tw );
        } else {
            init( th, tw );
            read_bparam( fin, get_pointer(), th*tw );
        }
    }

    void KMatrix::load( const string& file ) {
        passert_statement( !is_const(), "cannot modify const matrix" );
        ifstream fin;
        open_or_fail( file, fin );
        load( fin, "X" );
        fin.close();
    }

    void KMatrix::load( ifstream& fin, const string& mat_name ) {
        passert_statement( !is_const(), "cannot modify const matrix" );
        string stmp;
        read_string( fin, stmp, mat_name.c_str() );
        int th, tw;
        fin >> th >> tw;

        if( m_wrapper ) {
            if( th*tw != size() )
                logman_fatal( "wrapped matrix has invalid size" );
            nr = th;
            nc = tw;
        } else {
            init( th, tw );
        }
        for( int r=0; r<nr; r++ ) {
            double* row = get_row( r );
            for( int c=0; c<nc; c++ ) {
                fin >> row[c];
            }
        }
        check_file_stream_error( fin );
    }

//
//
//

    void mat_row_add( const KMatrix& A, int arid, double alpha,
                      const KMatrix& B, int brid, double beta,
                      KMatrix&       C, int crid ) {
        int nc = A.w();
        assert_statement( (B.w() == nc) && (C.w() == nc),
                          "invalid matrix size" );
        assert_boundary( arid, 0, A.h() );
        assert_boundary( brid, 0, B.h() );
        assert_boundary( crid, 0, C.h() );
        assert_number( alpha );
        assert_number( beta  );
        const double* arow = A.get_row( arid );
        const double* brow = B.get_row( brid );
        double      * crow = C.get_row( crid );

        for( int i=0; i<nc; i++ ) {
            crow[i] = alpha*arow[i] + beta * brow[i];
        }
    }

    void mat_Ax_plus_y( const KMatrix& A, const KMatrix& x, const KMatrix& y, KMatrix& c ) {
        assert_statement( ( x.w()==1     ) && ( y.w()==1     ), "x and y should be column vectors" );
        assert_statement( ( A.h()==x.h() ) && ( A.h()==y.h() ), "invalid matrix dimensions" );
        c.init( A.h(), 1 );
        double      * cp = c.get_pointer();
        const double* yp = y.get_const_pointer();
        const double* xp = x.get_const_pointer();
        for( int r=0; r<A.h(); r++ ) {
            cp[r] = dot( A.get_row(r), xp, A.w() ) + yp[r];
        }
    }


    /// c = alpha * A * x + beta * y; x and y are column vectors.
    void mat_alpha_Ax_plus_beta_y( const double& alpha, const KMatrix& A, const KMatrix& x,
                                   const double& beta,  const KMatrix& y, KMatrix& c ) {
        assert_statement( ( x.w()==1     ) && ( y.w()==1     ), "x and y should be column vectors" );
        assert_statement( ( A.h()==x.h() ) && ( A.h()==y.h() ), "invalid matrix dimensions" );

        c.resize( A.h(), 1 );
        double      * cp = c.get_pointer();
        const double* yp = y.get_const_pointer();
        const double* xp = x.get_const_pointer();
        for( int r=0; r<A.h(); r++ ) {
            cp[r] = alpha * dot( A.get_row(r), xp, A.w() ) + beta * yp[r];
        }

    }

    void mat_col_copy( const KMatrix& src, int sc, double alpha, KMatrix& dst, int dc ) {
        assert_boundary( dc, 0, dst.w() );
        assert_boundary( sc, 0, src.w() );
        assert_statement( dst.h() == src.h(), "dst src heights differ" );

        const double* src_col = src.get_col(sc);
        double      * dst_col = dst.get_col(dc);

        for( int i=0; i<dst.h(); i++ ) {
            dst_col[i*dst.w()] = alpha * src_col[i*src.w()];
        }
    }

    /// A = R*Q:
    /// R: upper triangular, Q: rotation matrix
    bool rq_givens_decomposition_3( const KMatrix& A, KMatrix& R, KMatrix& Q ) {
        assert_statement( A.is_square() && A.h() == 3, "invalid input A" );

        // set A(2,1) <-- 0 by Qx
        double r = sqrt( sq(A(2,1)) + sq( A(2,2) ) );
        double c = -A(2,2)/r;
        double s =  A(2,1)/r;

        if( !is_a_number( s*c ) ) return false;

        KMatrix Qx(3,3);
        const double qx_data[] = { 1., 0., 0., 0., c, -s, 0., s, c };
        Qx.set( qx_data, 9 );

        KMatrix AQx;
        mat_mat( A, Qx, AQx );

        // set A(2,0) <-- 0
        r = sqrt( sq(AQx(2,2)) + sq(AQx(2,0)) );
        c = AQx(2,2)/r;
        s = AQx(2,0)/r;
        if( !is_a_number( s*c ) ) return false;

        KMatrix Qy(3,3);
        const double qy_data[] = { c, 0., s, 0., 1., 0., -s, 0., c };
        Qy.set( qy_data, 9 );

        KMatrix AQxQy;
        mat_mat( AQx, Qy, AQxQy );

        // set A(1,0) <-- 0
        r = sqrt( sq(AQxQy(1,0)) + sq(AQxQy(1,1)) );
        c = -AQxQy(1,1) / r;
        s =  AQxQy(1,0) / r;
        if( !is_a_number( s*c ) ) return false;

        KMatrix Qz(3,3);
        const double qz_data[] = { c, -s, 0., s, c, 0., 0., 0., 1. };
        Qz.set( qz_data, 9 );

        mat_mat( AQxQy, Qz, R );
        mat_mat_mat( Qx, Qy, Qz, Q );

        Q.transpose();

        KMatrix Ar(3,3);
        mat_mat(R,Q,Ar);

        for(int i=0; i<9; i++) {
            if( fabs(A[i]) < 1e-15 )
                continue;
            if( fabs( (A[i]-Ar[i])/A[i] ) > 1e-3 )
                return false;
        }

        return true;
    }




}
