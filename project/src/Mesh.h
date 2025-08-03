#pragma once
#include "Math.h"
#include "Camera.h"
#include <vector>

struct ID3D11Device;

namespace dae
{
	class Effect;
	class FireEffect;
	class Texture;
	struct Vertex_PosColTex
	{
		Vector3 position{};
		//ColorRGB color{ colors::White };
		Vector3 normal{};
		Vector2 uv{};
		Vector3 tangent{};
	};

	class Mesh final
	{
	public:
		Mesh(ID3D11Device* pDevice,
			std::vector<Vertex_PosColTex> vertices,
			std::vector<uint32_t> indices,
			Texture* pDiffuseTexture,
			Texture* pNormalTexture,
			Texture* pSpecularTexture,
			Texture* pGlossinessTexture,
			const std::wstring& effectToload,
			bool isTransparent);
		~Mesh();

		void Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera);
		void Translate(const Vector3& trans);
		void RotateX(float pitch);
		void RotateY(float yaw);
		void RotateZ(float roll);
		void Scale(const Vector3& scale);

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		void SetSampState(int sampState);
		void SetCullMode(int sampState);
	private:
		Effect* m_pEffect{};
		FireEffect* m_pFireEffect{};

		Texture* m_pDiffuseTexture{};
		Texture* m_pNormalTexture{};
		Texture* m_pSpecularTexture{};
		Texture* m_pGlossinessTexture{};

		Matrix m_WorldMatrix{};

		Matrix m_RotationTransform{};
		Matrix m_TranslationTransform{};
		Matrix m_ScaleTransform{};

		ID3DX11EffectTechnique* m_pTechnique{};
		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11Buffer* m_pIndicesBuffer{};
		uint32_t m_NumIndices{};

		Camera m_Camera{};

		bool m_IsTransparent{};
		
	};
}