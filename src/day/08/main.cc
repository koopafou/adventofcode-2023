#include "parse-utils.hh"
#include "arith.hh"

struct Node {
	Node* connect[2];
	bool gold_terminal;

	Node()
		: connect{nullptr, nullptr}
	{}
};

struct Puzzle {
	std::vector<u64> directions;
	std::map<std::string, Node> store;
	std::map<Node*, std::string> reverse_store;

	Puzzle (std::istream& in) {
		std::string line;
		if (!std::getline(in, line))
			throw ParseError("expected directions line");

		directions.reserve(line.length());
		for (char cdir : line) {
			switch (cdir) {
			case 'L': directions.push_back(0); break;
			case 'R': directions.push_back(1); break;
			default: throw ParseError("expected L or R directions");
			}
		}

		while (std::getline(in, line)) {
			if (!line.length())
				continue;

			std::string label;
			std::string connect[2];
			auto nodename = lexeme[repeat(3)[alpha]];
			parse(line, nodename[assigner(label)] >> "="
			      >> "(" >> nodename[assigner(connect[0])]
			      >> "," >> nodename[assigner(connect[1])] >> ")");

			Node& node = store[label];
			reverse_store[&node] = label;
			node.connect[0] = &store[connect[0]];
			node.connect[1] = &store[connect[1]];
			node.gold_terminal = (label[2] == 'Z');
		}

		bool found_AAA = false;
		bool found_ZZZ = false;
		for (auto&& [label, node] : store) {
			found_AAA |= label == "AAA";
			found_ZZZ |= label == "ZZZ";
			if (!node.connect[0] || !node.connect[1])
				throw ParseError(fmt::format("missing connections in node {}", label));
		}
		if (!found_AAA)
			throw ParseError("couldn't find node AAA");
		if (!found_ZZZ)
			throw ParseError("couldn't find node ZZZ");
	}

	u64 silver () {
		Node* end = &store["ZZZ"];
		Node* current = &store["AAA"];
		u64 steps = 0;
		while (true) {
			for (u64 dir : directions) {
				current = current->connect[dir];
				++steps;
				if (current == end)
					return steps;
			}
		}
	}

	struct GoldNode {
		GoldNode* next;
		u64 distance_next;

		GoldNode()
			: next(nullptr)
			, distance_next(0)
		{}
	};

	struct GNKey {
		Node* node;
		u64 step;
	};

	u64 gold () {
		std::map<Node*, std::map<u64, GoldNode>> gnodes;
		std::map<GoldNode*, GNKey> reverse_gnodes;

		for (auto&& [label, node] : store) {
			if (label[2] != 'A')
				continue;

			Node* current = &node;
			u64 step = 0;
			GoldNode* prev_hook = nullptr;
			u64 distance_next = 0;

			while (true) {
				GoldNode& gn = gnodes[current][step];
				if (prev_hook) {
					prev_hook->next = &gn;
					prev_hook->distance_next = distance_next;
				}

				if (gn.next)
					break;

				auto& gnkey = reverse_gnodes[&gn];
				gnkey.node = current;
				gnkey.step = step;

				prev_hook = &gn;
				distance_next = 0;

				while (true) {
						current = current->connect[directions[step]];
						++distance_next;
						++step;
						if (step == directions.size())
							step = 0;
						if (current->gold_terminal)
							break;
				}
			}
		}

		std::vector<CrtEntry> system;

		for (auto&& [node, stepgn] : gnodes) {
			for (auto&& [step, gn] : stepgn) {
				auto& gnkey = reverse_gnodes[gn.next];
#if 0
				fmt::println("{}[{}] --- {} --> {}[{}]             divided={}",
				             reverse_store[node], step,
				             gn.distance_next,
				             reverse_store[gnkey.node], gnkey.step,
				             gn.distance_next / directions.size());
#endif

				// check that all gnodes begin the direction instructions
				if (step != 0)
					throw std::runtime_error("unsupported puzzle input (xxx[0])");

				// check that all gnodes point to an xxZ
				if (!gnkey.node->gold_terminal)
					throw std::runtime_error("unsupported puzzle input (-> xxZ)");

				// check that xxZ gnodes point to themselves
				if (node->gold_terminal && node != gnkey.node)
					throw std::runtime_error("unsupported puzzle input (xyZ -> xyZ)");

				if (!node->gold_terminal)
					system.emplace_back(gn.distance_next / directions.size(), gn.next->distance_next / directions.size());
			}
		}

		CrtEntry gold_crt = crt(system);
		__int128 gold_steps = gold_crt.cst;
		if (gold_steps == 0) // actually the puzzle is made so we fall in that category
			gold_steps = gold_crt.mod;

		gold_steps *= directions.size();

		if (gold_steps >> 64)
			throw std::runtime_error("puzzle answer exceeds 64 bits, sounds fishy");

		return (u64)gold_steps;
	}
};

int main (int argc, char** argv) {
	(void)argc;
	(void)argv;

	try {
		Puzzle pz(std::cin);
		fmt::println("{}", pz.silver());
		fmt::println("{}", pz.gold());
	} catch (const ParseError& e) {
		fmt::println("{}", e.what());
		return 1;
	} catch (const ArithError& e) {
		fmt::println("{}", e.what());
		return 2;
	} catch (std::runtime_error& e) {
		fmt::println("{}", e.what());
		return 3;
	}

	return 0;
}
