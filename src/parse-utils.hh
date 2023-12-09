#include "utils.hh"
#include <boost/spirit/home/x3.hpp>

namespace x3 = boost::spirit::x3;
namespace ascii = x3::ascii;
using ascii::alpha;
using ascii::alnum;
using x3::ulong_long;
using x3::lit;
using x3::lexeme;
using x3::repeat;

struct ParseError : std::runtime_error {
	using std::runtime_error::runtime_error;
};

template <typename Parser>
void parse (const std::string& input, Parser&& parser) {
	auto begin = input.begin();
	auto end = input.end();
	if (!x3::phrase_parse(begin, end, std::forward<Parser>(parser), x3::space) || (begin != end))
		throw ParseError(fmt::format("parsing failed at >>> {}", input));
}

template <typename T>
auto assigner (T& var) {
	return [&] (auto& ctx) { var = _attr(ctx); };
}

template <typename T>
auto pusher (T& var) {
	return [&] (auto& ctx) { var.push_back(_attr(ctx)); };
}
