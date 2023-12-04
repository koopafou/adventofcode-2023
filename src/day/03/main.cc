#include "utils.hh"

struct Coord {
	i64 x;
	i64 y;
};

bool operator< (const Coord& l, const Coord& r) {
	return (l.x == r.x ? l.y < r.y : l.x < r.x);
}

int main (int argc, char** argv) {
	(void)argc;
	(void)argv;

	u64 width = 0;
	std::vector<std::string> input;
	std::string line;
	while (std::getline(std::cin, line) && line.length()) {
		if (width == 0) {
			width = line.length();
		} else if (line.length() != width) {
			fmt::println("bad input, mismatched width");
			return 1;
		}
		input.emplace_back(std::move(line));
	}

	std::array<Coord, 8> dp_list;
	{
		u64 dp_index = 0;
		for (i64 dx = -1; dx <= 1; ++dx)
			for (i64 dy = -1; dy <= 1; ++dy)
				if (dx != 0 || dy != 0) {
					dp_list[dp_index] = Coord{dx, dy};
					++dp_index;
				}
	}

	u64 sum = 0;
	std::map<Coord, std::vector<u64>> gears;

	for (u64 row = 0; row < input.size(); ++row) {
		u64 current = 0;
		bool validated = false;
		std::set<Coord> nearby_gears;

		for (u64 col = 0; col < width; ++col) {
			char c = input[row][col];
			bool num_terminal = true;

			if ('0' <= c && c <= '9') {
				current = 10 * current + c - '0';

				for (Coord dp : dp_list) {
					u64 x = col + (u64)dp.x;
					u64 y = row + (u64)dp.y;

					if (x < width && y < input.size()) {
						char csym = input[y][x];

						if (!(('0' <= csym && csym <= '9') || csym == '.')) {
							validated = true;
							//fmt::println("found adjacent symbol {}", csym);
						}

						if (csym == '*') {
							nearby_gears.insert(Coord{(i64)x, (i64)y});
						}
					}
				}
				num_terminal = false;
			}

			if (col + 1 == width) {
				num_terminal = true;
			}

			if (num_terminal && validated) {
				fmt::println("added {}", current);
				sum += current;
				for (Coord gear : nearby_gears) {
					fmt::println("gear at {}x{}", gear.x, gear.y);
					gears[gear].push_back(current);
				}
			}

			if (num_terminal) {
				validated = false;
				current = 0;
				nearby_gears.clear();
			}
		}
	}

	fmt::println("{}", sum);

	u64 ratio = 0;

	for (auto&& [coord, gear_list] : gears) {
		fmt::println("gear at {}x{} has neighbors : {}", coord.x, coord.y, gear_list);
		if (gear_list.size() == 2) {
			ratio += gear_list[0] * gear_list[1];
		}
	}

	fmt::println("{}", ratio);

	return 0;
}
