
#ifndef BOOST_FSM_EVENT_INCLUDED
#define BOOST_FSM_EVENT_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: event.hpp 49240 2008-10-10 09:21:07Z agurtovoy $
// $Date: 2008-10-10 02:21:07 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49240 $

#include "base_event.hpp"

namespace fsm { namespace aux {

template< typename Derived >
struct event
    : base_event
{
 public:
    typedef base_event base_t;

 private:
    virtual std::auto_ptr<base_event> do_clone() const
    {
        return std::auto_ptr<base_event>(
              new Derived(static_cast<Derived const&>(*this))
            );
    }
};

}}

#endif // BOOST_FSM_EVENT_INCLUDED
