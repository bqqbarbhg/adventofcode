#include <algorithm>
#include <deque>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

template <typename T, typename S> inline T read(S &s) {
    T t; s >> t; return t;
}
template <typename T> inline T parse(const std::string &s) {
    std::stringstream ss{s}; return read<T>(ss);
}

struct int_window
{
    std::deque<int> queue;
    std::unordered_multiset<int> sums;

    void push_back(int a) {
        for (int &b : queue) {
            sums.insert(a + b);
        }
        queue.push_back(a);
    }

    void pop_front() {
        int a = queue.front();
        queue.pop_front();
        for (int &b : queue) {
            sums.erase(sums.find(a + b));
        }
    }

    bool contains(int a) const {
        return sums.find(a) != sums.end();
    }
};

template <typename It>
int find_part1(It begin, It end, size_t window_size) {
    int_window window;

    for (size_t i = 0; i < window_size; i++) {
        window.push_back(*begin);
        ++begin;
    }

    for (; begin != end; ++begin) {
        int val = *begin;
        if (!window.contains(val)) {
            return val;
        }
        window.pop_front();
        window.push_back(val);
    }

    return -1;
}

int main(int argc, char **argv)
{
    size_t window_size = parse<size_t>(argv[1]);

    std::vector<int> input;
    while (std::cin.good()) {
        input.push_back(read<int>(std::cin));
    }

    int target = find_part1(input.begin(), input.end(), window_size);

    std::vector<int> prefix_sum;
    std::partial_sum(input.begin(), input.end(), std::back_inserter(prefix_sum));

    for (auto lo = prefix_sum.begin(); lo != prefix_sum.end(); ++lo) {
        size_t lo_ix = lo - prefix_sum.begin();
        int rest = (*lo - input[lo_ix]) + target;
        if (rest <= 0) continue;

        auto hi = std::equal_range(lo, prefix_sum.end(), rest);
        if (hi.first == hi.second) continue;
        size_t hi_ix = hi.first - prefix_sum.begin();

        int min = *std::min_element(input.begin() + lo_ix, input.begin() + hi_ix);
        int max = *std::max_element(input.begin() + lo_ix, input.begin() + hi_ix);
        std::cout << (min + max) << std::endl;
        return 0;
    }

    std::cerr << "Not found!" << std::endl;
    return 1;
}
