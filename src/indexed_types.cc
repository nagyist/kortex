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

#include <algorithm>

#include <kortex/indexed_types.h>
#include <kortex/check.h>

using namespace std;

namespace kortex {

    inline bool ifloat_cmp_l(const ifloat& l, const ifloat& r) { return l.val > r.val; }
    inline bool ifloat_cmp_s(const ifloat& l, const ifloat& r) { return l.val < r.val; }

    void sort_ascending( vector<ifloat>& arr) {
        sort( arr.begin(), arr.end(), ifloat_cmp_s );
    }

    void sort_descending( vector<ifloat>& arr ) {
        sort( arr.begin(), arr.end(), ifloat_cmp_l );
    }

    void init( const vector<int>& inds, const vector<float>& vals, vector<ifloat>& ifarr ) {
        assert_statement( inds.size() == vals.size(), "size mismatch" );
        ifarr.resize( inds.size() );
        for( unsigned i=0; i<inds.size(); i++ ) {
            ifarr[i].id  = inds[i];
            ifarr[i].val = vals[i];
        }
    }

//
//
//

    inline bool iint_cmp_l(const iint& l, const iint& r) { return l.val > r.val; }
    inline bool iint_cmp_s(const iint& l, const iint& r) { return l.val < r.val; }

    void sort_ascending( vector<iint>& arr) {
        sort( arr.begin(), arr.end(), iint_cmp_s );
    }

    void sort_descending( vector<iint>& arr ) {
        sort( arr.begin(), arr.end(), iint_cmp_l );
    }

    void init( const vector<int>& inds, const vector<int>& vals, vector<iint>& ifarr ) {
        assert_statement( inds.size() == vals.size(), "size mismatch" );
        ifarr.resize( inds.size() );
        for( unsigned i=0; i<inds.size(); i++ ) {
            ifarr[i].id  = inds[i];
            ifarr[i].val = vals[i];
        }
    }


}

