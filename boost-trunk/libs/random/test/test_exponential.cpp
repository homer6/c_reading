/* test_exponential.cpp
 *
 * Copyright Steven Watanabe 2011
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id: test_exponential.cpp 68372 2011-01-22 21:44:25Z steven_watanabe $
 *
 */

#include <boost/random/exponential_distribution.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/math/distributions/exponential.hpp>

#define BOOST_RANDOM_DISTRIBUTION boost::random::exponential_distribution<>
#define BOOST_RANDOM_DISTRIBUTION_NAME exponential
#define BOOST_MATH_DISTRIBUTION boost::math::exponential
#define BOOST_RANDOM_ARG1_TYPE double
#define BOOST_RANDOM_ARG1_NAME lambda
#define BOOST_RANDOM_ARG1_DEFAULT 1000.0
#define BOOST_RANDOM_ARG1_DISTRIBUTION(n) boost::uniform_real<>(0.0001, n)

#include "test_real_distribution.ipp"
