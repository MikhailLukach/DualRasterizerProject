#include "pch.h"
#include "Mesh.h"
#include "Effect.h"
#include "FireEffect.h"
#include "Texture.h"

namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice,
		std::vector<Vertex_PosColTex> vertices,
		std::vector<uint32_t> indices,
		Texture* pDiffuseTexture,
		Texture* pNormalTexture,
		Texture* pSpecularTexture,
		Texture* pGlossinessTexture,
		const std::wstring& effectToload,
		bool isTransparent)
	{
		m_IsTransparent = isTransparent;
		if(m_IsTransparent == false)
		{
			m_pEffect = new Effect(pDevice, effectToload);
		}
		else
		{
			m_pFireEffect = new FireEffect(pDevice, effectToload);
		}
		m_pDiffuseTexture = pDiffuseTexture;
		m_pNormalTexture = pNormalTexture;
		m_pSpecularTexture = pSpecularTexture;
		m_pGlossinessTexture = pGlossinessTexture;
		static constexpr uint32_t numElements{ 4 }; //to 3
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0; //0
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		//vertexDesc[1].SemanticName = "COLOR";
		//vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		//vertexDesc[1].AlignedByteOffset = 12;
		//vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "NORMAL";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12; //12
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		//extend for uv
		//one component takes 4 bytes
		//starts 24
		vertexDesc[2].SemanticName = "TEXCOORD";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 24; //24
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TANGENT";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;				
		vertexDesc[3].AlignedByteOffset = 32; //32
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		if(m_IsTransparent == false)
		{
			m_pTechnique = m_pEffect->GetEffectTechnique();
		}
		else
		{
			m_pTechnique = m_pFireEffect->GetEffectTechnique();
		}
		//m_pTechnique = m_pEffect->GetEffectTechnique();
		D3DX11_PASS_DESC passDesc{};
		m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		HRESULT result = pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout);

		if(FAILED(result))
		{
			return;
		}

		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex_PosColTex) * static_cast<uint32_t>(vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = vertices.data();

		result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);

		if (FAILED(result))
		{
			return;
		}

		m_NumIndices = static_cast<uint32_t>(indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		initData.pSysMem = indices.data();
		result = pDevice->CreateBuffer(&bd, &initData, &m_pIndicesBuffer);

		if (FAILED(result))
		{
			return;
		}
	}

	Mesh::~Mesh()
	{
		m_pIndicesBuffer->Release();
		m_pInputLayout->Release();
		m_pVertexBuffer->Release();
		delete m_pEffect;
		delete m_pFireEffect;
	}

	void Mesh::Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera)
	{
		if(m_IsTransparent == false)
		{
			m_WorldMatrix = m_RotationTransform * m_TranslationTransform * m_ScaleTransform;
			Matrix WorldViewProjectionMatrix = m_WorldMatrix * camera.viewMatrix * camera.projectionMatrix;
			m_pEffect->SetMatrix(reinterpret_cast<const float*>(&WorldViewProjectionMatrix));
			m_pEffect->SetWorldMatrix(reinterpret_cast<const float*>(&m_WorldMatrix));

			//put here camera position
			m_pEffect->SetCameraPosition(camera.origin);
		}
		else
		{
			m_WorldMatrix = m_RotationTransform * m_TranslationTransform * m_ScaleTransform;
			Matrix WorldViewProjectionMatrixFire = m_WorldMatrix * camera.viewMatrix * camera.projectionMatrix;
			m_pFireEffect->SetMatrix(reinterpret_cast<const float*>(&WorldViewProjectionMatrixFire));
		}

		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pDeviceContext->IASetInputLayout(m_pInputLayout);

		constexpr UINT stride = sizeof(Vertex_PosColTex);
		constexpr UINT offset = 0;
		pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		
		pDeviceContext->IASetIndexBuffer(m_pIndicesBuffer, DXGI_FORMAT_R32_UINT, 0);

		//set diffuse map here (and other textures)
		if(m_IsTransparent == false)
		{
			m_pEffect->SetDiffuseMap(m_pDiffuseTexture);
			m_pEffect->SetNormalMap(m_pNormalTexture);
			m_pEffect->SetSpecularMap(m_pSpecularTexture);
			m_pEffect->SetGlossinessMap(m_pGlossinessTexture);


			D3DX11_TECHNIQUE_DESC techDesc{};
			m_pEffect->GetEffectTechnique()->GetDesc(&techDesc);
			for (UINT p{ 0 }; p < techDesc.Passes; ++p)
			{
				m_pEffect->GetEffectTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
				pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
			}
		}
		else
		{
			m_pFireEffect->SetDiffuseMap(m_pDiffuseTexture);
			D3DX11_TECHNIQUE_DESC techDesc{};
			m_pFireEffect->GetEffectTechnique()->GetDesc(&techDesc);
			for (UINT p{ 0 }; p < techDesc.Passes; ++p)
			{
				m_pFireEffect->GetEffectTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
				pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
			}
		}
	}

	void Mesh::Translate(const Vector3& trans)
	{
		m_TranslationTransform *= Matrix::CreateTranslation(trans);
	}

	void Mesh::RotateX(float pitch)
	{
		m_RotationTransform *= Matrix::CreateRotationX(pitch);
	}

	void Mesh::RotateY(float yaw)
	{
		m_RotationTransform *= Matrix::CreateRotationY(yaw);
	}

	void Mesh::RotateZ(float roll)
	{
		m_RotationTransform *= Matrix::CreateRotationZ(roll);
	}

	void Mesh::Scale(const Vector3& scale)
	{
		m_ScaleTransform *= Matrix::CreateScale(scale);
	}

	void Mesh::SetSampState(int sampState)
	{
		m_pEffect->SetSampState(sampState);
	}

	void Mesh::SetCullMode(int sampState)
	{
		m_pEffect->SetCullMode(sampState);
	}
}