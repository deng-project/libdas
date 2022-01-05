#include <cmath>
#include <iostream>
#include <random>
#include <Quaternion.h>

int main() {

    const __m128 v1reg = _mm_set_ps(0, vec1[2], vec1[1], vec1[0]);
    const __m128 v2reg = _mm_set_ps(0, vec2[2], vec2[1], vec2[0]);
    const __m128 cross = Libdas::FastCross(v1reg, v2reg);

    float cross_f[4];
    _mm_storeu_ps(cross_f, cross);

    for(int i = 0; i < 4; i++)
        std::cout << cross_f[i] << " ";
    std::cout << std::endl;
    return 0;
}
