/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: Vector.h - Vector structures header
/// author: Karl-Mihkel Ott

#ifndef VECTOR_H
#define VECTOR_H

#ifndef USE_SIMD
    #include <xmmintrin.h>
#endif

namespace Libdas {

    /**
     * 2D vector structure
     */
    template<typename T>
    struct Vector2 {
#ifdef ITERATORS_H
        typedef VectorIterator<T> iterator;
#endif
        Vector2();
        Vector2(const T &r1, const T &r2);
        Vector2(T &&r1, T &&r2);
        Vector2(const Vector2<T> &val);
        Vector2(Vector2<T>&& val);
        inline void operator=(const Vector2<T> &val);

        T first, second;

        /******************************/
        /***** Operator overloads *****/
        /******************************/

        inline Vector2<T> operator+(const Vector2<T> &_vec) const;
        inline Vector2<T> operator+(const T _c) const;
        inline Vector2<T> operator-(const Vector2<T> &_vec) const;
        inline Vector2<T> operator-(const T _c) const;
        inline Vector2<T> operator*(const T _c) const;
        inline T operator*(const Vector2<T> &_vec) const;
        inline Vector2<T> operator/(const T _c) const;
        inline void operator*=(const T _c);

#ifdef MATRIX_H
        void operator*=(const Matrix2<T> &m); // could be problematic
#endif
        inline void operator+=(const Vector2<T> &_vec);
        inline void operator+=(const T _c);
        inline void operator-=(const Vector2<T> &_vec);
        inline void operator-=(const T _c);
        inline void operator/=(const T _c);
        inline Vector2<T> operator-() const;

        inline bool operator==(const Vector2<T> &_vec) const;
        inline bool operator!=(const Vector2<T> &_vec) const;


        /**
         * Get the total length of the vector (T must be a numeral)
         */
        inline T Magnitude() const;
        /**
         * Normalise the vector coordinates (T must be a numeral)
         */
        inline void Normalise();

#ifdef ITERATORS_H
        /// Iterators
        iterator Begin() const {
            return iterator(const_cast<T*>(&first));
        }


        iterator End() const {
            return iterator(const_cast<T*>(&second + 1));
        }
#endif
    };


    template<typename T>
    Vector2<T>::Vector2() {
        memset(&first, 0, sizeof(T));
        memset(&second, 0, sizeof(T));
    }

    
    template<typename T>
    Vector2<T>::Vector2(const T &r1, const T &r2) {
        first = r1;
        second = r2;
    }


    template<typename T>
    Vector2<T>::Vector2(T &&r1, T &&r2) {
        first = std::move(r1);
        second = std::move(r2);
    }


    template<typename T>
    Vector2<T>::Vector2(const Vector2<T> &val) {
        first = val.first;
        second = val.second;
    }


    template<typename T>
    Vector2<T>::Vector2(Vector2<T> &&val) {
        first = std::move(val.first);
        second = std::move(val.second);
    }


    template<typename T>
    inline void Vector2<T>::operator=(const Vector2<T> &val) {
        first = val.first;
        second = val.second;
    }


    template<typename T>
    inline Vector2<T> Vector2<T>::operator+(const Vector2<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector2<T> out = { first + _vec.first, second + _vec.second };
            return out; 
        }
        
        else return Vector2<T>{};
    }


    template<typename T>
    inline Vector2<T> Vector2<T>::operator+(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector2<T> out = { first + _c, second + _c };
            return out; 
        }
        
        else return Vector2<T>{};
    }


    template<typename T>
    inline Vector2<T> Vector2<T>::operator-(const Vector2<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector2<T> out = { first - _vec.first, second - _vec.second };
            return out;
        }
        
        else return Vector2<T>{};
    }


    template<typename T>
    inline Vector2<T> Vector2<T>::operator-(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector2<T> out = { first - _c, second - _c };
            return out; 
        }
        
        else return Vector2<T>{};
    }


    template<typename T>
    inline T Vector2<T>::operator*(const Vector2<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            T out = (_vec.first * first + _vec.second * second);
            return out;
        }
        
        else return T{};
    }


    template<typename T>
    inline Vector2<T> Vector2<T>::operator*(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector2<T> out = {
                _c * first,
                _c * second,
            };

            return out; 
        }
        
        else return Vector2<T>{};
    }


    template<typename T>
    inline Vector2<T> Vector2<T>::operator/(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector2<T> out = {
                first / _c,
                second / _c
            };

            return _c;
        } 
        
        else return Vector2<T>{};
    }


    template<typename T>
    inline void Vector2<T>::operator*=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first *= _c;
            second *= _c;
        }
    }


    //template<typename T>
    //void Vector2<T>::operator*=(const Matrix2<T> &m) {
        //if(std::is_floating_point<T>::value || std::is_integral<T>::value) 
            //*this = m * (*this);
    //}


    template<typename T>
    inline void Vector2<T>::operator+=(const Vector2<T> &_vec) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first += _vec.first;
            second += _vec.second;
        }
    }


    template<typename T>
    inline void Vector2<T>::operator+=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first += _c;
            second += _c;
        }
    }

    
    template<typename T>
    inline void Vector2<T>::operator-=(const Vector2<T> &_vec) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first -= _vec.first;
            second -= _vec.second;
        }
    }

    
    template<typename T>
    inline void Vector2<T>::operator-=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first -= _c;
            second -= _c;
        }
    }


    template<typename T>
    inline void Vector2<T>::operator/=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first /= _c;
            second /= _c;
        }
    }


    template<typename T>
    inline Vector2<T> Vector2<T>::operator-() const {
        Vector2<T> v;
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            v.first = -first;
            v.second = -second;
        }

        return v;
    }


    template<typename T>
    inline bool Vector2<T>::operator==(const Vector2<T> &_vec) const {
        return first == _vec.first && second == _vec.second; 
    }


    template<typename T>
    inline T Vector2<T>::Magnitude() const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            return (T) sqrt (
                first * first +
                second * second
            );
        }

        else return T{};
    }


    template<typename T>
    inline void Vector2<T>::Normalise() {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            T len = Magnitude();
            first /= len;
            second /= len;
        }
    }


    /**
     * 3D vector structure
     */
    //template<typename T> struct Matrix3;
    template<typename T>
    struct Vector3 {
#ifdef ITERATORS_H
        typedef VectorIterator<T> iterator;
#endif
        Vector3();
        Vector3(const T &r1, const T &r2, const T &r3);
        Vector3(T &&r1, T &&r2, T &&r3);
        Vector3(const Vector3<T> &val);
        Vector3(Vector3<T>&& val);
        inline void operator=(const Vector3<T> &val);

        T first, second, third;
        
        /******************************/
        /***** Operator overloads *****/
        /******************************/

        inline Vector3<T> operator+(const Vector3<T> &_vec) const;
        inline Vector3<T> operator+(const T _c) const;
        inline Vector3<T> operator-(const Vector3<T> &_vec) const;
        inline Vector3<T> operator-(const T _c) const;
        inline T operator*(const Vector3<T> &_vec) const;
        inline Vector3<T> operator*(const T _c) const;
        inline Vector3<T> operator/(const T _c) const;
        inline void operator*=(const T _c);
        //void operator*=(const Matrix3<T> &m);
        inline void operator+=(const Vector3<T> &_vec);
        inline void operator+=(const T _c);
        inline void operator-=(const Vector3<T> &_vec);
        inline void operator-=(const T _c);
        inline void operator/=(const T _c);
        inline Vector3<T> operator-() const;

        inline void operator=(const Vector2<T> &_vec);
        inline bool operator==(const Vector3<T> &_vec) const;
        inline bool operator!=(const Vector3<T> &_vec) const;
        inline bool operator==(const Vector2<T> &_vec) const;
        inline bool operator!=(const Vector2<T> &_vec) const;


        /// Get the current length of the vector
        inline T Magnitude() const;


        /// Normalise the vector to length 1
        inline void Normalise();


        /// Find the crossproduct of two vectors
        inline static Vector3<T> Cross(const Vector3<T> &Vector1, const Vector3<T> &Vector2);

#ifdef ITERATORS_H
        // iterators
        iterator Begin() const {
            return iterator(const_cast<T*>(&first));
        }

        
        iterator End() const {
            return iterator(const_cast<T*>(&third + 1));
        }
#endif
    };

    /*********************************************/
    /********* Vector3 operators and methods ********/
    /*********************************************/

    template<typename T>
    Vector3<T>::Vector3() {
        memset(&first, 0 , sizeof(T));
        memset(&second, 0 , sizeof(T));
        memset(&third, 0 , sizeof(T));
    }


    template<typename T>
    Vector3<T>::Vector3(const T &r1, const T &r2, const T &r3) {
        first = r1;
        second = r2;
        third = r3;
    }


    template<typename T>
    Vector3<T>::Vector3(T &&r1, T &&r2, T &&r3) {
        first = std::move(r1);
        second = std::move(r2);
        third = std::move(r3);
    }


    template<typename T>
    Vector3<T>::Vector3(const Vector3<T> &val) {
        first = val.first;
        second = val.second;
        third = val.third;
    }


    template<typename T>
    Vector3<T>::Vector3(Vector3<T> &&val) {
        first = std::move(val.first);
        second = std::move(val.second);
        third = std::move(val.third);
    }


    template<typename T>
    void Vector3<T>::operator=(const Vector3<T> &val) {
        first = val.first;
        second = val.second;
        third = val.third;
    }


    template<typename T>
    inline Vector3<T> Vector3<T>::operator+(const Vector3<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector3<T> out = {
                first + _vec.first,
                second + _vec.second,
                third + _vec.third
            };
            return out; 
        }

        return Vector3<T>{};
    }


    template<typename T>
    inline Vector3<T> Vector3<T>::operator+(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector3<T> out = {
                first + _c,
                second + _c,
                third + _c 
            };

            return out; 
        }

        return Vector3<T>{};
    }


    template<typename T>
    inline Vector3<T> Vector3<T>::operator-(const Vector3<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector3<T> out = {
                first - _vec.first,
                second - _vec.second,
                third - _vec.third
            };
            return out; 
        }

        return Vector3<T>{};
    }


    template<typename T>
    inline Vector3<T> Vector3<T>::operator-(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector3<T> out = {
                first - _c,
                second - _c,
                third - _c 
            };
            return out; 
        }

        return Vector3<T>{};
    }


    template<typename T>
    inline T Vector3<T>::operator*(const Vector3<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            T out = static_cast<T>(
                first * _vec.first +
                second * _vec.second +
                third * _vec.third
            );

            return out; 
        }

        return T{};
    }


    template<typename T>
    inline Vector3<T> Vector3<T>::operator*(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector3<T> out = {
                _c * first,
                _c * second,
                _c * third,
            };
            return out; 
        }

        return Vector3<T>{};
    }


    template<typename T>
    inline Vector3<T> Vector3<T>::operator/(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector3<T> out = {
                first / _c,
                second / _c,
                third / _c,
            };
            return out; 
        }

        return Vector3<T>{};
    }


    template<typename T>
    inline void Vector3<T>::operator*=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first *= _c;
            second *= _c;
            third *= _c;
        }
    }


    //template<typename T>
    //void Vector3<T>::operator*=(const Matrix3<T> &_mat) {
        //if(std::is_floating_point<T>::value || std::is_integral<T>::value)
            //*this = _mat * (*this);
    //}


    template<typename T>
    inline void Vector3<T>::operator+=(const Vector3<T> &_vec) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first += _vec.first;
            second += _vec.second;
            third += _vec.third;
        }
    }


    template<typename T>
    inline void Vector3<T>::operator+=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first += _c;
            second += _c;
            third += _c;
        }
    }

    
    template<typename T>
    inline void Vector3<T>::operator-=(const Vector3<T> &_vec) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first -= _vec.first;
            second -= _vec.second;
            third -= _vec.third;
        }
    }


    template<typename T>
    inline void Vector3<T>::operator-=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first -= _c;
            second -= _c;
            third -= _c;
        }
    }


    template<typename T>
    inline void Vector3<T>::operator/=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first /= _c;
            second /= _c;
            third /= _c;
        }
    }


    template<typename T>
    inline Vector3<T> Vector3<T>::operator-() const {
        Vector3<T> v;
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            v.first = -first;
            v.second = -second;
            v.third = -third;
        }

        return v;
    }


    template<typename T>
    inline bool Vector3<T>::operator==(const Vector3<T> &_vec) const {
        return first == _vec.first && second == _vec.second && third == _vec.third; 
    }


    template<typename T>
    inline bool Vector3<T>::operator!=(const Vector3<T> &_vec) const {
        return first != _vec.first || second != _vec.second || third != _vec.third;
    }


    template<typename T>
    inline bool Vector3<T>::operator==(const Vector2<T> &_vec) const {
        return first == _vec.first && second == _vec.second;
    }


    template<typename T>
    inline bool Vector3<T>::operator!=(const Vector2<T> &_vec) const {
        return first != _vec.first || second != _vec.second;
    }


    template<typename T>
    inline void Vector3<T>::operator=(const Vector2<T> &_vec) {
        first = _vec.first;
        second = _vec.second;
    }
    

    template<typename T>
    inline T Vector3<T>::Magnitude() const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            return (T) sqrt(first * first + second * second + third * third);
        }

        return T{};
    }


    template<typename T>
    inline void Vector3<T>::Normalise() {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            T len = Magnitude();
            first /= len;
            second /= len;
            third /= len;
        }
    };


    template<typename T>
    inline Vector3<T> Vector3<T>::Cross(const Vector3<T> &Vector1, const Vector3<T> &Vector2) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector3<T> out = Vector3<T>{
                Vector1.second * Vector2.third -
                Vector1.third * Vector2.second,
                Vector1.third * Vector2.first -
                Vector1.first * Vector2.third,
                Vector1.first * Vector2.second -
                Vector1.second * Vector2.first
            };

            return out;
        }

        return Vector3<T>{};
    }


    /**
     * 4D vector structure
     */
    //template<typename T> struct Matrix4;
    template<typename T>
    struct Vector4 {
#ifdef ITERATORS_H
        typedef VectorIterator<T> iterator;
#endif
        Vector4();
        Vector4(const T &r1, const T &r2, const T &r3, const T &r4);
        Vector4(T &&r1, T &&r2, T &&r3, T &&r4);
        Vector4(const Vector4<T> &val);
        Vector4(Vector4<T>&& val);
        void operator=(const Vector4<T> &val);

        T first, second, third, fourth;

        /******************************/
        /***** Operator overloads *****/
        /******************************/

        inline Vector4<T> operator+(const Vector4<T> &_vec) const; 
        inline Vector4<T> operator+(const T _c) const; 
        inline Vector4<T> operator-(const Vector4<T> &_vec) const; 
        inline Vector4<T> operator-(const T _c) const; 
        inline T operator*(const Vector4<T> &_vec) const; 
        inline Vector4<T> operator*(const T _c) const; 
        inline Vector4<T> operator/(const T _c) const; 
        inline void operator*=(const T _c); 
        //inline void operator*=(const Matrix4<T> &m);
        inline void operator+=(const Vector4<T> &_vec); 
        inline void operator+=(const T _c); 
        inline void operator-=(const Vector4<T> &_vec); 
        inline void operator-=(const T _c); 
        inline void operator/=(const T _c); 
        inline Vector4<T> operator-() const;


        inline bool operator==(const Vector4<T> &_vec) const; 
        inline bool operator!=(const Vector4<T> &_vec) const;


        // inline Special structure assignment operators
        inline void operator=(const Vector2<T> &_vec);
        inline void operator=(const Vector3<T> &_vec);


        inline bool operator==(const Vector2<T> &_vec) const;
        inline bool operator!=(const Vector2<T> &_vec) const;
        inline bool operator==(const Vector3<T> &_vec) const;
        inline bool operator!=(const Vector3<T> &_vec) const;


        /// Get the current length of the vector
        const T Magnitude() const;

        
        /// Normalise the vector to length 1
        void Normalise();

        
        /// Find the crossproduct of two vectors
        /// PS! Only first three axes are used
        static const Vector4<T> Cross(const Vector4<T> &_vec1, const Vector4<T> &_vec2); 

#ifdef ITERATORS_H
        // iterators
        iterator Begin() {
            return iterator(const_cast<T*>(&first));
        }


        iterator End() {
            return iterator(const_cast<T*>(&fourth + 1));
        }
#endif
    };


    template<typename T>
    Vector4<T>::Vector4() {
        memset(&first, 0, sizeof(T));
        memset(&second, 0, sizeof(T));
        memset(&third, 0, sizeof(T));
        memset(&fourth, 0, sizeof(T));
    }


    template<typename T>
    Vector4<T>::Vector4(const T &r1, const T &r2, const T &r3, const T &r4) {
        first = r1;
        second = r2;
        third = r3;
        fourth = r4;
    }


    template<typename T>
    Vector4<T>::Vector4(T &&r1, T &&r2, T &&r3, T &&r4) {
        first = std::move(r1);
        second = std::move(r2);
        third = std::move(r3);
        fourth = std::move(r4);
    }


    template<typename T>
    Vector4<T>::Vector4(const Vector4<T> &val) {
        first = val.first;
        second = val.second;
        third = val.third;
        fourth = val.fourth;
    }


    template<typename T>
    Vector4<T>::Vector4(Vector4<T> &&val) {
        first = std::move(val.first);
        second = std::move(val.second);
        third = std::move(val.third);
        fourth = std::move(val.fourth);
    }


    template<typename T>
    inline void Vector4<T>::operator=(const Vector4<T> &val) {
        first = val.first;
        second = val.second;
        third = val.third;
        fourth = val.fourth;
    }


    template<typename T>
    inline Vector4<T> Vector4<T>::operator+(const Vector4<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector4<T> out = {
                first + _vec.first,
                second + _vec.second,
                third + _vec.third
            };
            return out; 
        }

        else return Vector4<T>{};
    }


    template<typename T>
    inline Vector4<T> Vector4<T>::operator+(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector4<T> out = {
                first + _c,
                second + _c,
                third + _c,
                fourth + _c
            };

            return out;
        }

        else return Vector4<T>{};
    }


    template<typename T>
    inline Vector4<T> Vector4<T>::operator-(const Vector4<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector4<T> out = {
                first - _vec.first,
                second - _vec.second,
                third - _vec.third,
                fourth - _vec.fourth
            };

            return out;
        }

        else return Vector4<T>{};
    }


    template<typename T>
    inline Vector4<T> Vector4<T>::operator-(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector4<T> out = {
                first - _c,
                second - _c,
                third - _c,
                fourth - _c
            };

            return out;
        }

        else return Vector4<T>{};
    }


    template<typename T>
    inline T Vector4<T>::operator*(const Vector4<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            T out = (first * _vec.first + second * _vec.second + third * _vec.third + fourth * _vec.fourth);
            return out; 
        }

        else return T{};
    }


    template<typename T>
    inline Vector4<T> Vector4<T>::operator*(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector4<T> out = {
                first * _c,
                second * _c,
                third * _c,
                fourth * _c
            };

            return out; 
        }

        else return Vector4<T>{};
    }


    template<typename T>
    inline Vector4<T> Vector4<T>::operator/(const T _c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector4<T> out = {
                first / _c,
                second / _c,
                third / _c,
                fourth / _c
            };

            return out; 
        }

        else return Vector4<T>{};
    }


    template<typename T>
    inline void Vector4<T>::operator*=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first *= _c;
            second *= _c;
            third *= _c;
            fourth *= _c;
        }
    }


    //template<typename T>
    //inline void Vector4<T>::operator*=(const Matrix4<T> &_mat) {
        //if(std::is_floating_point<T>::value || std::is_integral<T>::value)
            //*this = _mat * (*this);
    //}


    template<typename T>
    inline void Vector4<T>::operator+=(const Vector4<T> &_vec) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first += _vec.first;
            second += _vec.second;
            third += _vec.third;
            fourth += _vec.fourth;
        }
    }


    template<typename T>
    inline void Vector4<T>::operator+=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first += _c;
            second += _c;
            third += _c;
            fourth += _c;
        }
    }


    template<typename T>
    inline void Vector4<T>::operator-=(const Vector4<T> &_vec) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first -= _vec.first;
            second -= _vec.second;
            third -= _vec.third;
            fourth -= _vec.fourth;
        }
    }


    template<typename T>
    inline void Vector4<T>::operator-=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first -= _c;
            second -= _c;
            third -= _c;
            fourth -= _c;
        }
    }


    template<typename T>
    inline void Vector4<T>::operator/=(const T _c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            first /= _c;
            second /= _c;
            third /= _c;
            fourth /= _c;
        }
    }


    template<typename T>
    inline Vector4<T> Vector4<T>::operator-() const {
        Vector4<T> v;
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            v.first = -first;
            v.second = -second;
            v.third = -third;
            v.fourth = -fourth;
        }

        return v;
    }


    template<typename T>
    inline bool Vector4<T>::operator==(const Vector4<T> &_vec) const { 
        return first == _vec.first && second == _vec.second && third == _vec.third && fourth == _vec.fourth;
    }


    template<typename T>
    inline bool Vector4<T>::operator!=(const Vector4<T> &_vec) const {
        return first != _vec.first || second != _vec.second || third != _vec.third || fourth != _vec.fourth;
    }


    template<typename T>
    inline void Vector4<T>::operator=(const Vector2<T> &_vec) {
        first = _vec.first;
        second = _vec.second;
    }


    template<typename T>
    inline void Vector4<T>::operator=(const Vector3<T> &_vec) {
        first = _vec.first;
        second = _vec.second;
        third = _vec.third;
    }


    template<typename T>
    inline bool Vector4<T>::operator==(const Vector3<T> &_vec) const {
        return first == _vec.first && second == _vec.second && third == _vec.third;
    }


    template<typename T>
    inline bool Vector4<T>::operator!=(const Vector3<T> &_vec) const {
        return first != _vec.first || second != _vec.second || third != _vec.third;
    }


    template<typename T>
    inline bool Vector4<T>::operator==(const Vector2<T> &_vec) const {
        return first == _vec.first && second == _vec.second;
    }


    template<typename T>
    inline bool Vector4<T>::operator!=(const Vector2<T> &_vec) const {
        return first != _vec.first || second != _vec.second;
    }


    template<typename T>
    const T Vector4<T>::Magnitude() const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            return (T) sqrt (
                first * first +
                second * second +
                third * third +
                fourth * fourth
            );
        }

        else return T{};
    }


    template<typename T>
    void Vector4<T>::Normalise() {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            T len = Magnitude();
            first /= len;
            second /= len;
            third /= len;
            fourth /= len;
        }
    }


    template<typename T>
    const Vector4<T> Vector4<T>::Cross(const Vector4<T> &_vec1, const Vector4<T> &_vec2) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector4<T> out = (Vector4<T>) {
                _vec1.second * _vec2.third -
                _vec1.third * _vec2.second,
                _vec1.third * _vec2.first -
                _vec1.first * _vec2.third,
                _vec1.first * _vec2.second -
                _vec1.second * _vec2.first,
                0.0f
            };

            return out;
        }

        else return Vector4<T>{};
    }
}

#endif
