// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: Quaternion.h - Quaternion structure implementation
// author: Karl-Mihkel Ott

#ifndef QUATERNION_H
#define QUATERNION_H

#include <cmath>
#include <Vector.h>

namespace Libdas {

    /// Quaternion structure for Libdas
    struct Quaternion {
        float x, y, z, w;

        Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
        Quaternion() : x(0), y(0), z(0), w(0) {}

        ////////////////////////////////////
        // ***** Operator overloads ***** //
        ////////////////////////////////////

        /** 
         * Calculate Grassman product of two quaternions
         */
        Quaternion operator*(const Quaternion &_q) {
            Vector3<float> vec = Vector3<float>(_q.x, _q.y, _q.z) * w + Vector3<float>(x, y, z) * _q.w + Vector3<float>::Cross(Vector3<float>(x, y, z), Vector3<float>(_q.x, _q.y, _q.z));
            float scalar = w * _q.w - Vector3<float>(x, y, z) * Vector3<float>(_q.x, _q.y, _q.z);
            return Quaternion(vec.first, vec.second, vec.third, scalar);
        }

        inline Quaternion operator*(const float _c) {
            return Quaternion(x * _c, y * _c, z * _c, w * _c);
        }

        inline Quaternion operator/(const float _c) {
            return Quaternion(x / _c, y / _c, z / _c, w / _c);
        }

        /**
         * Calculate magnitude of quaternion
         */
        inline float Magnitude() {
            return sqrtf(x * x + y * y + z * z + w * w);
        }

        /**
         * Calculate the conjugate of quaternion
         */
        inline Quaternion Conjugate() {
            return Quaternion(-x, -y, -z, w);
        }

        /**
         * Calculate the inverse of quaternion using formula: q* / |q|
         */
        inline Quaternion Inverse() {
            return Conjugate() / Magnitude();
        }
    };
}

#endif
