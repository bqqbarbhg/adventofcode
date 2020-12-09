#include <algorithm>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

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

int main(int argc, char **argv)
{
    size_t window_size = parse<size_t>(argv[1]);
    int_window window;
    for (size_t i = 0; i < window_size; i++) {
        window.push_back(read<int>(std::cin));
    }

    while (std::cin.good()) {
        int val = read<int>(std::cin);
        if (!window.contains(val)) {
            std::cout << val << std::endl;
            return 0;
        }
        window.pop_front();
        window.push_back(val);
    }

    std::cerr << "Not found!" << std::endl;
    return 1;
}
