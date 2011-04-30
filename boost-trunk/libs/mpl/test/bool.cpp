
// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License,Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: bool.cpp 49240 2008-10-10 09:21:07Z agurtovoy $
// $Date: 2008-10-10 02:21:07 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49240 $

#include <boost/mpl/bool.hpp>
#include <boost/mpl/aux_/test.hpp>

#include <cassert>

#define BOOL_TEST(c) \
    { MPL_ASSERT(( is_same< bool_<c>::value_type, bool > )); } \
    { MPL_ASSERT(( is_same< bool_<c>, c##_ > )); } \
    { MPL_ASSERT(( is_same< bool_<c>::type, bool_<c> > )); } \
    { MPL_ASSERT_RELATION( bool_<c>::value, ==, c ); } \
    BOOST_TEST( bool_<c>() == c ); \
/**/

MPL_TEST_CASE()
{
    BOOL_TEST(true)
    BOOL_TEST(false)
}
