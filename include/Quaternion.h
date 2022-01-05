// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: Quaternion.h - Quaternion structure implementation
// author: Karl-Mihkel Ott

#ifndef QUATERNION_H
#define QUATERNION_H

#ifndef USE_SIMD
#define USE_SIMD
    #include <xmmintrin.h>
    #include <smmintrin.h>
#endif

namespace Libdas {

    /// Fast 3D vector cross product using SIMD instructions
    inline const __m128 FastCross(const __m128 &_vec1, const __m128 &_vec2) {
        __m128 tmp0 = _mm_shuffle_ps(_vec1, _vec1, _MM_SHUFFLE(3, 0, 2, 1));
        __m128 tmp1 = _mm_shuffle_ps(_vec2, _vec2, _MM_SHUFFLE(3, 1, 0, 2));
        __m128 tmp2 = _mm_mul_ps(tmp0, _vec2);
        __m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
        __m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
        return _mm_sub_ps(tmp3, tmp4);
    }


    inline const float FastDot(const __m128 &_vec1, const __m128 &_vec2) {
        __m128 mul_res, shuf_reg, sums_reg;
        mul_res = _mm_mul_ps(_vec1, _vec2);

        shuf_reg = _mm_movehdup_ps(mul_res);
        sums_reg = _mm_add_ps(mul_res, shuf_reg);
        shuf_reg = _mm_movehl_ps(shuf_reg, sums_reg);
        sums_reg = _mm_add_ss(sums_reg, shuf_reg);
        return _mm_cvtss_f32(sums_reg);
    }

    /// Quaternion structure for Libdas
    struct Quaternion {
        float x, y, z, w;

        Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
        Quaternion() : x(0), y(0), z(0), w(0) {}
        Quaternion(float *_a) : x(_a[0]), y(_a[1]), z(_a[2]), w(_a[3]) {}

        ////////////////////////////////////
        // ***** Operator overloads ***** //
        ////////////////////////////////////

        /** 
         * Calculate Grassman product of two quaternions
         */
        inline Quaternion operator*(const Quaternion &_q) {
            const __m128 pw_vec = _mm_set_ps1(w);
            const __m128 qw_vec = _mm_set_ps1(_q.w);
            const __m128 p_vec = _mm_set_ps(0, z, y, x);
            const __m128 q_vec = _mm_set_ps(0, _q.z, _q.y, _q.x);
            const __m128 vec3 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(pw_vec, q_vec), _mm_mul_ps(qw_vec, p_vec)), FastCross(p_vec, q_vec));
            float scalar = w * _q.w - FastDot(p_vec, q_vec);

            Quaternion out;
            _mm_storeu_ps(&out.x, vec3);
            out.w = scalar;
            return out;
        }

        inline Quaternion operator*(const float _c) {
            Quaternion out;
            const __m128 c = _mm_set_ps1(_c);
            const __m128 vec = _mm_set_ps(w, z, y, x);
            _mm_storeu_ps(&out.x, _mm_mul_ps(vec, c));
            return out;
        }

        inline Quaternion operator/(const float _c) {
            Quaternion out;
            const __m128 c = _mm_set_ps1(1 / _c);
            const __m128 vec = _mm_set_ps(w, z, y, x);
            _mm_storeu_ps(&out.x _mm_mul_ps(vec, c));
            return out;
        }

        /**
         * Calculate magnitude of quaternion
         */
        inline float Magnitude() {
            float *ptr = &x;
            
            #pragma omp simd 
            float sum = 0.0f;
            for(int i = 0; i < 4; i++)
                sum += ptr[i] * ptr[i];

            return sqrtf(sum);
        }

#ifdef VECTOR_H
        inline Vector4<float> operator*(const Vector4<float> &_vec) {
            Quaternion vq = &_vec.first;
            vq.w = 0;
            return *this * vq this->Inverse();
        }
#endif

#ifdef MATRIX_H
        // not using simd here for now
        Matrix3<float> ExpandToMatrix3() {
            float dxx = 2 * x * x, dyy = 2 * y * y, dzz = 2 * z * z;
            float dxy = 2 * x * y, dxz = 2 * x * z, 2xw = 2 * x * w;
            float dyz = 2 * y * z, dyw = 2 * y * w;
            float dzw = 2 * z * w;
            return Matrix3<float> {
                {1 - dyy - dzz, dxy + dzw, dxz - dyw},
                {dxy - dzw, 1 - dxx - dzz, dyz + dxw},
                {dxz + dyw, dyz - dxw, 1 - dxx - dyy}
            };
        }


        Matrix4<float> ExpandToMatrix4() {
            float dxx = 2 * x * x, dyy = 2 * y * y, dzz = 2 * z * z;
            float dxy = 2 * x * y, dxz = 2 * x * z, dxw = 2 * x * w;
            float dyz = 2 * y * z, dyw = 2 * y * w;
            float dzw = 2 * z * w;
            return Matrix4<float> {
                {1 - dyy - dzz, dxy + dzw, dxz - dyw, 0},
                {dxy - dzw, 1 - dxx - dzz, dyz + dxw, 0},
                {dxz + dyw, dyz - dxw, 1 - dxx - dyy, 0},
                {0,         0,         0,             1}
            };
        }
#endif

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
