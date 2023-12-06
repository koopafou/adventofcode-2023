#include "utils.hh"
#include <boost/spirit/home/x3.hpp>
#include <sstream>

namespace x3 = boost::spirit::x3;
namespace ascii = x3::ascii;
using ascii::alpha;
using x3::ulong_long;
using x3::lit;
using x3::lexeme;

template <typename Parser>
bool parse (const std::string& input, Parser&& parser) {
	auto begin = input.begin();
	auto end = input.end();
	return x3::phrase_parse(begin, end, std::forward<Parser>(parser), x3::space) && (begin == end);
}

u64 solve (u64 t, u64 d) {
	// d = h * (t - h)
	// normalized :
	// hh - ht + d = 0

	// roots for variable h

	if (t * t < 4 * d) {
		fmt::println("can't solve for t={} d={}", t, d);
		return 0;
	}

	u64 margin;

	// newton-raphson should be able to do it on 6 iterations (and zero conditional jumps), but integers are fickle
#if 0
	{
		u64 roots[2] = {0, t};
		for (u64 k = 0; k < 2; ++k) {
			u64& h = roots[k];
			for (u64 iter = 0; iter < 6; ++iter) {
				h = (h * h - d) / (2 * h - t);
			}
		}
		margin = roots[1] - roots[0]; // or some sort of adjustment
	}
#endif

	{
		// so we do binary search instead (64 iterations max)
		auto search = [] (u64 begin, u64 end, auto&& expr) {
			while (begin + 1 < end) {
				u64 mid = (begin + end) / 2;
				if (expr(mid))
					begin = mid;
				else
					end = mid;
			}
			return begin;
		};

		u64 h_optimal = t / 2; // 0 = derivative[h](d) = t - 2h
		u64 roots[2];
		roots[0] = search(0, h_optimal, [&] (u64 h) { return h * t - h * h <= d; });
		roots[1] = search(h_optimal, t, [&] (u64 h) { return d < h * t - h * h; });

		margin = roots[1] - roots[0];
		//fmt::println("t={}, d={} -> [{}, {}] -> {}", t, d, roots[0], roots[1], margin);
	}

	return margin;
}

int main (int argc, char** argv) {
	(void)argc;
	(void)argv;

	u64 N = 0;
	std::vector<u64> times;
	std::vector<u64> distances;

	{
		std::string time_str;
		std::string distance_str;
		if (!std::getline(std::cin, time_str))
			return 1;
		if (!std::getline(std::cin, distance_str))
			return 1;

		auto pusher = [] (auto& v) {
			return [&] (auto& ctx) { v.push_back(_attr(ctx)); };
		};

		if (!parse(time_str, "Time:" >> +ulong_long[pusher(times)]))
			return 1;
		if (!parse(distance_str, "Distance:" >> +ulong_long[pusher(distances)]))
			return 1;

		if (times.size() != distances.size())
			return 1;

		N = times.size();
	}

	u64 prod = 1;

	for (u64 i = 0; i < N; ++i) {
		prod *= solve(times[i], distances[i]);
	}

	fmt::println("{}", prod);

	// ass recombination, will not work if numbers were written with trailing zeroes
	auto recombine = [] (auto&& values) {
		std::stringstream sstr;
		for (u64 v : values)
			sstr << v;
		return std::stoull(sstr.str());
	};

	u64 gold = solve(recombine(times), recombine(distances));
	fmt::println("{}", gold);
	

	return 0;
}
