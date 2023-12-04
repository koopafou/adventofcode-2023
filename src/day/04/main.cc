#include "utils.hh"
#include <boost/spirit/home/x3.hpp>

struct Card {
	u64 id;
	std::set<u64> winning;
	std::vector<u64> have;

	using It = std::string::const_iterator;

	bool parse (It begin, It end) {
		namespace x3 = boost::spirit::x3;
		namespace ascii = boost::spirit::x3::ascii;
		using x3::int_;
		using x3::lit;

		auto assign = [] (auto& var) {
			return [&] (auto& ctx) { var = _attr(ctx); };
		};

		auto push = [] (auto& vector) {
			return [&] (auto& ctx) { vector.push_back(_attr(ctx)); };
		};

		auto insert = [] (auto& set) {
			return [&] (auto& ctx) { set.insert(_attr(ctx)); };
		};

		auto parser = ("Card" >> int_[assign(id)] >> ":"
		               >> +(int_[insert(winning)]) >> "|"
		               >> +(int_[push(have)]))
			;

		return x3::phrase_parse(begin, end, parser, ascii::space) && (begin == end);
	}
};

int main (int argc, char** argv) {
	(void)argc;
	(void)argv;

	std::vector<Card> cards;

	u64 line_index = 0;
	std::string line;
	while (std::getline(std::cin, line) && line.length()) {
		Card cd;
		if (cd.parse(line.cbegin(), line.cend()))
			cards.emplace_back(std::move(cd));
		else
			fmt::println("cannot parse line {}", line_index);
		++line_index;
	}

	u64 sum = 0;
	std::vector<u64> copies(cards.size(), 1);

	for (u64 i = 0; i < cards.size(); ++i) {
		Card& card = cards[i];
		u64 value = 1;
		u64 make_copies = 0;

		for (u64 number : card.have) {
			if (card.winning.contains(number)) {
				value *= 2;
				++make_copies;
			}
		}

		sum += value / 2;

		fmt::println("card {} ({} times) is generating {} copies", i, copies[i], make_copies);

		for (u64 j = i + 1; j < i + 1 + make_copies; ++j) {
			if (copies.size() <= j) {
				fmt::println("they lied");
				break;
			}

			copies[j] += copies[i];
			fmt::println("card {} now has {}", j, copies[j]);
		}
	}

	fmt::println("{}", sum);

	u64 copy_count = 0;
	for (u64 cp : copies) {
		copy_count += cp;
	}

	fmt::println("{}", copy_count);

	return 0;
}
