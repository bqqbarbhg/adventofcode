#include <algorithm>
#include <numeric>
#include <iostream>
#include <cstring>
#include <vector>
#include <string>

int count(int pos, int cur, int target, int *values, int *totals, int size)
{
	if (pos == size)
		return cur == target ? 1 : 0;

	if (cur > target)
		return 0;

	if (cur + totals[pos] < target)
		return 0;

	int val = values[pos];
	return count(pos + 1, cur, target, values, totals, size)
		+ count(pos + 1, cur + val, target, values, totals, size);
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

	std::cout << count(0, 0, target, values.data(), total.data(), values.size()) << std::endl;

	return 0;
}

