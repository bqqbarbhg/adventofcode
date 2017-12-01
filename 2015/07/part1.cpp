#include "shared.cpp"

int main(int argc, char **argv)
{
	node_map nodes = parse_nodes(std::cin);
	simulate(nodes);

	auto result = nodes.find("a");
	if (result == nodes.end()) throw std::runtime_error("Node a does not exist");
	std::cout << result->second.value() << std::endl;
}

