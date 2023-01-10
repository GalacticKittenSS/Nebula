#pragma once

#include "Nebula/Scene/Components.h"
#include "box2d/b2_body.h"

namespace Nebula::Utils
{
	inline b2BodyType Rigibody2DToBox2D(Rigidbody2DComponent::BodyType bodyType) 
	{
		switch (bodyType)
		{
			case Rigidbody2DComponent::BodyType::Static:	return b2BodyType::b2_staticBody;
			case Rigidbody2DComponent::BodyType::Dynamic:	return b2BodyType::b2_dynamicBody;
			case Rigidbody2DComponent::BodyType::Kinematic: return b2BodyType::b2_kinematicBody;
		}

		NB_ASSERT(false, "Unknown Rigidbody Type!");
		return b2BodyType::b2_staticBody;
	}

	inline Rigidbody2DComponent::BodyType Rigibody2DFromBox2D(b2BodyType bodyType)
	{
		switch (bodyType)
		{
			case b2BodyType::b2_staticBody:		return Rigidbody2DComponent::BodyType::Static;
			case b2BodyType::b2_dynamicBody:	return Rigidbody2DComponent::BodyType::Dynamic;
			case b2BodyType::b2_kinematicBody:	return Rigidbody2DComponent::BodyType::Kinematic;
		}

		NB_ASSERT(false, "Unknown Rigidbody Type!");
		return Rigidbody2DComponent::BodyType::Static;
	}
}