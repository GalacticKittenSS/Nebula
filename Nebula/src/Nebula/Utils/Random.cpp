#include "nbpch.h"
#include "Random.h"

#include <random>

namespace Nebula {
	static std::random_device s_RandomDevice;

	static std::mt19937 s_Engine32(s_RandomDevice());
	static std::uniform_int_distribution<uint32_t> s_UniformDistribution32;

	static std::mt19937_64 s_Engine64(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution64;

	uint32_t Random<uint32_t>::Shuffle() {
		m_Random = s_UniformDistribution32(s_Engine32);
		return m_Random;
	}

	uint32_t Random<uint32_t>::Shuffle(uint32_t Min, uint32_t Max) {
		std::uniform_int_distribution<uint32_t> distr(Min, Max);
		m_Random = distr(s_Engine32);
		return m_Random;
	}

	uint64_t Random<uint64_t>::Shuffle() {
		m_Random = s_UniformDistribution64(s_Engine64);
		return m_Random;
	}

	uint64_t Random<uint64_t>::Shuffle(uint64_t Min, uint64_t Max) {
		std::uniform_int_distribution<uint64_t> distr(Min, Max);
		m_Random = distr(s_Engine32);
		return m_Random;
	}

	int32_t Random<int32_t>::Shuffle() {
		m_Random = s_UniformDistribution32(s_Engine32);
		return m_Random;
	}

	int32_t Random<int32_t>::Shuffle(int32_t Min, int32_t Max) {
		std::uniform_int_distribution<uint32_t> distr(Min, Max);
		m_Random = distr(s_Engine32);
		return m_Random;
	}

	int64_t Random<int64_t>::Shuffle() {
		m_Random = s_UniformDistribution64(s_Engine64);
		return m_Random;
	}

	int64_t Random<int64_t>::Shuffle(int64_t Min, int64_t Max) {
		std::uniform_int_distribution<uint64_t> distr(Min, Max);
		m_Random = distr(s_Engine32);
		return m_Random;
	}
}