// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: LibdasAssert.h - assertion macro definitions for libdas project
// author: Karl-Mihkel Ott

#ifndef LIBDAS_ASSERT_H
#define LIBDAS_ASSERT_H

#ifdef __DEBUG
    #include <cassert>
    #define LIBDAS_ASSERT(_val) assert(_val)
#else 
    #define LIBDAS_ASSERT(_val)
#endif

#endif
