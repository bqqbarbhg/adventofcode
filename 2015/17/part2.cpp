#include <algorithm>
#include <numeric>
#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <unordered_map>

void count(int pos, int cur, int target, int *values, int *totals, int size, std::unordered_map<int, int> &stats, int used=0)
{
	if (pos == size)
	{
		if (cur == target)
			stats[used]++;
	}

	if (cur > target)
		return;

	if (cur + totals[pos] < target)
		return;

	int val = values[pos];
	count(pos + 1, cur, target, values, totals, size, stats, used);
	count(pos + 1, cur + val, target, values, totals, size, stats, used + 1);
}

int main(int argc, char **argv)
{
	int target = atoi(argv[1]);
	std::vector<int> values;
	std::vector<int> total;

	std::string line;
	while (std::getline(std::cin, line))
		values.push_back(std::stoi(line));

	total.resize(values.size());

	std::sort(values.begin(), values.end(), [](int a, int b){ return a > b; });
	std::partial_sum(values.rbegin(), values.rend(), total.rbegin());

	std::unordered_map<int, int> stats;
	count(0, 0, target, values.data(), total.data(), values.size(), stats);

	auto min = std::min_element(stats.begin(), stats.end());
	std::cout << min->second << std::endl;

	return 0;
}

