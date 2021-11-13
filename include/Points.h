/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: Points.h - Point data structures definitions
/// author: Karl-Mihkel Ott


#ifndef POINTS_H
#define POINTS_H

#include <type_traits>

namespace Libdas {

    template <typename T> struct Point2D;
    template <typename T> struct Point3D;
    template <typename T> struct Point4D;

    template <typename T>
    struct Point2D {
        T x, y;

        Point2D<T>() {
            if(std::is_floating_point<T>::value || std::is_integral<T>::value)
                x = 0, y = 0;
            else x = T(), y = T();
        }

        Point2D<T>(const Point2D<T> &_val) : x(_val.x), y(_val.y) {}

        Point2D<T>(Point2D<T> &&_val) : x(std::move(_val.x)), y(std::move(_val.y)) {}

        Point2D<T>(T &&_x, T &&_y = T()) : x(std::move(_x)), y(std::move(_y)){}

        Point2D<T>(const T &_val1, const T &_val2 = T()) : x(_val1), y(_val2) {}

        Point2D<T> operator+(const Point2D<T> &_pt) const {
            return Point2D<T>{x + _pt.x, y + _pt.y};
        }

        void operator=(const Point2D<T> &_pt) {
            x = _pt.x;
            y = _pt.y;
        }

        void operator=(Point2D<T> &&_pt) {
            x = std::move(_pt.x);
            y = std::move(_pt.y);
        }

        void operator+=(const Point2D<T> &_pt) {
            x += _pt.x;
            y += _pt.y;
        }

        Point2D<T> operator-(const Point2D<T> &_pt) const {
            return Point2D<T>{x - _pt.x, y - _pt.y};
        }

        void operator-=(const Point2D<T> &_pt) {
            x -= _pt.x;
            y -= _pt.y;
        }

        bool operator==(const Point2D<T> &_pt) const {
            return x == _pt.x && y == _pt.y;
        }
    };


    template <typename T>
    struct Point3D {
        T x, y, z;

        Point3D<T>() {
            if(std::is_floating_point<T>::value || std::is_integral<T>::value)
                x = 0, y = 0, z = 0;
            else x = T(), y = T(), z = T();
        }

        Point3D<T>(const Point3D<T> &_val) : x(_val.x), y(_val.y) {}

        Point3D<T>(Point3D<T> &&_val) : x(std::move(_val.x)), y(std::move(_val.y)) {}

        Point3D<T>(T &&_x, T &&_y = T(), T &&_z = T()) : x(std::move(_x)), y(std::move(_y)), z(std::move(_z)) {}

        Point3D<T>(const T &_x, const T &_y = T(), const T &_z = T()) : x(_x), y(_y), z(_z) {}

        Point3D<T> operator+(const Point3D<T> &_pt) const {
            return Point3D<T>{_pt.x + x, y + _pt.y, z + _pt.z};
        }

        void operator=(const Point3D<T> &_pt) {
            x = _pt.x;
            y = _pt.y;
            z = _pt.z;
        }

        void operator=(Point3D<T> &&_pt) {
            x = std::move(_pt.x);
            y = std::move(_pt.y);
            z = std::move(_pt.z);
        }

        void operator+=(const Point3D<T> &_pt) {
            x += _pt.x;
            y += _pt.y;
            z += _pt.z;
        }

        Point3D<T> operator-(const Point3D<T> &_pt) const {
            return Point3D<T>{x - _pt.x, y - _pt.y, z - _pt.z};
        }

        void operator-=(const Point3D<T> &_pt) {
            x -= _pt.x;
            y -= _pt.y;
            z -= _pt.z;
        }

        bool operator==(const Point3D<T> &_pt) const {
            return x == _pt.x && y == _pt.y && z == _pt.z;
        }

        // casting operator
        operator Point4D<T>() const {
            return Point4D<T>{x, y, z, Point4D<T>::GetNormalizedValue()};
        }
    };


    template <typename T>
    struct Point4D {
        T x, y, z, w;

        Point4D<T>() {
            if(std::is_floating_point<T>::value || std::is_integral<T>::value)
                x = 0, y = 0, z = 0, w = 1;
            else x = T(), y = T(), z = T(), w = T();
        }

        Point4D<T>(const Point4D<T> &_val) : x(_val.x), y(_val.y) {}

        Point4D<T>(Point4D<T> &&_val) : x(std::move(_val.x)), y(std::move(_val.y)) {}

        Point4D<T>(T &&_x, T &&_y = T(), T &&_z = T(), T &&_w = T()) : 
            x(std::move(_x)), y(std::move(_y)), z(std::move(_z)), w(std::move(_w)) {}

        Point4D<T>(const T &_x, const T &_y = T(), const T &_z = T(), const T &_w = T()) : 
            x(_x), y(_y), z(_z), w(_w) {}

        Point4D<T>(const Point3D<T> &_pt) {
            x = _pt.x;
            y = _pt.y;
            z = _pt.z;

            if(std::is_floating_point<T>::value || std::is_integral<T>::value)
                w = 1;
            else w = T();
        }

        Point4D<T> operator+(const Point4D<T> &_pt) const {
            return Point4D<T>{x + _pt.x, y + _pt.y, z + _pt.z, w + _pt.w};
        }

        void operator=(const Point4D<T> &_pt) {
            x = _pt.x;
            y = _pt.y;
            z = _pt.z;
            w = _pt.w;
        }

        void operator=(Point4D<T> &&_pt) {
            x = std::move(_pt.x);
            y = std::move(_pt.y);
            z = std::move(_pt.z);
            w = std::move(_pt.w);
        }

        void operator+=(const Point4D<T> &_pt) {
            x += _pt.x;
            y += _pt.y;
            z += _pt.z;
            w += _pt.w;
        }

        Point4D<T> operator-(const Point4D<T> &_pt) const {
            return Point4D<T>{x - _pt.x, y - _pt.y, z - _pt.z, w - _pt.w};
        }

        void operator-=(const Point4D<T> &_pt) {
            x -= _pt.x;
            y -= _pt.y;
            z -= _pt.z;
            w -= _pt.w;
        }

        bool operator==(const Point4D<T> &_pt) {
            return x == _pt.x && y == _pt.y && z == _pt.z && w == _pt.w;
        }

        static T GetNormalizedValue() {
            if(std::is_floating_point<T>::value || std::is_integral<T>::value)
                return 1;
            else return T();
        }
    };
}

#endif
