#include "utils.hh"

int main (int argc, char** argv) {
	(void)argc;
	(void)argv;

	std::vector<std::string> inputs;
	while (std::cin) {
		std::string line;
		std::getline(std::cin, line);
		if (line.length())
			inputs.push_back(std::move(line));
	}

	u64 sum = 0;

	for (auto& in : inputs) {
		const char* first = &in[0];
		while (*first && (*first < '0' || '9' < *first))
			++first;

		const char* last = &in[in.length()];
		while (last != first && (*last < '0' || '9' < *last))
			--last;

		if (!*first) {
			fmt::println("input {} line {} is missing numbers", in, &in - &inputs[0]);
			continue;
		}

		u64 num = 10 * (*first - '0') + (*last - '0');
		sum += num;
	}

	fmt::println("{}", sum);

	sum = 0;

	std::regex reg("(zero|0)|(one|1)|(two|2)|(three|3)|(four|4)|(five|5)|(six|6)|(seven|7)|(eight|8)|(nine|9)");
	std::regex regr("(orez|0)|(eno|1)|(owt|2)|(eerht|3)|(ruof|4)|(evif|5)|(xis|6)|(neves|7)|(thgie|8)|(enin|9)");

	for (auto& in : inputs) {
		auto find_index = [] (auto it) {
			auto match = *it;
			for (u64 i = 1; i < match.size(); ++i)
				if (match[i].matched)
					return i - 1;
			return (u64)0;
		};

		auto first = std::sregex_iterator(in.cbegin(), in.cend(), reg);
		auto last = std::regex_iterator<std::string::const_reverse_iterator>(in.crbegin(), in.crend(), regr);

		u64 num = (10 * find_index(first) + find_index(last));
		//fmt::println(">> {}", num);
		sum += num;
	}

	fmt::println("{}", sum);

	return 0;
}
