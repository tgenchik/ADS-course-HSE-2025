#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

struct ArrayGenerator {
  std::vector<long long> base_random;
  std::vector<long long> base_reverse;
  std::vector<long long> base_almost;

  ArrayGenerator(size_t max_n, int min_value, int max_value,
                 std::uint64_t seed = 42) {
    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<long long> dist(min_value, max_value);

    base_random.resize(max_n);
    for (size_t i = 0; i < max_n; ++i) {
      base_random[i] = dist(gen);
    }

    std::vector<long long> sorted = base_random;
    std::sort(sorted.begin(), sorted.end());

    base_reverse = sorted;
    std::reverse(base_reverse.begin(), base_reverse.end());

    base_almost = sorted;
    size_t swaps = max_n / 100;
    if (swaps == 0) {
      swaps = 1;
    }
    for (size_t i = 0; i < swaps; ++i) {
      size_t a = gen() % max_n;
      size_t b = gen() % max_n;
      std::swap(base_almost[a], base_almost[b]);
    }
  }

  std::vector<long long> get_random(size_t n) {
    return std::vector<long long>(base_random.begin(), base_random.begin() + n);
  }

  std::vector<long long> get_reverse(size_t n) {
    return std::vector<long long>(base_reverse.begin(),
                                  base_reverse.begin() + n);
  }

  std::vector<long long> get_almost_sorted(size_t n) {
    return std::vector<long long>(base_almost.begin(), base_almost.begin() + n);
  }
};

struct SortTester {
  enum ArrayType { RandomArray, ReverseArray, AlmostSortedArray };

  ArrayGenerator &generator;
  int repeats;

  static void insertion_sort(std::vector<long long> &a, int l, int r) {
    for (int i = l + 1; i <= r; ++i) {
      long long key = a[i];
      int j = i - 1;
      while (j >= l && a[j] > key) {
        a[j + 1] = a[j];
        --j;
      }
      a[j + 1] = key;
    }
  }

  static void merge(std::vector<long long> &a, std::vector<long long> &tmp,
                    int l, int m, int r) {
    int i = l;
    int j = m + 1;
    int k = l;

    while (i <= m && j <= r) {
      if (a[i] <= a[j]) {
        tmp[k++] = a[i++];
      } else {
        tmp[k++] = a[j++];
      }
    }
    while (i <= m) {
      tmp[k++] = a[i++];
    }
    while (j <= r) {
      tmp[k++] = a[j++];
    }
    for (int p = l; p <= r; ++p) {
      a[p] = tmp[p];
    }
  }

  static void merge_sort_rec(std::vector<long long> &a,
                             std::vector<long long> &tmp, int l, int r) {
    if (l >= r) {
      return;
    }
    int m = (l + r) / 2;
    merge_sort_rec(a, tmp, l, m);
    merge_sort_rec(a, tmp, m + 1, r);
    merge(a, tmp, l, m, r);
  }

  static void merge_sort(std::vector<long long> &a) {
    if (a.empty()) {
      return;
    }
    std::vector<long long> tmp(a.size());
    merge_sort_rec(a, tmp, 0, static_cast<int>(a.size()) - 1);
  }

  static void merge_sort_hybrid_rec(std::vector<long long> &a,
                                    std::vector<long long> &tmp, int l, int r,
                                    int threshold) {

    if (r - l + 1 <= threshold) {
      insertion_sort(a, l, r);
      return;
    }
    int m = (l + r) / 2;
    merge_sort_hybrid_rec(a, tmp, l, m, threshold);
    merge_sort_hybrid_rec(a, tmp, m + 1, r, threshold);
    merge(a, tmp, l, m, r);
  }

  static void merge_sort_hybrid(std::vector<long long> &a, int threshold) {
    if (a.empty()) {
      return;
    }
    std::vector<long long> tmp(a.size());
    merge_sort_hybrid_rec(a, tmp, 0, static_cast<int>(a.size()) - 1, threshold);
  }

  std::vector<long long> make_array(size_t n, ArrayType type) {
    if (type == RandomArray) {
      return generator.get_random(n);
    } else if (type == ReverseArray) {
      return generator.get_reverse(n);
    } else {
      return generator.get_almost_sorted(n);
    }
  }

  SortTester(ArrayGenerator &gen, int repeats_count)
      : generator(gen), repeats(repeats_count) {}

  long long measure_merge_ms(size_t n, ArrayType type) {
    long long total = 0;
    for (int t = 0; t < repeats; ++t) {
      std::vector<long long> a = make_array(n, type);

      auto start = std::chrono::high_resolution_clock::now();
      merge_sort(a);
      auto elapsed = std::chrono::high_resolution_clock::now() - start;
      long long msec =
          std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
              .count();

      total += msec;
    }
    return total / repeats;
  }

  long long measure_hybrid_ms(size_t n, ArrayType type, int threshold) {
    long long total = 0;
    for (int t = 0; t < repeats; ++t) {
      std::vector<long long> a = make_array(n, type);

      auto start = std::chrono::high_resolution_clock::now();
      merge_sort_hybrid(a, threshold);
      auto elapsed = std::chrono::high_resolution_clock::now() - start;
      long long msec =
          std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
              .count();

      total += msec;
    }
    return total / repeats;
  }
};

int main() {
  const size_t max_n = 100000;
  const int min_value = 0;
  const int max_value = 6000;
  const int repeats = 5;

  ArrayGenerator generator(max_n, min_value, max_value, 42);
  SortTester tester(generator, repeats);

  std::vector<int> thresholds;
  thresholds.push_back(5);
  thresholds.push_back(10);
  thresholds.push_back(15);
  thresholds.push_back(20);
  thresholds.push_back(30);
  thresholds.push_back(50);

  for (size_t n = 500; n <= max_n; n += 100) {
    long long rand_merge_time =
        tester.measure_merge_ms(n, SortTester::RandomArray);
    std::vector<long long> rand_hybrid_times(thresholds.size());
    for (size_t i = 0; i < thresholds.size(); ++i) {
      rand_hybrid_times[i] =
          tester.measure_hybrid_ms(n, SortTester::RandomArray, thresholds[i]);
    }

    long long rev_merge_time =
        tester.measure_merge_ms(n, SortTester::ReverseArray);
    std::vector<long long> rev_hybrid_times(thresholds.size());
    for (size_t i = 0; i < thresholds.size(); ++i) {
      rev_hybrid_times[i] =
          tester.measure_hybrid_ms(n, SortTester::ReverseArray, thresholds[i]);
    }

    long long almost_merge_time =
        tester.measure_merge_ms(n, SortTester::AlmostSortedArray);
    std::vector<long long> almost_hybrid_times(thresholds.size());
    for (size_t i = 0; i < thresholds.size(); ++i) {
      almost_hybrid_times[i] = tester.measure_hybrid_ms(
          n, SortTester::AlmostSortedArray, thresholds[i]);
    }

    std::cout << n << "," << rand_merge_time;
    for (size_t i = 0; i < thresholds.size(); ++i) {
      std::cout << "," << rand_hybrid_times[i];
    }

    std::cout << "," << rev_merge_time;
    for (size_t i = 0; i < thresholds.size(); ++i) {
      std::cout << "," << rev_hybrid_times[i];
    }

    std::cout << "," << almost_merge_time;
    for (size_t i = 0; i < thresholds.size(); ++i) {
      std::cout << "," << almost_hybrid_times[i];
    }

    std::cout << "\n";
  }

  return 0;
}
