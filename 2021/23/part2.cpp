#include <stdint.h>
#include <span>
#include <queue>
#include <unordered_set>
#include <stdint.h>

#if defined(_MSC_VER)
	#include <intrin.h>
	__forceinline static uint32_t ctz32(uint32_t mask) {
		unsigned long index;
		_BitScanForward(&index, mask);
		return (uint32_t)index;
	}
#else
	#define ctz32 __builtin_ctz
#endif

struct Pos
{
	int8_t x, y;
	Pos operator+(Pos rhs) const { return { (int8_t)(x + rhs.x), (int8_t)(y + rhs.y) }; }
};

static constexpr const uint8_t height = 7;
static constexpr const uint8_t width = 14;
static constexpr const uint8_t num_pods = 16;

static const Pos cardinals[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
static const size_t step_cost[16] = { 1, 1, 1, 1, 10, 10, 10, 10, 100, 100, 100, 100, 1000, 1000, 1000, 1000 };
static const uint8_t room_x[16] = { 3, 3, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7, 9, 9, 9, 9 };
static const uint8_t room_y = 2;
static const char names[] = "AAAABBBBCCCCDDDD";

struct Path
{
	Pos pos;
	uint8_t steps;
};

struct MapHash
{
	uint64_t a, b, c;
	bool operator==(const MapHash &rhs) const { return a == rhs.a && b == rhs.b && c == rhs.c; }
	bool operator!=(const MapHash &rhs) const { return !(*this == rhs); }
};

struct MapHasher
{
	std::size_t operator()(const MapHash &mh) const noexcept {
		uint64_t h = 0;
		h = std::hash<uint64_t>()(mh.a) + 0x9e3779b9 + (h<<6) + (h>>2);
		h = std::hash<uint64_t>()(mh.b) + 0x9e3779b9 + (h<<6) + (h>>2);
		h = std::hash<uint64_t>()(mh.c) + 0x9e3779b9 + (h<<6) + (h>>2);
		return h;
	}
};

struct Map
{
	Pos pods[num_pods];
	uint16_t pods_to_move;
	uint16_t pods_to_return;
	size_t cost;
	char grid[height][width];

	inline void set(Pos pos, char ch) { grid[pos.y][pos.x] = ch; }
	inline char get(Pos pos) const { return grid[pos.y][pos.x]; }
	inline bool is_wall(Pos pos) const { return get(pos) == '#'; }
	inline bool is_free(Pos pos) const { return get(pos) == '.'; }

	inline void move(size_t index, Pos pos, size_t steps) {
		set(pods[index], '.');
		set(pos, names[index]);
		pods[index] = pos;
		cost += (size_t)step_cost[index] * steps;
	}
};

MapHash map_to_hash(const Map &map)
{
	uint64_t a = 0, b = 0, c = 0;
	for (size_t index = 0; index < 8; index++) {
		Pos p1 = map.pods[index], p2 = map.pods[index + 8];
		a |= (uint64_t)(((uint32_t)p1.x&0xfu) | ((uint32_t)p1.y&0xfu) << 4u) << (index * 8);
		b |= (uint64_t)(((uint32_t)p2.x&0xfu) | ((uint32_t)p2.y&0xfu) << 4u) << (index * 8);
	}
	c = (uint64_t)map.pods_to_move | (uint64_t)map.pods_to_return << 16;
	return { a, b, c };
}

std::span<Path> find_paths(const Map &map, Pos start, Path buffer[64])
{
	bool seen[height][width] = { 0 };
	size_t count = 1, explored = 0;
	buffer[0] = { start, 0 };
	while (explored < count) {
		Path path = buffer[explored++];
		for (Pos step : cardinals) {
			Pos next = path.pos + step;
			if (!map.is_free(next)) continue;
			if (seen[next.y][next.x]) continue;
			seen[next.y][next.x] = true;
			buffer[count++] = { next, (uint8_t)(path.steps + 1) };
		}
	}
	return { buffer + 1, count - 1 };
}

std::span<Map> neighbors(const Map &map, Map buffer[256])
{
	size_t count = 0;
	Path path_buf[64];

	uint32_t to_move = map.pods_to_move;
	while (to_move) {
		uint32_t index = ctz32(to_move);
		to_move &= to_move - 1;
		for (Path path : find_paths(map, map.pods[index], path_buf)) {
			if (!map.is_wall(path.pos + Pos{0,1})) continue;
			if (path.pos.y >= room_y) continue;
			Map &dst = buffer[count++];
			dst = map;
			dst.pods_to_move &= ~(1u << index);
			dst.pods_to_return |= 1u << index;
			dst.move(index, path.pos, path.steps);
		}
	}

	uint32_t to_return = map.pods_to_return;
	while (to_return) {
		uint32_t index = ctz32(to_return);
		to_return &= to_return - 1;
		for (Path path : find_paths(map, map.pods[index], path_buf)) {
			if (path.pos.x != room_x[index]) continue;
			if (path.pos.y < room_y) continue;
			char below = map.get(path.pos + Pos{0,1});
			if (below != '#' && below != names[index]) continue;
			Map &dst = buffer[count++];
			dst = map;
			dst.pods_to_return |= 1u << index;
			dst.move(index, path.pos, path.steps);
		}
	}

	return { buffer, count };
}

size_t heuristic(const Map &map)
{
	size_t score = 0;
	for (size_t index = 0; index < num_pods; index++) {
		int dx = (int)map.pods[index].x - (int)room_x[index];
		if (dx != 0) {
			score += (1+std::abs(dx)) * step_cost[index];
		}
		if (dx != 0 && (map.pods_to_move & (1u << index)) != 0) {
			score += ((map.pods[index].y - room_y) + 1) * step_cost[index];
		}
	}
	return score;
}

bool is_solved(const Map &map)
{
	for (size_t index = 0; index < num_pods; index++) {
		if (map.pods[index].y < room_y) return false;
		if (map.pods[index].x != room_x[index]) return false;
	}
	return true;
}

struct State
{
	size_t score;
	Map map;

	bool operator<(const State &rhs) const { return score > rhs.score; }
};

size_t search_best(const Map &init_map)
{
	Map map_buffer[256];

	std::priority_queue<State> states;
	std::unordered_set<MapHash, MapHasher> seen;

	states.push({ 0, init_map });
	while (!states.empty()) {
		State state = states.top();
		states.pop();

		MapHash mh = map_to_hash(state.map);
		if (!seen.insert(mh).second) continue;

		if (is_solved(state.map)) {
			return state.score;
		}

		for (const Map &neighbor : neighbors(state.map, map_buffer)) {
			size_t hcost = neighbor.cost + heuristic(neighbor);
			states.push({ hcost, neighbor });
		}
	}

	return SIZE_MAX;
}

int main(int argc, char **argv)
{
	char line[16] = { };
	Map map = { };
	uint32_t num_pods[4] = { };
	for (size_t y = 0; y < height; y++) {
		fgets(line, sizeof(line), stdin);
		for (size_t x = 0; x < width; x++) {
			char ch = line[x];
			if (ch == '\0' || ch == '\n' || ch == '\r') break;
			map.grid[y][x] = ch;
			uint32_t index = (uint32_t)(ch - 'A');
			if (index < 4) {
				index = index * 4 + num_pods[index]++;
				map.pods[index] = { (int8_t)x, (int8_t)y };
				map.pods_to_move |= 1u << index;
			}
		}
	}

	size_t cost = search_best(map);
	printf("%zu\n", cost);

	return 0;
}
