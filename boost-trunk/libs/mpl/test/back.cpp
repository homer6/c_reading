
// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License,Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: back.cpp 49240 2008-10-10 09:21:07Z agurtovoy $
// $Date: 2008-10-10 02:21:07 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49240 $

#include <boost/mpl/back.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/aux_/test.hpp>

template< typename Seq, int value > struct back_test
{
    typedef typename back<Seq>::type t;
    MPL_ASSERT_RELATION( t::value, ==, value );
};

MPL_TEST_CASE()
{
    back_test< range_c<int,0,1>, 0 >();
    back_test< range_c<int,0,10>, 9 >();
    back_test< range_c<int,-10,0>, -1 >();
}
