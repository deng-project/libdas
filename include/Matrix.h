/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: Matrix.h - Matrix structures header
/// author: Karl-Mihkel Ott

#ifndef MATRIX_H
#define MATRIX_H

namespace Libdas {

    /**
     * Iterator class for matrices
     */
    template<typename Val_t>
    class MatrixIterator {
        private:
            using pointer = Val_t*;
            using reference = Val_t&;

            size_t m_mat_size;
            bool m_is_row_major;
            size_t m_index = 0;
            pointer m_data;

            int i, j;

        public:
            MatrixIterator(pointer _data, size_t _mat_size, bool _is_row_major = true) :
                m_data(_data), m_mat_size(_mat_size), m_is_row_major(_is_row_major) {}

            // a, b, _c, d
            // e, f, g, h,
            // i, j, k, l
            // m, n, o, p
            //
            // memory layout:
            //      a, b, _c, d, e, f, g, h, i, j, k, l, m, n, o, p
            
            void operator++(int) {
                // row major matrix order is selected
                if(m_is_row_major)
                    m_data++;
                else {
                    m_index += m_mat_size;
                    // check if the index exceeds maximum row
                    if(m_index / m_mat_size >= m_mat_size) {
                        m_index -= m_mat_size;
                        m_data = m_data - m_index + (m_index % m_mat_size) + 1;
                        m_index %= m_mat_size;
                        m_index++;

                        // out of column bounds
                        if(m_index >= m_mat_size)
                            m_data = m_data - m_index + m_mat_size * m_mat_size;
                    }
                    else m_data += m_mat_size;
                }
            }


            bool operator!=(const MatrixIterator &_it) {
                return m_data != _it.m_data;
            }


            bool operator==(const MatrixIterator &_it) {
                return m_data == _it.m_data;
            }

            reference operator*() const {
                return *m_data;
            }

            pointer operator->() {
                return m_data;
            }


            pointer GetData() {
                return m_data;
            }
    
    };


    /**
     * 2x2 matrix structure
     */
    template<typename T>
    struct Matrix2 {
        typedef MatrixIterator<T> iterator;

        Vector2<T> row1, row2;

        Matrix2();
        Matrix2(const Vector2<T> &r1, const Vector2<T> &r2);
        Matrix2(Vector2<T> &&r1, Vector2<T> &&r2);
        Matrix2(const Matrix2<T> &val);
        Matrix2(Matrix2<T> &&val);
        void operator=(const Matrix2<T> &val);

    
        /******************************/
        /***** Operator overloads *****/
        /******************************/

        Matrix2<T> operator+(const Matrix2<T> &_mat) const; 
        Matrix2<T> operator+(const T &_c) const;
        Matrix2<T> operator-(const Matrix2<T> &_mat) const;
        Matrix2<T> operator-(const T &_c) const;
        Matrix2<T> operator*(const T &_c) const;
        Matrix2<T> operator*(const Matrix2<T> &_mat) const; 
        Vector2<T> operator*(const Vector2<T> &_vec) const;
        Matrix2<T> operator/(const T &_c) const;
        void operator*=(const T &_c);
        void operator*=(const Matrix2<T> &_mat);
        void operator+=(const T &_c);
        void operator+=(const Matrix2<T> &_mat);
        void operator-=(const T &_c);
        void operator-=(const Matrix2<T> &_mat);
        void operator/=(const T &_c);
        bool operator==(const Matrix2<T> &_mat) const;
        bool operator!=(const Matrix2<T> &_mat) const;


        /** 
         * Find the determinant of current matrix instance
         */
        template<typename DT>
        static DT Determinant(const Matrix2<DT> &_mat);
        /** 
         * Find the inverse of the current matrix 
         */
        Matrix2<T> Inverse() const;
        /** 
         * Transpose the current matrix
         */
        Matrix2<T> Transpose() const;

        /// Iterators
        
        iterator BeginRowMajor() const {
            return iterator(const_cast<T*>(&row1.first), 2, true);
        }

        iterator EndRowMajor() const {
            return iterator(const_cast<T*>(&row2.second + 1), 2, true);
        }

        iterator BeginColumnMajor() const {
            return iterator(const_cast<T*>(&row1.first), 2, false);
        }

        iterator EndColumnMajor() const {
            return iterator(const_cast<T*>(&row2.second) + 1, 2, false);
        }


#ifdef _DEBUG
        /** 
         * Log matrix into console output and add description to it
         */
        void Log(const std::string &_desc);
#endif
    };

    
    template<typename T>
    Matrix2<T>::Matrix2() {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1 = Vector2<T>(1, 0);
            row2 = Vector2<T>(0, 1);
        }
    }


    template<typename T>
    Matrix2<T>::Matrix2(const Vector2<T> &_r1, const Vector2<T> &_r2) {
        row1 = _r1;
        row2 = _r2;
    }


    template<typename T>
    Matrix2<T>::Matrix2(Vector2<T> &&_r1, Vector2<T> &&_r2) {
        row1 = std::move(_r1);
        row2 = std::move(_r2);
    }


    template<typename T>
    Matrix2<T>::Matrix2(const Matrix2<T> &_val) {
        row1 = _val.row1;
        row2 = _val.row2;
    }


    template<typename T>
    Matrix2<T>::Matrix2(Matrix2<T> &&_val) {
        row1 = std::move(_val.row1);
        row2 = std::move(_val.row2);
    }


    template<typename T>
    void Matrix2<T>::operator=(const Matrix2<T> &_val) {
        row1 = _val.row1;
        row2 = _val.row2;
    }


    /// Add two matrices together
    template<typename T>
    Matrix2<T> Matrix2<T>::operator+(const Matrix2<T> &_mat) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix2<T> out{}; 
            out.row1 = Vector2<T>{row1.first + _mat.row1.first, row1.second + _mat.row1.second};
            out.row2 = Vector2<T>{row2.first + _mat.row2.first, row1.second + _mat.row2.second};

            return out;
        }

        else return Matrix2<T>{};
    }


    /// Add constant to the current matrix
    template<typename T>
    Matrix2<T> Matrix2<T>::operator+(const T &_c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix2<T> out{};
            out.row1 = Vector2<T>{row1.first + _c, row1.second + _c};
            out.row1 = Vector2<T>{row2.first + _c, row2.second + _c};

            return out;
        }

        else return Matrix2<T>{};
    }


    /// Substract current matrix with given matrix
    template<typename T>
    Matrix2<T> Matrix2<T>::operator-(const Matrix2<T> &_mat) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix2<T> out{};
            out.row1 = Vector2<T>{row1.first - _mat.row1.first, row1.second - _mat.row1.second};
            out.row2 = Vector2<T>{row2.first - _mat.row2.first, row2.second - _mat.row2.second};

            return out;
        }

        else return Matrix2<T>{};
    }


    /// Substract a constant number from the current matrix
    template<typename T>
    Matrix2<T> Matrix2<T>::operator-(const T &_c) const {  
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix2<T> out{};
            out.row1 = Vector2<T>{row1.first - _c, row1.second - _c};
            out.row2 = Vector2<T>{row2.first - _c, row2.second - _c};

            return out;
        }

        else return Matrix2<T>{};
    }

    
    /// Multiply all matrix members with a constant
    template<typename T>
    Matrix2<T> Matrix2<T>::operator*(const T &_c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix2<T> out;
            out.row1 = {row1.first * _c, row1.second * _c};
            out.row2 = {row2.first * _c, row2.second * _c};

            return out; 
        }

        else return Matrix2<T>{};
    }


    /// Find the dot product of two matrices
    template<typename T>
    Matrix2<T> Matrix2<T>::operator*(const Matrix2<T> &_mat) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix2<T> out_mat;
            out_mat.row1 = Vector2<float>{
                ((row1.first * _mat.row1.first) + (row1.second * _mat.row2.first)),
                ((row1.first * _mat.row1.second) + (row1.second * _mat.row2.second))
            }; 

            out_mat.row2 = Vector2<float>{
                ((row2.first * _mat.row1.first) + (row2.second * _mat.row2.first)),
                ((row2.first * _mat.row1.second) + (row2.second * _mat.row2.second))
            };

            return out_mat;
        }

        else return Matrix2<T>{};
    }


    /// Find the dot product of current matrix and a vector
    template<typename T>
    Vector2<T> Matrix2<T>::operator*(const Vector2<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector2<T> out = Vector2<T>{
                (row1.first * _vec.first + row1.second * _vec.second),
                (row2.first * _vec.first + row2.second * _vec.second)
            };

            return out; 
        }

        else return Vector2<T>{};
    }


    /// Divide all matrix elements with constant
    template<typename T>
    Matrix2<T> Matrix2<T>::operator/(const T &_c) const { 
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix2<T> out;
            out.row1 = Vector2<T>{row1.first / _c, row1.second / _c};
            out.row2 = Vector2<T>{row2.first / _c, row2.second / _c};

            return out;
        }

        else return Matrix2<T>{};
    }


    /// Multiply all matrix members with a constant and
    /// set the product as the value of the current matrix instance
	template<typename T>
    void Matrix2<T>::operator*=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first *= _c;
            row1.second *= _c;
            row2.first *= _c;
            row2.second *= _c;
        }
    }


    /// Find the cross product of two matrices and set the current matrix
    /// instance value to it
	template<typename T>
    void Matrix2<T>::operator*=(const Matrix2<T> &_mat) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix2<T> new_mat{};
            new_mat.row1 = Vector2<float>{
                (
                    (row1.first * _mat.row1.first) +
                    (row1.second * _mat.row2.first)
                ),
                (
                    (row1.first * _mat.row1.second) +
                    (row1.second * _mat.row2.second)
                )
            }; 

            new_mat.row2 = Vector2<float>{
                (
                    (row2.first * _mat.row1.first) +
                    (row2.second * _mat.row2.first)
                ),
                (
                    (row2.first * _mat.row1.second) +
                    (row2.second * _mat.row2.second)
                )
            };

            *this = new_mat;
        }
    }


    /// Add constant value to matrix and store the value in current matrix instance
	template<typename T>
    void Matrix2<T>::operator+=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first += _c;
            row1.second += _c;
            row2.first += _c;
            row2.second += _c;
        }
    }


    /// Add two matrices together and store the value in current matrix instance
	template<typename T>
    void Matrix2<T>::operator+=(const Matrix2<T> &_mat) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first += _mat.row1.first;
            row1.second += _mat.row1.second;
            row2.first += _mat.row2.first;
            row2.second += _mat.row2.second;
        }
    }


    /// Substract constant value from matrix and store the result in current matrix instance
	template<typename T>
    void Matrix2<T>::operator-=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first -= _c;
            row1.second -= _c;
            row2.first -= _c;
            row2.second -= _c;
        }
    }


    /// Substract a matrix from current matrix and store the result in current matrix instance
	template<typename T>
    void Matrix2<T>::operator-=(const Matrix2<T> &_mat) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first -= _mat.row1.first;
            row1.second -= _mat.row1.second;
            row2.first -= _mat.row2.first;
            row2.second -= _mat.row2.second;
        }
    }


    /// Divide all matrix elements with constant and store the value in current matrix instance
	template<typename T>
    void Matrix2<T>::operator/=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first /= _c;
            row1.second /= _c;
            row2.first /= _c;
            row2.second /= _c;
        }
    }
    
    
    /// Check if current and given matrix instances have equal values
	template<typename T>
    bool Matrix2<T>::operator==(const Matrix2<T> &_mat) const {
        return row1 == _mat.row1 && row2 == _mat.row2;
    }


    /// Check if current and given matrix don't have equal values
    template<typename T>
    bool Matrix2<T>::operator!=(const Matrix2<T> &_mat) const {
        return row1 != _mat.row1 || row2 != _mat.row1;
    }


    /// Find the determinant of current matrix instance
    template<typename T>
    template<typename DT>
    DT Matrix2<T>::Determinant(const Matrix2<DT> &_mat) { 
        return _mat.row1.first * _mat.row2.second - _mat.row1.second * _mat.row2.first; 
    }


    /// Find the inverse of the current matrix 
    template<typename T>
    Matrix2<T> Matrix2<T>::Inverse() const {
        Matrix2<float> fl_mat;
        float inv_det = 1 / Matrix2<T>::Determinant(*this);
        fl_mat.row1.first = row2.second * inv_det;
        fl_mat.row1.second = -row2.first * inv_det;
        fl_mat.row2.first = -row1.second * inv_det;
        fl_mat.row2.second = row1.first * inv_det;

        Matrix2<T> out_mat;
        out_mat.row1 = {(T) fl_mat.row1.first, (T) fl_mat.row1.second};
        out_mat.row2 = {(T) fl_mat.row2.first, (T) fl_mat.row2.second};

        return out_mat;
    }


    /// Transpose the current matrix
    template<typename T>
    Matrix2<T> Matrix2<T>::Transpose() const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix2<T> new_mat{};
            new_mat.row1 = Vector2<T>{row1.first, row2.first};
            new_mat.row2 = Vector2<T>{row1.second, row2.second};

            return new_mat;
        }

        else return Matrix2<T>{};
    }


#ifdef _DEBUG
    template<typename T>
    void Matrix2<T>::Log(const std::string &desc) {
        std::cout << "MAT_LOG: " <<  desc << std::endl;
        std::cout << row1.first << " | " << row1.second << std::endl;
        std::cout << row2.first << " | " << row2.second << "\n" << std::endl;
    }
#endif


    /**
     * 3x3 matrix structure
     */
    template<typename T>
    struct Matrix3 {
        typedef MatrixIterator<T> iterator;
        Vector3<T> row1, row2, row3;

        Matrix3();
        Matrix3(const Vector3<T> &r1, const Vector3<T> &r2, const Vector3<T> &r3);
        Matrix3(Vector3<T> &&r1, Vector3<T> &&r2, Vector3<T> &&r3);
        Matrix3(const Matrix3<T> &val);
        Matrix3(Matrix3<T>&& val);
        void operator=(const Matrix3<T> &val);
        
        
        /******************************/
        /***** Operator overloads *****/
        /******************************/

        Matrix3<T> operator+(const Matrix3<T> &_mat) const;
        Matrix3<T> operator+(const T &_c) const;
        Matrix3<T> operator-(const Matrix3<T> &_mat) const;
        Matrix3<T> operator-(const T &_c) const;
        Matrix3<T> operator*(const T &_c) const;
        Matrix3<T> operator*(const Matrix3<T> &_mat) const;
        Vector3<T> operator*(const Vector3<T> &_vec) const;
        Matrix3<T> operator/(const T &_c) const;
        void operator*=(const T &_c);
        void operator*=(const Matrix3<T> &_mat);
        void operator+=(const T &_c);
        void operator+=(const Matrix3<T> &_mat);
        void operator-=(const T &_c);
        void operator-=(const Matrix3<T> &_mat);
        void operator/=(const T &_c);
        bool operator==(const Matrix3<T> &_mat) const;
        bool operator!=(const Matrix3<T> &_mat) const;
        /**
         * Find the determinant of current matrix instance
         */
        template<typename DT>
        static DT Determinant(const Matrix3<DT> &_mat);
        /** 
         * Find the inverse of the current matrix 
         */
        Matrix3<T> Inverse() const;
        /** 
         * Transpose the current matrix
         */
        Matrix3<T> Transpose() const;

        /// Iterators

        iterator BeginRowMajor() const {
            return iterator(const_cast<T*>(&row1.first), 3, true);
        }

        iterator EndRowMajor() const {
            return iterator(const_cast<T*>(&row3.third) + 1, 3, true);
        }

        iterator BeginColumnMajor() const {
            return iterator(const_cast<T*>(&row1.first), 3, false);
        }

        iterator EndColumnMajor() const {
            return iterator(const_cast<T*>(&row3.third + 1), 3, false);
        }


#ifdef _DEBUG
        void Log(const std::string &desc);
#endif
    };


    template<typename T>
    Matrix3<T>::Matrix3() {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1 = Vector3<T>{1, 0, 0};
            row2 = Vector3<T>{0, 1, 0};
            row3 = Vector3<T>{0, 0, 1};
        }
    }


    template<typename T>
    Matrix3<T>::Matrix3(const Vector3<T> &r1, const Vector3<T> &r2, const Vector3<T> &r3) {
        row1 = r1;
        row2 = r2;
        row3 = r3;
    }


    template<typename T>
    Matrix3<T>::Matrix3(Vector3<T> &&r1, Vector3<T> &&r2, Vector3<T> &&r3) {
        row1 = std::move(r1);
        row2 = std::move(r2);
        row3 = std::move(r3);
    }


    template<typename T>
    Matrix3<T>::Matrix3(const Matrix3<T> &val) {
        row1 = val.row1;
        row2 = val.row2;
        row3 = val.row3;
    }


    template<typename T>
    Matrix3<T>::Matrix3(Matrix3<T> &&val) {
        row1 = std::move(val.row1);
        row2 = std::move(val.row2);
        row3 = std::move(val.row3);
    }


    template<typename T>
    void Matrix3<T>::operator=(const Matrix3<T> &val) {
        row1 = val.row1;
        row2 = val.row2;
        row3 = val.row3;
    }


    /// Add two matrices together
    template<typename T>
    Matrix3<T> Matrix3<T>::operator+(const Matrix3<T> &_mat) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix3<T> out;
            out.row1 = Vector3<T>{row1.first + _mat.row1.first, row1.second + _mat.row1.second, row1.third + _mat.row1.third};
            out.row2 = Vector3<T>{row2.first + _mat.row2.first, row2.second + _mat.row2.second, row2.third + _mat.row2.third};
            out.row3 = Vector3<T>{row3.first + _mat.row3.first, row3.second + _mat.row3.second, row3.third + _mat.row3.third};
            return out;
        }

        else return Matrix3<T>{};
    }


    /// Add constant to the current matrix
    template<typename T>
    Matrix3<T> Matrix3<T>::operator+(const T &_c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix3<T> out{};
            out.row1 = Vector3<T>{row1.first + _c, row1.second + _c, row1.third + _c};
            out.row2 = Vector3<T>{row2.first + _c, row2.second + _c, row2.third + _c};
            out.row3 = Vector3<T>{row3.first + _c, row3.second + _c, row3.third + _c};

            return out;
        }

        else return Matrix3<T>{};
    }


    /// Substract current matrix with given matrix
    template<typename T>
    Matrix3<T> Matrix3<T>::operator-(const Matrix3<T> &_mat) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix3<T> out{};
            out.row1 = Vector3<T>{row1.first - _mat.row1.first, row1.second - _mat.row1.second, row1.third - _mat.row1.third};
            out.row2 = Vector3<T>{row2.first - _mat.row2.first, row2.second - _mat.row2.second, row2.second - _mat.row2.third};
            out.row3 = Vector3<T>{row3.first - _mat.row3.first, row3.second - _mat.row3.second, row3.second - _mat.row3.third};

            return out;
        }

        else return Matrix3<T>{};
    }


    /// Substract a constant number from the current matrix
    template<typename T>
    Matrix3<T> Matrix3<T>::operator-(const T &_c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix3<T> out{};
            out.row1 = Vector3<T>{row1.first - _c, row1.second - _c, row1.third - _c};
            out.row2 = Vector3<T>{row2.first - _c, row2.second - _c, row2.third - _c};
            out.row3 = Vector3<T>{row3.first - _c, row3.second - _c, row3.third - _c};

            return out;
        }

        else return Matrix3<T>{};
    }


    /// Multiply all matrix members with a constant
    template<typename T>
    Matrix3<T> Matrix3<T>::operator*(const T &_c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix3<T> out;
            out.row1 = {row1.first * _c, row1.second * _c, row1.third * _c};
            out.row2 = {row2.first * _c, row2.second * _c, row2.third * _c};
            out.row3 = {row3.first * _c, row3.second * _c, row3.third * _c};
            return out;
        }

        else return Matrix3<T>{};
    }


    /// Find the dot product of two matrices
    template<typename T>
    Matrix3<T> Matrix3<T>::operator*(const Matrix3<T> &matrix) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix3<T> out_mat;
            out_mat.row1 = Vector3<float>{
                ((row1.first * matrix.row1.first) + (row1.second * matrix.row2.first) + (row1.third * matrix.row3.first)), 
                ((row1.first * matrix.row1.second) + (row1.second * matrix.row2.second) + (row1.third * matrix.row3.second)), 
                ((row1.first * matrix.row1.third) + (row1.second * matrix.row2.third) + (row1.third * matrix.row3.third))
            };

            out_mat.row2 = Vector3<float>{
                ((row2.first * matrix.row1.first) + (row2.second * matrix.row2.first) + (row2.third * matrix.row3.first)), 
                ((row2.first * matrix.row1.second) + (row2.second * matrix.row2.second) + (row2.third * matrix.row3.second)), 
                ((row2.first * matrix.row1.third) + (row2.second * matrix.row2.third) + (row2.third * matrix.row3.third))
            };

            out_mat.row3 = Vector3<float>{
                ((row3.first * matrix.row1.first) + (row3.second * matrix.row2.first) + (row3.third * matrix.row3.first)), 
                ((row3.first * matrix.row1.second) + (row3.second * matrix.row2.second) + (row3.third * matrix.row3.second)), 
                ((row3.first * matrix.row1.third) + (row3.second * matrix.row2.third) + (row3.third * matrix.row3.third))
            };

            return out_mat;
        }
        
        else return Matrix3<T>{};
    }


    /// Find the dot product of current matrix and a vector
    template<typename T>
    Vector3<T> Matrix3<T>::operator*(const Vector3<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector3<T> out = {
                (row1.first * _vec.first + row1.second * _vec.second + row1.third * _vec.third),
                (row2.first * _vec.first + row2.second * _vec.second + row2.third * _vec.third),
                (row3.first * _vec.first + row3.second * _vec.second + row3.third * _vec.third)
            }; 

            return out;
        }

        else return Vector3<T>{};
    }


    /// Divide all matrix elements with a constant
    template<typename T>
    Matrix3<T> Matrix3<T>::operator/(const T &_c) const { 
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix3<T> out;
            out.row1 = Vector3<T>{row1.first / _c, row1.second / _c, row1.third / _c};
            out.row2 = Vector3<T>{row2.first / _c, row2.second / _c, row2.third / _c};
            out.row3 = Vector3<T>{row3.first / _c, row3.second / _c, row3.third / _c};

            return out;
        }

        else return Matrix3<T>{};
    }


    /// Multiply all matrix members with a constant and set the product as the value of the current matrix instance
	template<typename T>
    void Matrix3<T>::operator*=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first *= _c;
            row1.second *= _c;
            row1.third *= _c;

            row2.first *= _c;
            row2.second *= _c;
            row2.third *= _c;

            row3.first *= _c;
            row3.second *= _c;
            row3.third *= _c;
        }
    }


    /// Find the cross product of two matrices and set the current matrix instance value to it
    template<typename T>
    void Matrix3<T>::operator*=(const Matrix3<T> &_mat) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix3<T> new_mat{};
            new_mat.row1 = {
                ((row1.first * _mat.row1.first) + (row1.second * _mat.row2.first) + (row1.third * _mat.row3.first)), 
                ((row1.first * _mat.row1.second) + (row1.second * _mat.row2.second) + (row1.third * _mat.row3.second)), 
                ((row1.first * _mat.row1.third) + (row1.second * _mat.row2.third) + (row1.third * _mat.row3.third)) 
            };

            new_mat.row2 = {
                ((row2.first * _mat.row1.first) + (row2.second * _mat.row2.first) + (row2.third * _mat.row3.first)), 
                ((row2.first * _mat.row1.second) + (row2.second * _mat.row2.second) + (row2.third * _mat.row3.second)), 
                ((row2.first * _mat.row1.third) + (row2.second * _mat.row2.third) + (row2.third * _mat.row3.third)) 
            };

            new_mat.row3 = {
                ((row3.first * _mat.row1.first) + (row3.second * _mat.row2.first) + (row3.third * _mat.row3.first)), 
                ((row3.first * _mat.row1.second) + (row3.second * _mat.row2.second) + (row3.third * _mat.row3.second)), 
                ((row3.first * _mat.row1.third) + (row3.second * _mat.row2.third) + (row3.third * _mat.row3.third))
            };

            *this = new_mat;
        }
    }


    /// Add constant value to matrix and store the value in current matrix instance
    template<typename T>
    void Matrix3<T>::operator+=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first += _c;
            row1.second += _c;
            row1.third += _c;

            row2.first += _c;
            row2.second += _c;
            row2.third += _c;

            row3.first += _c;
            row3.second += _c;
            row3.third += _c;
        }
    }


    /// Add two matrices together and store the value in current matrix instance
    template<typename T>
    void Matrix3<T>::operator+=(const Matrix3<T> &_mat) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first += _mat.row1.first;
            row1.second += _mat.row1.second;
            row1.third += _mat.row1.third;

            row2.first += _mat.row2.first;
            row2.second += _mat.row2.second;

            row3.first += _mat.row3.first;
            row3.second += _mat.row3.second;
            row3.third += _mat.row3.third;
        }
    }


    /// Substract constant value from matrix and store the result in current matrix instance
    template<typename T>
    void Matrix3<T>::operator-=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first -= _c;
            row1.second -= _c;
            row1.third -= _c;

            row2.first -= _c;
            row2.second -= _c;
            row2.third -= _c;

            row3.first -= _c;
            row3.second -= _c;
            row3.third -= _c;
        }
    }


    /// Substract a matrix from current matrix and store the result in current matrix instance
    template<typename T>
    void Matrix3<T>::operator-=(const Matrix3<T> &_mat) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first -= _mat.row1.first;
            row1.second -= _mat.row1.second;
            row1.third -= _mat.row1.third;

            row2.first -= _mat.row2.first;
            row2.second -= _mat.row2.second;
            row2.third -= _mat.row2.third;

            row3.first -= _mat.row3.first;
            row3.second -= _mat.row3.second;
            row3.third -= _mat.row3.third;
        }
    }


    /// Divide all matrix elements with constant and store the value in current matrix instance
    template<typename T>
    void Matrix3<T>::operator/=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first /= _c;
            row1.second /= _c;
            row1.third /= _c;

            row2.first /= _c;
            row2.second /= _c;
            row2.third /= _c;

            row3.first /= _c;
            row3.second /= _c;
            row3.third /= _c;
        }
    }
    
    
    /// Check if current and given matrix instances have equal values
    template<typename T>
    bool Matrix3<T>::operator==(const Matrix3<T> &_mat) const {
        return row1 == _mat.row1 && row2 == _mat.row2 && row3 == _mat.row3;
    }


    /// Check if current and given matrix instances don't have equal values
    template <typename T>
    bool Matrix3<T>::operator!=(const Matrix3<T> &_mat) const {
        return row1 != _mat.row1 || row2 != _mat.row2 || row3 != _mat.row3;
    }


    /// Find the determinant of current matrix instance
    template<typename T>
    template<typename DT>
    DT Matrix3<T>::Determinant(const Matrix3<DT> &_mat) {
        return static_cast<DT>(
            (_mat.row1.first * _mat.row2.second * _mat.row3.third) +
            (_mat.row1.second * _mat.row2.third * _mat.row3.first) + 
            (_mat.row1.third * _mat.row2.first * _mat.row3.second) -

            (_mat.row1.third * _mat.row2.second * _mat.row3.first) -
            (_mat.row1.first * _mat.row2.third * _mat.row3.second) -
            (_mat.row1.second * _mat.row2.first * _mat.row3.third) 
        );
    }


    /// Find the inverse of the current matrix 
    template<typename T>
    Matrix3<T> Matrix3<T>::Inverse() const {
        Matrix3<float> fl_mat;
        float inv_det = 1 / Matrix3<T>::Determinant(*this);

        fl_mat.row1 = {
            inv_det * (row2.second * row3.third - row2.third * row3.second),
            inv_det * -(row1.second * row3.third - row1.third * row3.second),
            inv_det * (row1.second * row2.third - row1.third * row2.second) 
        };

        fl_mat.row2 = {
            inv_det * -(row2.first * row3.third - row2.third * row3.first),
            inv_det * (row1.first * row3.third - row1.third * row3.first),
            inv_det * -(row1.first * row2.third - row1.third * row2.first)
        };
        
        fl_mat.row3 = {
            inv_det * (row2.first * row3.second - row2.second * row3.first),
            inv_det * -(row1.first * row3.second - row1.second * row3.first),
            inv_det * (row1.first * row2.second - row1.second * row2.first)
        };

        Matrix3<T> out_mat;
        out_mat.row1 = {(T) fl_mat.row1.first, (T) fl_mat.row1.second, (T) fl_mat.row1.third}; 
        out_mat.row2 = {(T) fl_mat.row2.first, (T) fl_mat.row2.second, (T) fl_mat.row2.third};
        out_mat.row3 = {(T) fl_mat.row3.first, (T) fl_mat.row3.second, (T) fl_mat.row3.third};
        
        return out_mat;
    }


    /// Transpose the current matrix
    template<typename T>
    Matrix3<T> Matrix3<T>::Transpose() const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix3<T> new_mat{};
            new_mat.row1 = Vector3<T>{row1.first, row2.first, row3.first};
            new_mat.row2 = Vector3<T>{row1.second, row2.second, row3.second};
            new_mat.row3 = Vector3<T>{row1.third, row2.third, row3.third};

            return new_mat;
        }

        else return Matrix3<T>{};
    }


#ifdef _DEBUG
    template<typename T>
    void Matrix3<T>::Log(const std::string &desc) {
        std::cout << "MAT_LOG: " <<  desc << std::endl;
        std::cout << row1.first << " | " << row1.second << " | " << row1.third << std::endl;
        std::cout << row2.first << " | " << row2.second << " | " << row2.third << std::endl;
        std::cout << row3.first << " | " << row3.second << " | " << row3.third << "\n" << std::endl;
    }
#endif


    /**
     * 4x4 matrix structure
     */
    template<typename T>
    struct Matrix4 {
        typedef MatrixIterator<T> iterator;
        Matrix4();
        Matrix4(const Vector4<T> &_r1, const Vector4<T> &_r2, const Vector4<T> &_r3, const Vector4<T> &_r4);
        Matrix4(Vector4<T> &&_r1, Vector4<T> &&_r2, Vector4<T> &&_r3, Vector4<T> &&_r4);
        Matrix4(const Matrix4<T> &_val);
        Matrix4(Matrix4<T>&& _val);
        void operator=(const Matrix4<T> &_val);

        Vector4<T> row1, row2, row3, row4; 


        /******************************/
        /***** Operator overloads *****/
        /******************************/

        Matrix4<T> operator+(const Matrix4<T> &_mat) const; 
        Matrix4<T> operator+(const T &_c) const;
        Matrix4<T> operator-(const Matrix4<T> &_mat) const;
        Matrix4<T> operator-(const T &_c) const;
        Matrix4<T> operator*(const T &_c) const;
        Matrix4<T> operator*(const Matrix4<T> &_mat) const; 
        Vector4<T> operator*(const Vector4<T> &_vec) const; 
        Matrix4<T> operator/(const T &_c) const;
        void operator*=(const T &_c);
        void operator*=(const Matrix4<T> &_mat);
        void operator+=(const T &_c);
        void operator+=(const Matrix4<T> &_mat);
        void operator-=(const T &_c);
        void operator-=(const Matrix4<T> &_mat);
        void operator/=(const T &_c);
        bool operator==(const Matrix4<T> &_mat) const;
        bool operator!=(const Matrix4<T> &_mat) const;


        /// Find the determinant of current matrix instance
        template<typename DT>
        static DT Determinant(const Matrix4<DT> &_mat);

        
        /// Find the inverse of the current matrix 
        Matrix4<T> Inverse() const;


        /// Transpose the current matrix
        Matrix4<T> Transpose() const;

        /// Iterators

        iterator BeginRowMajor() const {
            return iterator(const_cast<T*>(&row1.first), 4, true);
        }

        iterator EndRowMajor() const {
            return iterator(const_cast<T*>(&row4.fourth + 1), 4, true);
        }

        iterator BeginColumnMajor() const {
            return iterator(const_cast<T*>(&row1.first), 4, false);
        }

        iterator EndColumnMajor() const {
            return iterator(const_cast<T*>(&row4.fourth + 1), 4, false);
        }


#ifdef _DEBUG
        /// Log matrix into console output and add description to it
        void Log(const std::string &desc);
#endif
    };


    template<typename T>
    Matrix4<T>::Matrix4() {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1 = Vector4<T>{1, 0, 0, 0};
            row2 = Vector4<T>{0, 1, 0, 0};
            row3 = Vector4<T>{0, 0, 1, 0};
            row4 = Vector4<T>{0, 0, 0, 1};
        }
    }


    template<typename T>
    Matrix4<T>::Matrix4(const Vector4<T> &_r1, const Vector4<T> &_r2, const Vector4<T> &_r3, const Vector4<T> &_r4) {
        row1 = _r1;
        row2 = _r2;
        row3 = _r3;
        row4 = _r4;
    }


    template<typename T>
    Matrix4<T>::Matrix4(Vector4<T> &&_r1, Vector4<T> &&_r2, Vector4<T> &&_r3, Vector4<T> &&_r4) {
        row1 = std::move(_r1);
        row2 = std::move(_r2);
        row3 = std::move(_r3);
        row4 = std::move(_r4);
    }


    template<typename T>
    Matrix4<T>::Matrix4(const Matrix4<T> &_val) {
        row1 = _val.row1;
        row2 = _val.row2;
        row3 = _val.row3;
        row4 = _val.row4;
    }


    template<typename T>
    Matrix4<T>::Matrix4(Matrix4<T> &&_val) {
        row1 = std::move(_val.row1);
        row2 = std::move(_val.row2);
        row3 = std::move(_val.row3);
        row4 = std::move(_val.row4);
    }


    template<typename T>
    void Matrix4<T>::operator=(const Matrix4<T> &_val) {
        row1 = _val.row1;
        row2 = _val.row2;
        row3 = _val.row3;
        row4 = _val.row4;
    }

    
    /// Add two matrices together
    template<typename T>
    Matrix4<T> Matrix4<T>::operator+(const Matrix4<T> &_mat) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix4<T> out;
            out.row1 = {row1.first + _mat.row1.first, row1.second + _mat.row1.second, row1.third + _mat.row1.third, row1.fourth + _mat.row1.fourth};
            out.row2 = {row2.first + _mat.row2.first, row2.second + _mat.row2.second, row2.third + _mat.row2.third, row2.fourth + _mat.row2.fourth};
            out.row3 = {row3.first + _mat.row3.first, row3.second + _mat.row3.second, row3.third + _mat.row3.third, row3.fourth + _mat.row3.fourth};
            out.row4 = {row4.first + _mat.row4.first, row4.second + _mat.row4.second, row4.third + _mat.row4.third, row4.fourth + _mat.row4.fourth};

            return out;
        }

        else return Matrix4<T>{};
    }


    /// Add constant to the current matrix
    template<typename T>
    Matrix4<T> Matrix4<T>::operator+(const T &_c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix4<T> out{};
            out.row1 = Vector4<T>{row1.first + _c, row1.second + _c, row1.third + _c, row1.fourth + _c};
            out.row2 = Vector4<T>{row2.first + _c, row2.second + _c, row2.third + _c, row2.fourth + _c};
            out.row3 = Vector4<T>{row3.first + _c, row3.second + _c, row3.third + _c, row3.fourth + _c};
            out.row4 = Vector4<T>{row4.first + _c, row4.second + _c, row4.third + _c, row4.fourth + _c};

            return out;
        }

        else return Matrix4<T>{};
    }


    /// Substract current matrix with given matrix
    template<typename T>
    Matrix4<T> Matrix4<T>::operator-(const Matrix4<T> &_mat) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix4<T> out{};
            out.row1 = Vector4<T>{row1.first - _mat.row1.first, row1.second - _mat.row1.second, row1.third - _mat.row1.third, row1.fourth - _mat.row1.fourth};
            out.row2 = Vector4<T>{row2.first - _mat.row2.first, row2.second - _mat.row2.second, row2.second - _mat.row2.third, row2.fourth - _mat.row2.fourth};
            out.row3 = Vector4<T>{row3.first - _mat.row3.first, row3.second - _mat.row3.second, row3.second - _mat.row3.third, row3.fourth - _mat.row3.fourth};
            out.row4 = Vector4<T>{row4.first - _mat.row4.first, row4.second - _mat.row4.second, row4.second - _mat.row4.third, row4.fourth - _mat.row4.fourth};

            return out;
        }

        else return Matrix4<T>{};
    }


    /// Substract a constant number from the current matrix
    template<typename T>
    Matrix4<T> Matrix4<T>::operator-(const T &_c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix4<T> out{};
            out.row1 = Vector4<T>{row1.first - _c, row1.second - _c, row1.third - _c, row1.fourth - _c};
            out.row2 = Vector4<T>{row2.first - _c, row2.second - _c, row2.third - _c, row2.fourth - _c};
            out.row3 = Vector4<T>{row3.first - _c, row3.second - _c, row3.third - _c, row3.fourth - _c};
            out.row4 = Vector4<T>{row4.first - _c, row4.second - _c, row4.third - _c, row4.fourth - _c};

            return out;
        }

        else return Matrix4<T>{};
    }


    /// Multiply all matrix members with a constant
    template<typename T>
    Matrix4<T> Matrix4<T>::operator*(const T &_c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix4<T> out;
            out.row1 = {row1.first * _c, row1.second * _c, row1.third * _c, row1.fourth * _c};
            out.row2 = {row2.first * _c, row2.second * _c, row2.third * _c, row2.fourth * _c};
            out.row3 = {row3.first * _c, row3.second * _c, row3.third * _c, row3.fourth * _c};
            out.row4 = {row4.first * _c, row4.second * _c, row4.third * _c, row4.fourth * _c};
            return out;
        }

        else return Matrix4<T>{};
    }
 
    
    /// Find the dot product of two matrices
    template<typename T>
    Matrix4<T> Matrix4<T>::operator*(const Matrix4<T> &_mat) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix4<T> out_mat;
            out_mat.row1 = Vector4<T>{
                ((row1.first * _mat.row1.first) + (row1.second * _mat.row2.first) + (row1.third * _mat.row3.first) + (row1.fourth * _mat.row4.first)), 
                ((row1.first * _mat.row1.second) + (row1.second * _mat.row2.second) + (row1.third * _mat.row3.second) + (row1.fourth * _mat.row4.second)), 
                ((row1.first * _mat.row1.third) + (row1.second * _mat.row2.third) + (row1.third * _mat.row3.third) + (row1.fourth * _mat.row4.third)), 
                ((row1.first * _mat.row1.fourth) + (row1.second * _mat.row2.fourth) + (row1.third * _mat.row3.fourth) + (row1.fourth * _mat.row4.fourth))
            };

            out_mat.row2 = Vector4<T>{ 
                ((row2.first * _mat.row1.first) + (row2.second * _mat.row2.first) + (row2.third * _mat.row3.first) + (row2.fourth * _mat.row4.first)), 
                ((row2.first * _mat.row1.second) + (row2.second * _mat.row2.second) + (row2.third * _mat.row3.second) + (row2.fourth * _mat.row4.second)), 
                ((row2.first * _mat.row1.third) + (row2.second * _mat.row2.third) + (row2.third * _mat.row3.third) + (row2.fourth * _mat.row4.third)), 
                ((row2.first * _mat.row1.fourth) + (row2.second * _mat.row2.fourth) + (row2.third * _mat.row3.fourth) + (row2.fourth * _mat.row4.fourth))
            };

            out_mat.row3 = Vector4<T>{
                ((row3.first * _mat.row1.first) + (row3.second * _mat.row2.first) + (row3.third * _mat.row3.first) + (row3.fourth * _mat.row4.first)), 
                ((row3.first * _mat.row1.second) + (row3.second * _mat.row2.second) + (row3.third * _mat.row3.second) + (row3.fourth * _mat.row4.second)), 
                ((row3.first * _mat.row1.third) + (row3.second * _mat.row2.third) + (row3.third * _mat.row3.third) + (row3.fourth * _mat.row4.third)), 
                ((row3.first * _mat.row1.fourth) + (row3.second * _mat.row2.fourth) + (row3.third * _mat.row3.fourth) + (row3.fourth * _mat.row4.fourth))
            };

            out_mat.row4 = Vector4<T>{
                ((row4.first * _mat.row1.first) + (row4.second * _mat.row2.first) + (row4.third * _mat.row3.first) + (row4.fourth * _mat.row4.first)), 
                ((row4.first * _mat.row1.second) + (row4.second * _mat.row2.second) + (row4.third * _mat.row3.second) + (row4.fourth * _mat.row4.second)), 
                ((row4.first * _mat.row1.third) + (row4.second * _mat.row2.third) + (row4.third * _mat.row3.third) + (row4.fourth * _mat.row4.third)), 
                ((row4.first * _mat.row1.fourth) + (row4.second * _mat.row2.fourth) + (row4.third * _mat.row3.fourth) + (row4.fourth * _mat.row4.fourth))
            };

            return out_mat;
        }

        else return Matrix4<T>{};
    }


    /// Multiply with column vector
    template<typename T>
    Vector4<T> Matrix4<T>::operator*(const Vector4<T> &_vec) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Vector4<T> out_vec;
            out_vec.first = { _vec.first * row1.first + _vec.second * row1.second + _vec.third * row1.third + _vec.fourth * row1.fourth };
            out_vec.second = { _vec.first * row2.first + _vec.second * row2.second + _vec.third * row2.third + _vec.fourth * row2.fourth };
            out_vec.third = { _vec.first * row3.first + _vec.second * row3.second + _vec.third * row3.third + _vec.fourth * row3.fourth };
            out_vec.fourth = { _vec.first * row4.first + _vec.second * row4.second + _vec.third * row4.third + _vec.fourth * row4.fourth };

            return out_vec;
        }

        else return Vector4<T>{};
    }


    /// Divide all matrix elements with a constant
    template<typename T>
    Matrix4<T> Matrix4<T>::operator/(const T &_c) const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix4<T> out;
            out.row1 = Vector4<T>{row1.first / _c, row1.second / _c, row1.third / _c, row1.fourth / _c};
            out.row2 = Vector4<T>{row2.first / _c, row2.second / _c, row2.third / _c, row2.fourth / _c};
            out.row3 = Vector4<T>{row3.first / _c, row3.second / _c, row3.third / _c, row3.fourth / _c};
            out.row4 = Vector4<T>{row4.first / _c, row4.second / _c, row4.third / _c, row4.fourth / _c};

            return out;
        }

        else return Matrix4<T>{};
    }


    /// Multiply all matrix members with a constant and set the product as the value of the current matrix instance
	template<typename T>
    void Matrix4<T>::operator*=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first *= _c;
            row1.second *= _c;
            row1.third *= _c;
            row1.fourth *= _c;

            row2.first *= _c;
            row2.second *= _c;
            row2.third *= _c;
            row2.fourth *= _c;

            row3.first *= _c;
            row3.second *= _c;
            row3.third *= _c;
            row3.fourth *= _c;

            row4.first *= _c;
            row4.second *= _c;
            row4.third *= _c;
            row4.fourth *= _c;
        }
    }


    /// Find the cross product of two matrices and set the current matrix instance value to it
    template<typename T>
    void Matrix4<T>::operator*=(const Matrix4<T> &_mat) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix4<T> new_mat{};
            new_mat.row1 = Vector4<T>{
                ((row1.first * _mat.row1.first) + (row1.second * _mat.row2.first) + (row1.third * _mat.row3.first) + (row1.fourth * _mat.row4.first)), 
                ((row1.first * _mat.row1.second) + (row1.second * _mat.row2.second) + (row1.third * _mat.row3.second) + (row1.fourth * _mat.row4.second)), 
                ((row1.first * _mat.row1.third) + (row1.second * _mat.row2.third) + (row1.third * _mat.row3.third) + (row1.fourth * _mat.row4.third)), 
                ((row1.first * _mat.row1.fourth) + (row1.second * _mat.row2.fourth) + (row1.third * _mat.row3.fourth) + (row1.fourth * _mat.row4.fourth))
            };

            new_mat.row2 = Vector4<T>{ 
                ((row2.first * _mat.row1.first) + (row2.second * _mat.row2.first) + (row2.third * _mat.row3.first) + (row2.fourth * _mat.row4.first)), 
                ((row2.first * _mat.row1.second) + (row2.second * _mat.row2.second) + (row2.third * _mat.row3.second) + (row2.fourth * _mat.row4.second)), 
                ((row2.first * _mat.row1.third) + (row2.second * _mat.row2.third) + (row2.third * _mat.row3.third) + (row2.fourth * _mat.row4.third)), 
                ((row2.first * _mat.row1.fourth) + (row2.second * _mat.row2.fourth) + (row2.third * _mat.row3.fourth) + (row2.fourth * _mat.row4.fourth))
            };

            new_mat.row3 = Vector4<T>{
                ((row3.first * _mat.row1.first) + (row3.second * _mat.row2.first) + (row3.third * _mat.row3.first) + (row3.fourth * _mat.row4.first)), 
                ((row3.first * _mat.row1.second) + (row3.second * _mat.row2.second) + (row3.third * _mat.row3.second) + (row3.fourth * _mat.row4.second)), 
                ((row3.first * _mat.row1.third) + (row3.second * _mat.row2.third) + (row3.third * _mat.row3.third) + (row3.fourth * _mat.row4.third)), 
                ((row3.first * _mat.row1.fourth) + (row3.second * _mat.row2.fourth) + (row3.third * _mat.row3.fourth) + (row3.fourth * _mat.row4.fourth))
            };

            new_mat.row4 = Vector4<T>{
                ((row4.first * _mat.row1.first) + (row4.second * _mat.row2.first) + (row4.third * _mat.row3.first) + (row4.fourth * _mat.row4.first)), 
                ((row4.first * _mat.row1.second) + (row4.second * _mat.row2.second) + (row4.third * _mat.row3.second) + (row4.fourth * _mat.row4.second)), 
                ((row4.first * _mat.row1.third) + (row4.second * _mat.row2.third) + (row4.third * _mat.row3.third) + (row4.fourth * _mat.row4.third)), 
                ((row4.first * _mat.row1.fourth) + (row4.second * _mat.row2.fourth) + (row4.third * _mat.row3.fourth) + (row4.fourth * _mat.row4.fourth))
            };

            *this = new_mat;
        }
    }


    /// Add constant value to matrix and store the value in current matrix instance
    template<typename T>
    void Matrix4<T>::operator+=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first += _c;
            row1.second += _c;
            row1.third += _c;
            row1.fourth += _c;

            row2.first += _c;
            row2.second += _c;
            row2.third += _c;
            row2.fourth += _c;

            row3.first += _c;
            row3.second += _c;
            row3.third += _c;
            row3.fourth += _c;

            row4.first += _c;
            row4.second += _c;
            row4.third += _c;
            row4.fourth += _c;
        }
    }


    /// Add two matrices together and store the value in current matrix instance
    template<typename T>
    void Matrix4<T>::operator+=(const Matrix4<T> &_mat) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first += _mat.row1.first;
            row1.second += _mat.row1.second;
            row1.third += _mat.row1.third;
            row1.fourth += _mat.row1.fourth;

            row2.first += _mat.row2.first;
            row2.second += _mat.row2.second;
            row2.third += _mat.row2.third;
            row2.fourth += _mat.row2.fourth;

            row3.first += _mat.row3.first;
            row3.second += _mat.row3.second;
            row3.third += _mat.row3.third;
            row3.fourth += _mat.row3.fourth;

            row4.first += _mat.row4.first;
            row4.second += _mat.row4.second;
            row4.third += _mat.row4.third;
            row4.fourth += _mat.row4.fourth;
        }
    }


    /// Substract constant value from matrix and store the result in current matrix instance
    template<typename T>
    void Matrix4<T>::operator-=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first -= _c;
            row1.second -= _c;
            row1.third -= _c;
            row1.fourth -= _c;

            row2.first -= _c;
            row2.second -= _c;
            row2.third -= _c;
            row2.fourth -= _c;

            row3.first -= _c;
            row3.second -= _c;
            row3.third -= _c;
            row3.fourth -= _c;

            row4.first -= _c;
            row4.second -= _c;
            row4.third -= _c;
            row4.fourth -= _c;
        }
    }


    /// Substract a matrix from current matrix and store the result in current matrix instance
    template<typename T>
    void Matrix4<T>::operator-=(const Matrix4<T> &_mat) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first -= _mat.row1.first;
            row1.second -= _mat.row1.second;
            row1.third -= _mat.row1.third;
            row1.fourth -= _mat.row1.fourth;

            row2.first -= _mat.row2.first;
            row2.second -= _mat.row2.second;
            row2.third -= _mat.row2.third;
            row2.fourth -= _mat.row2.fourth;

            row3.first -= _mat.row3.first;
            row3.second -= _mat.row3.second;
            row3.third -= _mat.row3.third;
            row3.fourth -= _mat.row3.fourth;

            row4.first -= _mat.row4.first;
            row4.second -= _mat.row4.second;
            row4.third -= _mat.row4.third;
            row4.fourth -= _mat.row4.fourth;
        }
    }


    /// Divide all matrix elements with constant and store the value in current matrix instance
    template<typename T>
    void Matrix4<T>::operator/=(const T &_c) {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            row1.first /= _c;
            row1.second /= _c;
            row1.third /= _c;
            row1.fourth /= _c;

            row2.first /= _c;
            row2.second /= _c;
            row2.third /= _c;
            row2.fourth /= _c;

            row3.first /= _c;
            row3.second /= _c;
            row3.third /= _c;
            row3.fourth /= _c;

            row4.first /= _c;
            row4.second /= _c;
            row4.third /= _c;
            row4.fourth /= _c;
        }
    }
    
    
    /// Check if current and given matrix instances have equal values
    template<typename T>
    bool Matrix4<T>::operator==(const Matrix4<T> &_mat) const {
        return row1 == _mat.row1 && row2 == _mat.row2 && row3 == _mat.row3 && row4 == _mat.row4;
    }


    /// Check if current and given matrices don't have equal values
    template<typename T>
    bool Matrix4<T>::operator!=(const Matrix4<T> &_mat) const {
        return row1 != _mat.row1 || row2 != _mat.row2 || row3 != _mat.row3 || row4 != _mat.row4;
    }

    
    /// Find the determinant of current matrix instance
    template<typename T>
    template<typename DT>
    DT Matrix4<T>::Determinant(const Matrix4<DT> &_mat) {
        Matrix3<DT> adj_mat[4];
        
        adj_mat[0].row1 = Vector3<float>{_mat.row2.second, _mat.row2.third, _mat.row2.fourth};
        adj_mat[0].row2 = Vector3<float>{_mat.row3.second, _mat.row3.third, _mat.row3.fourth};
        adj_mat[0].row3 = Vector3<float>{_mat.row4.second, _mat.row4.third, _mat.row4.fourth};

        adj_mat[1].row1 = Vector3<float>{_mat.row2.first, _mat.row2.third, _mat.row2.fourth};
        adj_mat[1].row2 = Vector3<float>{_mat.row3.first, _mat.row3.third, _mat.row3.fourth};
        adj_mat[1].row3 = Vector3<float>{_mat.row4.first, _mat.row4.third, _mat.row4.fourth};

        adj_mat[2].row1 = Vector3<float>{_mat.row2.first, _mat.row2.second, _mat.row2.fourth};
        adj_mat[2].row2 = Vector3<float>{_mat.row3.first, _mat.row3.second, _mat.row3.fourth};
        adj_mat[2].row3 = Vector3<float>{_mat.row4.first, _mat.row4.second, _mat.row4.fourth};

        adj_mat[3].row1 = Vector3<float>{_mat.row2.first, _mat.row2.second, _mat.row2.third};
        adj_mat[3].row2 = Vector3<float>{_mat.row3.first, _mat.row3.second, _mat.row3.third};
        adj_mat[3].row3 = Vector3<float>{_mat.row4.first, _mat.row4.second, _mat.row4.third};

        Vector4<DT> out;
        out.first = _mat.row1.first * Matrix3<DT>::Determinant(adj_mat[0]);
        out.second = _mat.row1.second * Matrix3<DT>::Determinant(adj_mat[1]);
        out.third = _mat.row1.third * Matrix3<DT>::Determinant(adj_mat[2]);
        out.fourth = _mat.row1.fourth * Matrix3<DT>::Determinant(adj_mat[3]); 
        
        return out.first - out.second + out.third - out.fourth;
    };


    /// Find the inverse of the current matrix 
    template<typename T>
    Matrix4<T> Matrix4<T>::Inverse() const {
        float inv_det = 1 / Matrix4<T>::Determinant(*this);
        Matrix4<T> out_mat;
        Matrix3<float> adj_mat;
        
        // Row 1
        adj_mat = {
            {row2.second, row2.third, row2.fourth},
            {row3.second, row3.third, row3.fourth},
            {row4.second, row4.third, row4.fourth}
        };
        out_mat.row1.first = static_cast<T>(inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.second, row1.third, row1.fourth},
            {row3.second, row3.third, row3.fourth},
            {row4.second, row4.third, row4.fourth}
        };
        out_mat.row1.second = static_cast<T>(-inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.second, row1.third, row1.fourth},
            {row2.second, row2.third, row2.fourth},
            {row4.second, row4.third, row4.fourth}
        };
        out_mat.row1.third = static_cast<T>(inv_det * Matrix3<T>::Determinant(adj_mat));
        
        adj_mat = {
            {row1.second, row1.third, row1.fourth},
            {row2.second, row2.third, row2.fourth},
            {row3.second, row3.third, row3.fourth}
        };
        out_mat.row1.fourth = static_cast<T>(-inv_det * Matrix3<T>::Determinant(adj_mat)); 

        // Row 2
        adj_mat = {
            {row2.first, row2.third, row2.fourth},
            {row3.first, row3.third, row3.fourth},
            {row4.first, row4.third, row4.fourth}
        };
        out_mat.row2.first = static_cast<T>(-inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.first, row1.third, row1.fourth},
            {row3.first, row3.third, row3.fourth},
            {row4.first, row4.third, row4.fourth}
        };
        out_mat.row2.second = static_cast<T>(inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.first, row1.third, row1.fourth},
            {row2.first, row2.third, row2.fourth},
            {row4.first, row4.third, row4.fourth}
        };
        out_mat.row2.third = static_cast<T>(-inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.first, row1.third, row1.fourth},
            {row2.first, row2.third, row2.fourth},
            {row3.first, row3.third, row3.fourth}
        };
        out_mat.row2.fourth = static_cast<T>(inv_det * Matrix3<T>::Determinant(adj_mat)); 

        // Row 3
        adj_mat = {
            {row2.first, row2.second, row2.fourth},
            {row3.first, row3.second, row3.fourth},
            {row4.first, row4.second, row4.fourth}
        };
        out_mat.row3.first = static_cast<T>(inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.first, row1.second, row1.fourth},
            {row3.first, row3.second, row3.fourth},
            {row4.first, row4.second, row4.fourth}
        };
        out_mat.row3.second = static_cast<T>(-inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.first, row1.second, row1.fourth},
            {row2.first, row2.second, row2.fourth},
            {row4.first, row4.second, row4.fourth}
        };
        out_mat.row3.third = static_cast<T>(inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.first, row1.second, row1.fourth},
            {row2.first, row2.second, row2.fourth},
            {row3.first, row3.second, row3.fourth}
        };
        out_mat.row3.fourth = static_cast<T>(-inv_det * Matrix3<T>::Determinant(adj_mat)); 

        // Row 4
        adj_mat = {
            {row2.first, row2.second, row2.third},
            {row3.first, row3.second, row3.third},
            {row4.first, row4.second, row4.third}
        };
        out_mat.row4.first = static_cast<T>(-inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.first, row1.second, row1.third},
            {row3.first, row3.second, row3.third},
            {row4.first, row4.second, row4.third}
        };
        out_mat.row4.second = static_cast<T>(inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.first, row1.second, row1.third},
            {row2.first, row2.second, row2.third},
            {row4.first, row4.second, row4.third}
        };
        out_mat.row4.third = static_cast<T>(-inv_det * Matrix3<T>::Determinant(adj_mat));

        adj_mat = {
            {row1.first, row1.second, row1.third},
            {row2.first, row2.second, row2.third},
            {row3.first, row3.second, row3.third}
        };
        out_mat.row4.fourth = static_cast<T>(inv_det * Matrix3<T>::Determinant(adj_mat)); 

        return out_mat;
    }


    /// Transpose the current matrix
    template<typename T>
    Matrix4<T> Matrix4<T>::Transpose() const {
        if(std::is_floating_point<T>::value || std::is_integral<T>::value) {
            Matrix4<T> new_mat;
            new_mat.row1 = Vector4<T>{row1.first, row2.first, row3.first, row4.first};
            new_mat.row2 = Vector4<T>{row1.second, row2.second, row3.second, row4.second};
            new_mat.row3 = Vector4<T>{row1.third, row2.third, row3.third, row4.third};
            new_mat.row4 = Vector4<T>{row1.fourth, row2.fourth, row3.fourth, row4.fourth};

            return new_mat;
        }

        return Matrix4<T>{};
    }


#ifdef _DEBUG
    template<typename T>
    void Matrix4<T>::Log(const std::string &desc) {
        std::cout << "MAT_LOG: " <<  desc << std::endl;
        std::cout << row1.first << " | " << row1.second << " | " << row1.third << " | " << row1.fourth << std::endl;
        std::cout << row2.first << " | " << row2.second << " | " << row2.third << " | " << row2.fourth << std::endl;
        std::cout << row3.first << " | " << row3.second << " | " << row3.third << " | " << row3.fourth << std::endl;
        std::cout << row4.first << " | " << row4.second << " | " << row4.third << " | " << row4.fourth << "\n" << std::endl;
    }
#endif
}

#endif
