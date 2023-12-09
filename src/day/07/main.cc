#include "parse-utils.hh"

enum StarMode {
	SM_SILVER,
	SM_GOLD
};

enum HandType {
	HT_HIGH,
	HT_PAIR,
	HT_BIPAIR,
	HT_3KIND,
	HT_HOUSE,
	HT_4KIND,
	HT_5KIND,
	HT_N
};

const std::string& hand_name (HandType type) {
	static std::vector<std::string> names{"high", "pair", "bipair", "3 kind", "full house", "4 kind", "5 kind", "error"};
	return names[type];
}

u64 card_value (StarMode sm, char c) {
	static std::map<char, u64> cvs[2];

	{
		auto& cv = cvs[SM_SILVER];
		if (cv.size() == 0) {
			for (u64 i = 2; i < 10; ++i)
				cv[i + '0'] = i;
			cv['T'] = 10;
			cv['J'] = 11;
			cv['Q'] = 12;
			cv['K'] = 13;
			cv['A'] = 14;
		}
	}

	{
		auto& cv = cvs[SM_GOLD];
		if (cv.size() == 0) {
			for (u64 i = 2; i < 10; ++i)
				cv[i + '0'] = i;
			cv['T'] = 10;
			cv['J'] = 0; //
			cv['Q'] = 12;
			cv['K'] = 13;
			cv['A'] = 14;
		}
	}

	auto& cv = cvs[sm];
	auto it = cv.find(c);
	return it == cv.end() ? 0 : it->second;
}

struct Hand {
	std::string cards_raw;
	u64 bid;

	HandType type;
	std::array<u64, 5> cards_value;

	Hand (const std::string& line, StarMode sm) {
		auto parser = lexeme[+alnum][assigner(cards_raw)] >> ulong_long[assigner(bid)];
		parse(line, parser);

		if (cards_raw.size() != 5)
			throw ParseError("bad card count");

		std::map<u64, u64> value_count;
		for (u64 i = 0; i < 5; ++i) {
			cards_value[i] = card_value(sm, cards_raw[i]);
			++value_count[cards_value[i]];
		}

		bool k5 = false;
		bool k4 = false;
		bool k3 = false;
		u64 k2 = 0;
		for (auto&& [value, count] : value_count) {
			switch (count) {
			case 2: ++k2; break;
			case 3: k3 = true; break;
			case 4: k4 = true; break;
			case 5: k5 = true; break;
			default:;
			}
		}

		type = HT_HIGH;
		if (k5) type = HT_5KIND;
		if (k4) type = HT_4KIND;
		if (k3 && k2 == 1) type = HT_HOUSE;
		if (k3 && k2 == 0) type = HT_3KIND;
		if (k2 == 2) type = HT_BIPAIR;
		if (!k3 && k2 == 1) type = HT_PAIR;

		if (sm == SM_GOLD) {
			u64 njoker = value_count[card_value(sm, 'J')];

			HandType promote[HT_N][6] = {
				// high card, 0/1 jokers
				{HT_HIGH, HT_PAIR, HT_N, HT_N, HT_N, HT_N},
				// single pair, 0/1/2 jokers, if joker pair then it's the og pair
				{HT_PAIR, HT_3KIND, HT_3KIND, HT_N, HT_N, HT_N},
				// two pairs, 0/1/2 jokers, if joker pair then it's one of the og pairs
				{HT_BIPAIR, HT_HOUSE, HT_4KIND, HT_N, HT_N, HT_N},
				// triple, 0/1/3 jokers
				{HT_3KIND, HT_4KIND, HT_N, HT_4KIND, HT_N, HT_N},
				// full house, 0/2/3 jokers
				{HT_HOUSE, HT_N, HT_5KIND, HT_5KIND, HT_N, HT_N},
				// quadruple, 0/1/4 jokers
				{HT_4KIND, HT_5KIND, HT_N, HT_N, HT_5KIND, HT_N},
				// silly 5 jokers
				{HT_N, HT_N, HT_N, HT_N, HT_N, HT_5KIND}
			};

			type = promote[type][njoker];
		}
	}
};

void puzzle (bool debug) {
	std::vector<Hand> hands[2];

	std::string line;
	while (std::getline(std::cin, line)) {
		hands[SM_SILVER].emplace_back(line, SM_SILVER);
		hands[SM_GOLD].emplace_back(line, SM_GOLD);
	}

	for (StarMode sm : {SM_SILVER, SM_GOLD}) {
		std::vector<Hand*> hands_order;
		for (Hand& hand : hands[sm])
			hands_order.push_back(&hand);

		std::sort(hands_order.begin(), hands_order.end(), [] (auto&& l, auto&& r) {
			if (l->type != r->type)
				return l->type < r->type;

			for (u64 i = 0; i < 5; ++i) {
				if (l->cards_value[i] != r->cards_value[i])
					return l->cards_value[i] < r->cards_value[i];
			}

			return true; // actually, we're not allowed to reach equality
		});

		//fmt::println("reordered :");

		u64 winning = 0;
		for (u64 i = 0; i < hands_order.size(); ++i) {
			Hand& hand = *hands_order[i];

			if (debug || hand.type == HT_N)
				fmt::println("rank={}, bid={}, raw={}, type={}, values={}", i, hand.bid, hand.cards_raw, hand_name(hand.type), hand.cards_value);

			winning += (i + 1) * hand.bid;

		}
		fmt::println("{}", winning);
	}
}

int main (int argc, char** argv) {
	(void)argc;
	(void)argv;

	bool debug = 1 < argc && argv[1] == std::string("-g");

	try {
		puzzle(debug);
	} catch (const ParseError& e) {
		return 1;
	}

	return 0;
}
