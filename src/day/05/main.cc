#include "utils.hh"
#define BF_DEBUG
#include "binary-function.hh"
#include <boost/spirit/home/x3.hpp>

namespace x3 = boost::spirit::x3;
namespace ascii = x3::ascii;
using ascii::alpha;
using x3::ulong_long;
using x3::lit;
using x3::lexeme;

template <typename T>
auto attr (T& var) {
	return [&] (auto& ctx) { var = _attr(ctx); };
}

struct Range {
	u64 dst;
	u64 src;
	u64 length;

	template <typename It>
	bool parse (It begin, It end) {
		auto parser = ulong_long[attr(dst)] >> ulong_long[attr(src)] >> ulong_long[attr(length)];
		return x3::phrase_parse(begin, end, parser, ascii::space) && (begin == end);
	}
};

struct Map {
	std::string from;
	std::string to;
	std::vector<Range> mapping;

	bool parse (const std::vector<std::string>& lines) {
		if (lines.size() < 2) {
			fmt::println("parsing failed : not enough lines in map block");
			return false;
		}

		auto parser = lexeme[+alpha][attr(from)] >> "-to-" >> lexeme[+alpha][attr(to)] >> "map:";
		auto begin = lines[0].begin();
		auto end = lines[0].end();
		if (!x3::phrase_parse(begin, end, parser, ascii::space) || (begin != end)) {
			fmt::println("parsing failed : map block header");
			return false;
		}

		for (auto it = lines.begin() + 1; it != lines.end(); ++it) {
			mapping.emplace_back();
			if (!mapping.back().parse(it->begin(), it->end())) {
				fmt::println("parsing failed : ranges");
				return false;
			}
		}

		return true;
	}
};

struct Seeding {
	std::vector<u64> seeds;
	std::vector<Map> maps;

	bool parse (std::istream& in) {
		std::string seed_line;
		if (!std::getline(in, seed_line))
			return false;

		auto pusher = [&] (auto& ctx) { seeds.push_back(_attr(ctx)); };
		auto parser = "seeds:" >> +(ulong_long[pusher]);
		auto begin = seed_line.begin();
		auto end = seed_line.end();
		if (!x3::phrase_parse(begin, end, parser, ascii::space) || (begin != end)) {
			fmt::println("parsing failed : seed list");
			return false;
		}

		std::string line;
		std::vector<std::string> map_lines;

		while (true) {
			bool stop = !std::getline(in, line);

			if (stop || line.size() == 0) {
				if (map_lines.size()) {
					maps.emplace_back();
					if (!maps.back().parse(map_lines))
						return false;
					map_lines.clear();
				}

				if (stop)
					break;

				continue;
			}

			map_lines.emplace_back(std::move(line));
		}

		return true;
	}
};

struct Transformer : BinaryFunction {
	u64 offset;

	Transformer (const Range& range)
		: BinaryFunction(range.src, range.src + range.length)
		, offset(range.dst - range.src)
	{}
};

struct SeedTransformer {
	std::string to;
	std::vector<Transformer> transformers;

	SeedTransformer (const Map& map)
		: to(map.to) {
		for (const Range& range : map.mapping)
			transformers.emplace_back(range);
	}
};

using Lookup = std::map<std::string, SeedTransformer>;

u64 find_location (BinaryFunction current, const Lookup& strans) {
	const std::string final_stage = "location";
	std::string stage = "seed";

	while (stage != final_stage) {

		auto found_strans = strans.find(stage);
		if (found_strans == strans.end()) {
			fmt::println("bad stage name : {}", stage);
			return 0;
		}
		const SeedTransformer* st = &found_strans->second;
		const u64 N = st->transformers.size();

		// contains : {current, transformed current through mappings * ...}
		std::vector<BinaryFunction> next_offsets;
		next_offsets.reserve(N + 1);
		next_offsets.emplace_back(current);

		// * = on src offset
		for (const Transformer& transfo : st->transformers) {
			BinaryFunction& next_offset = next_offsets.emplace_back();
			BinaryFunction::scan({&current, (BinaryFunction*)&transfo}, BinaryFunction::all_scanner(next_offset));
		}

		// current - transformed
		BinaryFunction current_removed;
		BinaryFunction::scan(next_offsets, [&] (u64 begin, u64 end, auto&& args) {
			if (!args[0])
				return;
			bool taken = false;
			for (auto it = args.begin() + 1; it != args.end(); ++it)
				taken |= *it;
			if (!taken)
				current_removed.push_back(begin, end);
		});

		// * = on dst offset
		for (u64 i = 0; i < N; ++i) {
			next_offsets[i + 1].shift(st->transformers[i].offset);
		}

		current = BinaryFunction();
		next_offsets[0] = current_removed;
		// {removed, transformed on dst ...}
		BinaryFunction::scan(next_offsets, BinaryFunction::any_scanner(current));
		stage = st->to;
	}

	if (current.toggles.size() == 0)
		throw BinaryFunctionError{};

	return current.toggles[0];
}

int solve () {
	Seeding seeding;
	if (!seeding.parse(std::cin))
		return 1;

	Lookup strans;
	for (Map& map : seeding.maps)
		strans.emplace(std::make_pair(map.from, SeedTransformer(map)));

	std::vector<BinaryFunction> build_silver;
	for (u64 seed : seeding.seeds)
		build_silver.emplace_back(seed, seed + 1);
	BinaryFunction silver;
	BinaryFunction::scan(build_silver, BinaryFunction::any_scanner(silver));
	fmt::println("{}", find_location(silver, strans));

	if (seeding.seeds.size() % 2 != 0) {
		fmt::println("bad seeds count");
		return 1;
	}

	std::vector<BinaryFunction> build_gold;
	for (u64 i = 0; i < seeding.seeds.size(); i += 2)
		build_gold.emplace_back(seeding.seeds[i], seeding.seeds[i] + seeding.seeds[i + 1]);
	BinaryFunction gold;
	BinaryFunction::scan(build_gold, BinaryFunction::any_scanner(gold));
	fmt::println("{}", find_location(gold, strans));

	return 0;
}

int main (int argc, char** argv) {
	(void)argc;
	(void)argv;

	try {
		return solve();
	} catch (const BinaryFunctionError& e) {
		fmt::println("binary function misuse");
	}

	return 0;
}
