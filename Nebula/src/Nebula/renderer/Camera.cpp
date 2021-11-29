#include "nbpch.h"
#include "Camera.h"

namespace Nebula {
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top):
		m_ViewMatrix(1.0f) {
		NB_PROFILE_FUNCTION();

		SetProjection(left, right, bottom, top);
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top) {
		NB_PROFILE_FUNCTION();

		m_ProjectionMatrix = ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix() {
		NB_PROFILE_FUNCTION();

		mat4 transform = translate(m_Position)
			* rotate(radians(m_Rotation), vec3(0, 0, 1));

		m_ViewMatrix = inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}