#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace glm
{
	template<typename T, qualifier Q>
	inline constexpr bool operator>(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2) {
		return v1.x > v2.x || v1.y > v2.y;
	}

	template<typename T, qualifier Q>
	inline constexpr bool operator<(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2) {
		return v1.x < v2.x || v1.y < v2.y;
	}
}

namespace Nebula::Maths {
	inline constexpr float epsilon() { return std::numeric_limits<float>::epsilon(); }
	inline constexpr float pi() { return 3.14159265358979323846264338327950288f; }

	inline bool epsilonEqual(float const& x, float const& y, float const& epsilon) { return abs(x - y) < epsilon; }
	inline bool epsilonNotEqual(float const& x, float const& y, float const& epsilon) { return abs(x - y) >= epsilon; }
	
	template<typename T>
	inline glm::vec<3, T> scaleVec(glm::vec<3, T> const& v, T desiredLength) {
		return v * desiredLength / length(v);
	}

	bool DecomposeTransform(glm::mat4& transform, glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);

	template<typename OStream, glm::length_t L, typename T>
	inline OStream& operator<<(OStream& os, const glm::vec<L, T>& vector) {
		return os << to_string(vector);
	}

	template<typename OStream, glm::length_t C, glm::length_t R, typename T>
	inline OStream& operator<<(OStream& os, const glm::mat<C, R, T>& matrix) {
		return os << to_string(matrix);
	}

	template<typename OStream, typename T>
	inline OStream& operator<<(OStream& os, glm::qua<T> quaternion) {
		return os << to_string(quaternion);
	}
}