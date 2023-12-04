#pragma once
#include <iostream>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <cstdint>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <regex>

using u32 = uint32_t;
using i32 = int32_t;

using u64 = uint64_t;
using i64 = int64_t;

namespace fmt {
	template <typename... T>
	FMT_INLINE void println (format_string<T...> fmt, T&& ... args) {
		print(fmt, std::forward<T>(args)...);
		print("\n");
		std::cout << std::flush;
	}
}
