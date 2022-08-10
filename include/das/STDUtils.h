// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: STDUtils.h - Comparison classes for std ADTs 
// author: Karl-Mihkel Ott

#ifndef STD_UTILS_H
#define STD_UTILS_H

namespace Libdas {

    template <typename _T1, typename _T2>
    struct FirstLess {
        typedef std::pair<_T1, _T2> Pair;
        bool operator()(const Pair &_p1, const Pair &_p2) {
            return _p1.first < _p2.first;
        }
    };


    template <typename _T1, typename _T2>
    struct FirstGreater {
        typedef std::pair<_T1, _T2> Pair;
        bool operator()(const Pair &_p1, const Pair &_p2) {
            return _p1.first > _p2.first;
        }
    };
}

#endif
