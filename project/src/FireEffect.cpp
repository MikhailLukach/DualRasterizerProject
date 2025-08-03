#include "pch.h"
#include "FireEffect.h"

namespace dae
{
	FireEffect::FireEffect(ID3D11Device* pDevice, const std::wstring& effectToload)
	{
		m_pEffect = LoadFireEffect(pDevice, effectToload);
		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
		if (!m_pTechnique->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable is not valid\n";
		}

		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable is not valid!\n";
		}
	}

	ID3DX11Effect* FireEffect::LoadFireEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); i++)
				{
					ss << pErrors[i];
				}
				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << std::endl;
			}
			else
			{
				std::wstringstream ss;
				ss << "effectLoader: Failed to CreateEffectFromFile\nPath: " << assetFile;
				std::wcout << ss.str() << std::endl;
				return nullptr;
			}
		}
		return pEffect;
	}

	FireEffect::~FireEffect()
	{
		m_pEffect->Release();
		//m_pTechnique->Release();
	}

	ID3DX11Effect* FireEffect::GetEffect()
	{
		return m_pEffect;
	}

	ID3DX11EffectTechnique* FireEffect::GetEffectTechnique()
	{
		return m_pTechnique;
	}

	void FireEffect::SetMatrix(const float* pfloat)
	{
		m_pMatWorldViewProjVariable->SetMatrix(pfloat);
	}

	void FireEffect::SetDiffuseMap(Texture* pDiffuseTexture)
	{
		if (m_pDiffuseMapVariable)
		{
			m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
		}
	}
}