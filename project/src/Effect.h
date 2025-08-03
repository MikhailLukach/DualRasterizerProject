#pragma once
#include <string>
#include "Texture.h"

struct ID3DX11Effect;
struct ID3DX11EffectTechnique;
struct ID3D11Device;
struct ID3DX11EffectShaderResourceVariable;

namespace dae
{
	enum class samState
	{
		point,
		linear,
		antisotropic
	};
	class Effect final
	{
	public:
		Effect(ID3D11Device* pDevice, const std::wstring& effectToload);
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		~Effect();

		ID3DX11Effect* GetEffect();
		ID3DX11EffectTechnique* GetEffectTechnique();

		void SetMatrix(const float* pfloat);
		void SetWorldMatrix(const float* pfloat);
		void SetCameraPosition(Vector3 cameraPos);
		void SetDiffuseMap(Texture* pDiffuseTexture);
		void SetNormalMap(Texture* pNormalTexture);
		void SetSpecularMap(Texture* pSpecularTexture);
		void SetGlossinessMap(Texture* pGlossinessTexture);
		void SetSampState(int enumNum);
		void SetCullMode(int enumNum);

		Effect(const Effect&) = delete;
		Effect(Effect&&) noexcept = delete;
		Effect& operator=(const Effect&) = delete;
		Effect& operator=(Effect&&) noexcept = delete;
	private:
		ID3D11Device* m_pDevice{};
		ID3DX11Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		ID3DX11EffectMatrixVariable* m_pMatWorldMatrixVariable{};
		ID3DX11EffectVectorVariable* m_pVecCameraPositionVariable{};

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

		ID3DX11EffectRasterizerVariable* m_pRasterizerVariable{};
		ID3D11RasterizerState* m_pRasterizerStateBackface; // BackFace Culling
		ID3D11RasterizerState* m_pRasterizerStateFrontface; // FrontFace Culling
		ID3D11RasterizerState* m_pRasterizerStateNone; // NO Culling
	};
}