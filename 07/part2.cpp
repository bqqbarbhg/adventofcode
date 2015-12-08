#include "shared.cpp"

int main(int argc, char **argv)
{
	node_map nodes = parse_nodes(std::cin);
	node_map second_nodes = nodes;

	simulate(nodes);

	auto result = nodes.find("a");
	if (result == nodes.end()) throw std::runtime_error("Node a does not exist");
	const Node &a = result->second;

	// HACK: Override the B node.
	Node &b = second_nodes["b"];
	b.wait_count = 0;
	b.op = Operator::Set;
	b.inputs[1] = a.value();

	simulate(second_nodes);

	auto second_result = second_nodes.find("a");
	if (second_result == second_nodes.end()) throw std::runtime_error("Node a does not exist");

	std::cout << second_result->second.value() << std::endl;
}

