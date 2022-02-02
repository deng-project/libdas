/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: Iterators.h - Iterators for point, vector and matrix structures
/// author: Karl-Mihkel Ott

#ifndef ITERATORS_H
#define ITERATORS_H

namespace Libdas {

    /**
     * Iterator class for point and vector structures
     */
    template<typename Val_t>
    class VectorIterator {
        private:
            using pointer = Val_t*;
            using reference = Val_t&;

            pointer m_data;

        public:
            VectorIterator(pointer _data) :
                m_data(_data) {}

            void operator++(int) {
                m_data++;
            }


            bool operator!=(const VectorIterator &_it) const {
                return m_data != _it.m_data;
            }


            bool operator==(const VectorIterator &_it) const {
                return m_data == _it.m_data;
            }


            reference operator*() {
                return *m_data;
            }


            pointer operator->() {
                return m_data;
            }
    };


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
                m_mat_size(_mat_size), m_is_row_major(_is_row_major), m_data(_data) {}

            // a, b, c, d
            // e, f, g, h,
            // i, j, k, l
            // m, n, o, p
            //
            // memory layout:
            //      a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p
            
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


};

#endif
