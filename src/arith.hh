// extracted from AoC 2020, hopefully doesn't break
#pragma once
#include "utils.hh"

struct ArithError : std::runtime_error {
	using std::runtime_error::runtime_error;
};

std::ostream& operator<< (std::ostream& o, __int128 x) {
	std::string str(40, ' ');
	bool neg = false;
	if (x < 0) {
		neg = true;
		x = -x;
	}
	int dex = str.size() - 1;
	while (x) {
		str[dex] = '0' + x % 10;
		x /= 10;
		--dex;
	}
	if (neg) {
		str[dex] = '-';
		--dex;
	}
	return o << str.c_str() + dex + 1;
}

struct EedState {
	__int128 r, s, t;
};

__int128 correct_mod (__int128 a, __int128 b) {
	return ((a%b)+b)%b;
}

// extended euclidean division
// return {r,s,t} such that gcd(a, b) == r == a * s + b * t
EedState eed (__int128 a, __int128 b) {
	EedState state_[] = {
		{a, 1, 0},
		{b, 0, 1},
		{0, 0, 0},
		{0, 0, 0}
	};
	u32 dex = 0;
	auto state = [&] (int offset) -> EedState& {
		return state_[(dex + offset) & 3];
	};
	while (state(1).r != 0) {
		__int128 q = state(0).r / state(1).r;
		state(2).r = state(0).r - q * state(1).r;
		state(2).s = state(0).s - q * state(1).s;
		state(2).t = state(0).t - q * state(1).t;
		++dex;
	}
	if (state(0).r < 0) {
		state(0).r = -state(0).r;
		state(0).s = -state(0).s;
		state(0).t = -state(0).t;
	}
#ifdef ARITH_DEBUG
	std::cerr << "eed>> " << a << " * " << state(0).s << " + " << b << " * " << state(0).t << " = " << state(0).r << std::endl;
#endif
	return state(0);
}


struct CrtEntry {
	__int128 cst;
	__int128 mod;
};

// chinese remainder theorem
// return x such that for each i, x % system[i].mod == system[i].cst
// assume 0 <= cst < mod
CrtEntry crt (const std::vector<CrtEntry>& system) {
	CrtEntry master = system[0];
	for (u64 i = 1; i < system.size(); ++i) {
		EedState state = eed(master.mod, system[i].mod);
		if (state.r != 1)
			throw ArithError(fmt::format("{}th module {} isn't coprime with {}", i, system[i].mod, master.mod));
		// that version has higher chances to overflow
		//master.cst = master.cst * system[i].mod * state.t + system[i].cst * master.mod * state.s;
		master.cst = system[i].cst + (master.cst - system[i].cst) * system[i].mod * state.t;
		master.mod *= system[i].mod;
#ifdef ARITH_DEBUG
		std::cerr << " . >>> " << master.cst << std::endl;
#endif
		master.cst = correct_mod(master.cst, master.mod);
#ifdef ARITH_DEBUG
		std::cerr << "cst>>> " << master.cst << std::endl;
		std::cerr << "mod>>> " << master.mod << std::endl;
#endif
	}
	return master;
}
