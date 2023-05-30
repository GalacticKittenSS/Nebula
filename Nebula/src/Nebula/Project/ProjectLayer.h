#pragma once

namespace Nebula
{
	struct ProjectLayer
	{
		std::string Name;
		uint16_t Identity;

		enum Options {
			A = 0x0001, B = 0x0002, C = 0x0004, D = 0x0008,
			E = 0x0010, F = 0x0020, G = 0x0040, H = 0x0080,
			I = 0x0100, J = 0x0200, K = 0x0400, L = 0x0800,
			M = 0x1000, N = 0x2000, O = 0x4000, P = 0x8000
		};
	};
}