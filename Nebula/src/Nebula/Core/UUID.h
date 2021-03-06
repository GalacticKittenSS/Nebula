#pragma once

#include "Nebula/Utils/Random.h"

namespace Nebula {
	class UUID {
	public:
		UUID(): m_UUID(Rand64()) { }
		UUID(uint64_t uuid) : m_UUID(uuid) { }
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}

namespace std {
	template<>
	struct hash<Nebula::UUID> {
		std::size_t operator()(const Nebula::UUID& uuid) const {
			return (uint64_t)uuid;
		}
	};
}
