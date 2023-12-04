#include "utils.hh"
#include <boost/spirit/home/x3.hpp>

struct Game {
	u64 id;
	std::vector<std::array<u64, 3>> picks;

	using It = std::string::const_iterator;

	bool parse (It begin, It end) {
		namespace x3 = boost::spirit::x3;
		namespace ascii = boost::spirit::x3::ascii;
		using x3::int_;
		using x3::lit;

		int color_count = 0;
		std::array<u64, 3> current_pick{0, 0, 0};

		auto assign = [] (auto& var) {
			return [&] (auto& ctx) { var = _attr(ctx); };
		};

		auto color = [&] (const char* name, int index) {
			auto action = [&, index] (auto&) { current_pick[index] = color_count; };
			return lit(name)[action];
		};

		auto action_pick = [&] (auto&) {
			picks.push_back(current_pick);
			current_pick = std::array<u64, 3>{0, 0, 0};
		};

		auto parser = ("Game" >> int_[assign(id)] >> ":"
		               >> (((int_[assign(color_count)]
		                     >> (color("red", 0)
		                         | color("green", 1)
		                         | color("blue", 2)))
		                    % ",")[action_pick]
		                   % ";"));

		return x3::phrase_parse(begin, end, parser, ascii::space) && (begin == end);
	}
};


int main (int argc, char** argv) {
	(void)argc;
	(void)argv;

	std::vector<Game> games;
	std::array<u64, 3> constraint{12, 13, 14};

	u64 line_index = 0;
	std::string line;
	while (std::getline(std::cin, line) && line.length()) {
		Game g;
		if (g.parse(line.cbegin(), line.cend()))
			games.push_back(g);
		else
			fmt::println("cannot parse line {}", line_index);
		++line_index;
	}

	u64 sum_game = 0;
	u64 sum_power = 0;

	for (const Game& g : games) {
		std::array<u64, 3> min{0, 0, 0};

		for (auto& pick : g.picks) {
			for (u64 i = 0; i < 3; ++i) {
				min[i] = std::max(min[i], pick[i]);
			}
		}

		bool possible = true;
		for (u64 i = 0; i < 3; ++i) {
			possible &= min[i] <= constraint[i];
		}

		if (possible) {
			sum_game += g.id;
		}

		sum_power += min[0] * min[1] * min[2];
	}

	fmt::println("{}", sum_game);
	fmt::println("{}", sum_power);

	return 0;
}
