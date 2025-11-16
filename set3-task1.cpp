#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm>

void intersection(
    double x1, double y1, double r1,
    double x2, double y2, double r2,
    double x3, double y3, double r3,
    int n
) {
    double xmin = std::min({x1 - r1, x2 - r2, x3 - r3});
    double xmax = std::max({x1 + r1, x2 + r2, x3 + r3});
    double ymin = std::min({y1 - r1, y2 - r2, y3 - r3});
    double ymax = std::max({y1 + r1, y2 + r2, y3 + r3});

    double r1sq = r1 * r1;
    double r2sq = r2 * r2;
    double r3sq = r3 * r3;

    static std::mt19937_64 gen(228);
    std::uniform_real_distribution<double> dist_x(xmin, xmax);
    std::uniform_real_distribution<double> dist_y(ymin, ymax);

    long long inside = 0;

    for (int i = 0; i < n; ++i) {
        double x = dist_x(gen);
        double y = dist_y(gen);

        double dx1 = x - x1;
        double dy1 = y - y1;
        if (dx1 * dx1 + dy1 * dy1 > r1sq) continue;

        double dx2 = x - x2;
        double dy2 = y - y2;
        if (dx2 * dx2 + dy2 * dy2 > r2sq) continue;

        double dx3 = x - x3;
        double dy3 = y - y3;
        if (dx3 * dx3 + dy3 * dy3 > r3sq) continue;

        ++inside;
    }

    double rect_area = (xmax - xmin) * (ymax - ymin);
    double area = (static_cast<double>(inside) / static_cast<double>(n)) * rect_area;

    std::cout << std::fixed << std::setprecision(15) << area << '\n';
}


int main() {
    double x1 = 1.0, y1 = 1.0, r1 = 1.0;
    double x2 = 1.5, y2 = 2.0, r2 = std::sqrt(5.0) / 2.0;
    double x3 = 2.0, y3 = 1.5, r3 = std::sqrt(5.0) / 2.0;

    for (int i = 100; i <= 100000; i += 500) {
        std::cout << i << " ";
        intersection(x1, y1, r1, x2, y2, r2, x3, y3, r3, i);
    }
    

    return 0;
}
