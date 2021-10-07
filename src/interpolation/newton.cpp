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
#include <cstring>

struct point {
    double x;
    double y;
};


struct dif_ratio {
    double val;
    unsigned int min;
    unsigned int max;
};


// O(n^2)
double n_int(std::vector<point> &pts, int n, double x) {
    // loo tabel koikidest voimalikest esimest järku diferentssuhetest
    //dratio difs[n][n];
    dif_ratio difs[n];

    double val = 0.0;
    double base = 1;
    memset(difs, 0, n * sizeof(dif_ratio));


    for(int i = 0; i < n; i++) {
        // taida tabeli iga n-o rida i astme diferentssuhtega
        for(int j = 0; j < n - i; j++) {

            // juhul kui on tegu esimest järgu diferentssuhega
            if(i == 0) {
                difs[j].val = pts[j].y;
                difs[j].min = j;
                difs[j].max = j;
            }
            else {
                difs[j].max = difs[j + 1].max;
                difs[j].val = (difs[j + 1].val - difs[j].val) / (pts[difs[j].max].x - pts[difs[j].min].x);
            }
        }

        val += difs[0].val * base;
        base *= (x - pts[i].x);
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

    std::cout << n_int(pts, n, x) << std::endl;
}
