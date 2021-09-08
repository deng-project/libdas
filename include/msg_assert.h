/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: msg_assert.h - message assert macro definition
/// author: Karl-Mihkel Ott


#ifndef __MSG_ASSERT_H
#define __MSG_ASSERT_H

#define msgassert(val, msg) \
{ \
    if(!(val)) { \
        fprintf(stderr, "Assertion failed in %s:%u\n%s\n", __FILE__, __LINE__, std::string(msg).c_str()); \
        exit(1); \
    } \
}


#endif
