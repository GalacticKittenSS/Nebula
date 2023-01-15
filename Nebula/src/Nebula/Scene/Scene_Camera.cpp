#include "nbpch.h"
#include "Scene_Camera.h"

namespace Nebula {
	SceneCamera::SceneCamera() {
		RecalculateProjectionMatrix();
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip) {
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;

		RecalculateProjectionMatrix();
	}

	void SceneCamera::SetPerspective(float fov, float nearClip, float farClip) {
		m_PerspectiveFOV = fov;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;

		RecalculateProjectionMatrix();
	}


	void SceneCamera::SetViewPortSize(uint32_t width, uint32_t height) {
		NB_ASSERT(width > 0 && height > 0, "");
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjectionMatrix();
	}


	void SceneCamera::RecalculateProjectionMatrix() {
		if (m_ProjectionType == ProjectionType::Perspective) {
			m_ProjectionMatrix = glm::perspective(m_PerspectiveFOV, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		} else {
			float Left = -m_OrthographicSize * m_AspectRatio * 0.5f;
			float Right = m_OrthographicSize * m_AspectRatio * 0.5f;
			float Bottom = -m_OrthographicSize * 0.5f;
			float Top = m_OrthographicSize * 0.5f;
			m_ProjectionMatrix = glm::ortho(Left, Right, Bottom, Top, m_OrthographicNear, m_OrthographicFar);
		}
	}
}