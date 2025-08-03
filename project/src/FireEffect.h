#pragma once
#include <string>
#include "Texture.h"

struct ID3DX11Effect;
struct ID3DX11EffectTechnique;
struct ID3D11Device;
struct ID3DX11EffectShaderResourceVariable;

namespace dae
{
	class FireEffect final
	{
	public:
		FireEffect(ID3D11Device* pDevice, const std::wstring& effectToload);
		static ID3DX11Effect* LoadFireEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		~FireEffect();

		ID3DX11Effect* GetEffect();
		ID3DX11EffectTechnique* GetEffectTechnique();
		void SetMatrix(const float* pfloat);
		void SetDiffuseMap(Texture* pDiffuseTexture);

		FireEffect(const FireEffect&) = delete;
		FireEffect(FireEffect&&) noexcept = delete;
		FireEffect& operator=(const FireEffect&) = delete;
		FireEffect& operator=(FireEffect&&) noexcept = delete;
	private:
		ID3DX11Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	};
}