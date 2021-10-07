/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: lagrange.cpp - Lagrange interpolation testing algorithm
/// author: Karl-Mihkel Ott

// INPUT:
// First line has parameter n, specifying the total amount of vertices used and parameter m, specifying the x 
// value to find interpolated y value
//   The next n lines are whitespace separated floating point values of x and y of the point 
//
// EXAMPLE:
// 5 1.5
// 1 2
// 2 0.5
// 3.4 4
// 4 8
// 5 1

// OUTPUT:
// A single floating point number specifying the interpolated y value

#include <iostream>
#include <vector>

struct point {
    double x;
    double y;
};


double l_int(std::vector<point> &pts, int n, double x) {
    double val = 0.0;

    for(int i = 0; i < n; i++) {
        double L_num = 1, L_denom = 1;
        for(int j = 0; j < n; j++) {
            if(i != j) {
                L_num *= (x - pts[j].x);
                L_denom *= pts[i].x - pts[j].x;
            }
        }

        val += L_num / L_denom * pts[i].y;
    }

    return val;
}


int main() {
    int n;
    double x;

    // read input
    std::cin >> n;
    std::cin >> x;
    std::vector<point> pts(n);

    for(int i = 0; i < n; i++) {
        std::cin >> pts[i].x;
        std::cin >> pts[i].y;
    }

    std::cout << l_int(pts, n, x) << std::endl;
    
    return 0;
}
