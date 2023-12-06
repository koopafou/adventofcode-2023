#pragma once
#include "utils.hh"

struct BinaryFunctionError {};

struct BinaryFunction {
	std::vector<u64> toggles;

	BinaryFunction()
	{}

	BinaryFunction(u64 begin, u64 end) {
		push_back(begin, end);
	}

	void push_back (u64 begin, u64 end) {
		if (begin == end)
			return;

		if (end < begin)
			throw BinaryFunctionError{};

		if (toggles.size()) {
			if (begin < toggles.back())
				throw BinaryFunctionError{};

			if (begin == toggles.back()) {
				toggles.back() = end;
				return;
			}
		}

		toggles.push_back(begin);
		toggles.push_back(end);
	}

	void shift (u64 offset) {
		for (u64& value : toggles)
			value += offset;
	}

	template <typename BF>
	static decltype(auto) ref (BF&& bf) {
		if constexpr (std::is_pointer_v<std::decay_t<BF>>)
			return *bf;
		else
			return bf;
	}

	template <typename T, typename F>
	static void scan (const std::vector<T>& bfs, F&& scanner) {
		scan_impl(bfs, std::forward<F>(scanner));
	}

	template <typename T, typename F>
	static void scan (const std::initializer_list<T>& bfs, F&& scanner) {
		scan_impl(std::vector<T>(bfs), std::forward<F>(scanner));
	}

	template <typename ScanInput, typename F>
	static void scan_impl (const ScanInput& bfs, F&& scanner) {
		const u64 N = bfs.size();
		std::vector<bool> args(N, false);

		std::vector<std::vector<u64>::const_iterator> afters;
		for (auto&& bf : bfs) {
			afters.push_back(ref(bf).toggles.cbegin());
		}

		u64 begin = 0;

		while (true) {
			u64 end_index = N;
			u64 end = (u64)-1;
			for (u64 i = 0; i < N; ++i) {
				if (afters[i] != ref(bfs[i]).toggles.cend() &&
				    *afters[i] < end) {
					end_index = i;
					end = *afters[i];
				}
			}

			scanner(begin, end, args);

			if (end_index == N)
				return;

			args[end_index] = !args[end_index];
			begin = end;
			++afters[end_index];
		}
	}

	static auto any_scanner (BinaryFunction& output) {
		return [&] (u64 begin, u64 end, const std::vector<bool>& args) {
			bool is_on = false;
			for (bool b : args)
				is_on |= b;
			if (is_on) {
				output.push_back(begin, end);
			}
		};
	}

	static auto all_scanner (BinaryFunction& output) {
		return [&] (u64 begin, u64 end, const std::vector<bool>& args) {
			bool is_on = true;
			for (bool b : args)
				is_on &= b;
			if (is_on) {
				output.push_back(begin, end);
			}
		};
	}
};
