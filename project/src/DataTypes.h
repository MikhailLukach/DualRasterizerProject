#pragma once
#include "Math.h"
#include "vector"

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		ColorRGB color{ colors::White };
		Vector2 uv{};
		Vector3 normal{}; //W4
		Vector3 tangent{}; //W4
		Vector3 viewDirection{}; //W4
	};

	struct Vertex_Out
	{
		Vector4 position{};
		ColorRGB color{ colors::White };
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	struct SoftwareMesh
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleList };

		std::vector<Vertex_Out> vertices_out{};
		Matrix worldMatrix{
			Vector4{1.f, 0.f, 0.f, 0.f},
			Vector4{0.f, 1.f, 0.f, 0.f},
			Vector4{0.f, 0.f, 1.f, 0.f},
			Vector4{0.f, 0.f, 0.f, 1.f}
		};
	};
}