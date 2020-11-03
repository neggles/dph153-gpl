// { dg-do compile }
// 2001-06-25  Benjamin Kosnik  <bkoz@redhat.com>

// Copyright (C) 2001, 2003 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// 24.5.1 Template class istream_iterator

#include <iterator>
#include <sstream>
#include <testsuite_hooks.h>

void test01()
{
  using namespace std;

  // Check for required base class.
  typedef istream_iterator<long> test_iterator;
  typedef iterator<input_iterator_tag, long, ptrdiff_t, const long*, 
    		   const long&> base_iterator;
  test_iterator  r_it;
  base_iterator* base __attribute__((unused)) = &r_it;
}
