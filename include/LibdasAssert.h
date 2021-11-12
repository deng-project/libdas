#ifndef LIBDAS_ASSERT_H
#define LIBDAS_ASSERT_H

#ifdef _DEBUG
    #include <cassert>
    #define LIBDAS_ASSERT(_val) assert(_val)
#else 
    #define LIBDAS_ASSERT(_val)
#endif

#endif
