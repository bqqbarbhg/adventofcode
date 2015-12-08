#include <iostream>
#include <string>
#include <cstdint>
#include <regex>
#include <vector>
#include <unordered_map>
#include <utility>

enum class Operator
{
	Undefined,
	Set,
	Not,
	And,
	Or,
	LShift,
	RShift,
};

class Node
{
public:
	uint16_t inputs[2];
	uint16_t output;
	int wait_count;
	bool calculated;
	Operator op;

	std::vector<std::pair<std::string, int>> uses;

	Node()
		: wait_count(2)
		, calculated(false)
		, op(Operator::Undefined)
	{
	}

	void set_input(int index, uint16_t value)
	{
		inputs[index] = value;
		wait_count--;
	}

	void add_use(std::string name, int index)
	{
		uses.push_back({std::move(name), index});
	}

	uint16_t calculate_value() const
	{
		if (wait_count > 0) throw std::runtime_error("Inputs are not ready");

		switch (op) {
		case Operator::Set: return inputs[1];
		case Operator::Not: return ~inputs[1];
		case Operator::And: return inputs[0] & inputs[1];
		case Operator::Or: return inputs[0] | inputs[1];
		case Operator::LShift: return inputs[0] << inputs[1];
		case Operator::RShift: return inputs[0] >> inputs[1];
		default: throw std::runtime_error("Unexpected operation");
		}
	}

	uint16_t calculate()
	{
		calculated = true;
		return output = calculate_value();
	}

	uint16_t value() const
	{
		if (!calculated) throw std::runtime_error("Node not calculated");
		return output;
	}

	bool is_ready() const { return wait_count == 0; }
};

typedef std::unordered_map<std::string, Node> node_map;

node_map parse_nodes(std::istream& input)
{
	const std::unordered_map<std::string, Operator> operators = {
		{ "AND", Operator::And },
		{ "OR", Operator::Or },
		{ "LSHIFT", Operator::LShift },
		{ "RSHIFT", Operator::RShift },
		{ "NOT", Operator::Not },
	};

	node_map nodes;

	//                                      v-1-v v---2--v      v---3--v        v-4-v v---5--v         v---6--v
	const std::regex pattern{R"(\s*(?:(?:(?:(\d+)|([a-z]+))\s+)?([A-Z]+)\s+)?(?:(\d+)|([a-z]+))\s+->\s+([a-z]+)\s*)",
		std::regex_constants::ECMAScript };

	std::string line;
	for (;;) {
		std::getline(input, line);
		if (!input.good()) break;

		std::smatch match;
		if (!std::regex_match(line, match, pattern))
			continue;

		std::string output_name = match[6];
		Node& output = nodes[output_name];

		if (match[3].length() > 0) {
			auto it = operators.find(match[3].str());
			if (it == operators.end()) throw std::runtime_error("Unexpected operator");
			output.op = it->second;
		} else {
			output.op = Operator::Set;
		}

		if (match[1].length() > 0) {
			output.set_input(0, (uint16_t)std::stoi(match[1].str()));
		} else if (match[2].length() > 0) {
			nodes[match[2].str()].add_use(output_name, 0);
		} else {
			output.set_input(0, 0);
		}

		if (match[4].length() > 0) {
			output.set_input(1, (uint16_t)std::stoi(match[4].str()));
		} else if (match[5].length() > 0) {
			nodes[match[5].str()].add_use(output_name, 1);
		} else {
			throw std::runtime_error("Should never get here");
		}
	}

	return nodes;
}

void simulate(node_map& nodes)
{
	std::vector<Node*> work_list, next_work;

	for (auto& pair : nodes) {
		if (pair.second.is_ready()) {
			next_work.push_back(&pair.second);
		}
	}

	while (!next_work.empty()) {
		work_list.swap(next_work);
		next_work.clear();

		for (auto node_ptr : work_list) {
			auto& node = *node_ptr;
			uint16_t value = node.calculate();

			for (auto use : node.uses) {
				auto it = nodes.find(use.first);
				if (it == nodes.end()) throw std::runtime_error("Node not found (should not happen)");
				auto& dependent = it->second;

				dependent.set_input(use.second, value);
				if (dependent.is_ready()) {
					next_work.push_back(&dependent);
				}
			}
		}
	}
}

