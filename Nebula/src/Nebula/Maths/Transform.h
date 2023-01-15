#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace glm
{
	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER mat<4, 4, T, Q> translate(vec<3, T, Q> const& v)
	{
		return translate(mat<4, 4, T, Q>(1.0f), v);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER mat<4, 4, T, Q> rotate(T angle, vec<3, T, Q> const& v)
	{
		return rotate(mat<4, 4, T, Q>(1.0f), angle, v);
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER mat<4, 4, T, Q> scale(vec<3, T, Q> const& v)
	{
		return scale(mat<4, 4, T, Q>(1.0f), v);
	}
}