#include "pch.h"
#include "Effect.h"
//#include <d3dxGlobal.h>

namespace dae
{
	Effect::Effect(ID3D11Device* pDevice, const std::wstring& effectToload)
	{
		m_pEffect = LoadEffect(pDevice, effectToload);
		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
		m_pDevice = pDevice;
		if(!m_pTechnique->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if(!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable is not valid\n";
		}
		m_pMatWorldMatrixVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
		if (!m_pMatWorldMatrixVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldMatrixVariable is not valid!\n";
		}
		m_pVecCameraPositionVariable = m_pEffect->GetVariableByName("gCameraPosition")->AsVector();
		if(!m_pVecCameraPositionVariable->IsValid())
		{
			std::wcout << L"m_pVecCameraPositionVariable is not valid!\n";
		}

		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if(!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable is not valid!\n";
		}
		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if(!m_pNormalMapVariable->IsValid())
		{
			std::wcout << L"m_pNormalMapVariable is not valid!\n";
		}
		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if(!m_pSpecularMapVariable->IsValid())
		{
			std::wcout << L"m_pSpecularMapVariable is not valid!\n";
		}
		m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		if(!m_pGlossinessMapVariable->IsValid())
		{
			std::wcout << L"m_pGlossinessMapVariable is not valid!\n";
		}

		m_pRasterizerVariable = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();
		if (!m_pRasterizerVariable->IsValid())
		{
			std::wcout << L"m_pShininessVar is not valid!\n";
		}

		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		HRESULT hr;

		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		hr = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerStateNone);
		if (FAILED(hr))
			std::wcout << L"none creation failed" << std::endl;

		rasterizerDesc.CullMode = D3D11_CULL_FRONT;
		hr = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerStateFrontface);
		if (FAILED(hr))
			std::wcout << L"frontface creation failed" << std::endl;

		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		hr = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerStateBackface);
		if (FAILED(hr))
			std::wcout << L"backface creation failed" << std::endl;
	}

	ID3DX11Effect* dae::Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
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

		if(FAILED(result))
		{
			if(pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for(unsigned int i{0}; i < pErrorBlob->GetBufferSize(); i++)
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

	Effect::~Effect()
	{
		m_pTechnique->Release();
		m_pEffect->Release();
		//m_pRasterizerVariable->Release();
	}

	ID3DX11Effect* Effect::GetEffect()
	{
		return m_pEffect;
	}

	ID3DX11EffectTechnique* Effect::GetEffectTechnique()
	{
		return m_pTechnique;
	}

	void Effect::SetMatrix(const float* pfloat)
	{
		m_pMatWorldViewProjVariable->SetMatrix(pfloat);
	}

	void Effect::SetWorldMatrix(const float* pfloat)
	{
		m_pMatWorldMatrixVariable->SetMatrix(pfloat);
	}

	void Effect::SetCameraPosition(Vector3 cameraPos)
	{
		if(m_pVecCameraPositionVariable)
		{
			m_pVecCameraPositionVariable->SetRawValue(reinterpret_cast<float*>(&cameraPos), 0, sizeof(float) * 3);
		}
	}

	void Effect::SetDiffuseMap(Texture* pDiffuseTexture)
	{
		if(m_pDiffuseMapVariable)
		{
			m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
		}
	}

	void Effect::SetNormalMap(Texture* pNormalTexture)
	{
		if(m_pNormalMapVariable)
		{
			m_pNormalMapVariable->SetResource(pNormalTexture->GetShaderResourceView());
		}
	}

	void Effect::SetSpecularMap(Texture* pSpecularTexture)
	{
		if(m_pSpecularMapVariable)
		{
			m_pSpecularMapVariable->SetResource(pSpecularTexture->GetShaderResourceView());
		}
	}

	void Effect::SetGlossinessMap(Texture* pGlossinessTexture)
	{
		if(m_pGlossinessMapVariable)
		{
			m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetShaderResourceView());
		}
	}

	void Effect::SetSampState(int enumNum)
	{
		switch(enumNum)
		{
		case 0:
			m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
			if (!m_pTechnique->IsValid())
			{
				std::wcout << L"Technique not valid\n";
			}
			break;
		case 1:
			m_pTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
			if (!m_pTechnique->IsValid())
			{
				std::wcout << L"Technique not valid\n";
			}
			break;
		case 2:
			m_pTechnique = m_pEffect->GetTechniqueByName("AntisotropicTechnique");
			if (!m_pTechnique->IsValid())
			{
				std::wcout << L"Technique not valid\n";
			}
			break;
		}
	}

	void Effect::SetCullMode(int enumNum)
	{
		switch(enumNum)
		{
		case 0:
			m_pRasterizerVariable->SetRasterizerState(0, m_pRasterizerStateNone);
			break;
		case 1:
			m_pRasterizerVariable->SetRasterizerState(0, m_pRasterizerStateFrontface);
			break;
		case 2:
			m_pRasterizerVariable->SetRasterizerState(0, m_pRasterizerStateBackface);
			break;
		}
	}
}
