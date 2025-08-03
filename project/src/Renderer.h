#pragma once
#include <cstdint>
#include <vector>
#include "SoftwareTexture.h"
#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Mesh;
	class Texture;
	enum class SamplingStates
	{
		point,
		linear,
		antisotropic
	};
	enum class CullingMode
	{
		None,      // No culling
		FrontFace, // Cull front-facing triangles
		BackFace   // Cull back-facing triangles
	};
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		void ToggleSamplingStates();
		void ToggleCullingModes();
		void SetUsesDirectX(bool newState);
		void SetAllowRotation(bool newState);
		void SetUseUniformColour(bool newState);
		void SetDrawFireFX(bool newState);

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render();

		//DirectX functions
		void HardwareRender() const;
		SamplingStates GetSamplingState();
		bool GetUsesDirectX();
		bool GetUseUniformColor();
		bool GetDrawFireFX();

		//Software functions
		void SoftwareRender();
		std::vector<Vertex_Out> VectorNDCToScreenSpaceConverter(const std::vector<Vertex_Out>& ndcVectors_in);
		std::vector<Vertex_Out> VertexPerspectiveDivide(const std::vector<Vertex_Out>& ndcVectors_in);
		bool PerformDepthTest(float pixelDepth, int px, int py);
		ColorRGB PixelShading(const Vertex_Out& v, const ColorRGB& lambertDiffuse, const Vector3& normal, const Vector3& tangent,
			const Vector2& uv, const Vector3& viewDirect);
		void CycleLightingMode();
		void ToggleNormalMap();
		void ToggleDepthBuffer();
		void ToggleBoundingBox();
		bool GetAllowRotation();
		CullingMode GetCullingMode();
	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		const float m_RotationSpeed{ 45.f }; //0.05f

		bool m_IsInitialized{ false };
		bool m_UseUniformColour{ false };

		//SOFTWARE
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};

		//SoftwareMesh m_Meshes;

		SoftwareTexture* m_pSoftwareTexture;
		SoftwareTexture* m_pNormalSoftwareTexture;
		SoftwareTexture* m_pGlossSoftwareTexture;
		SoftwareTexture* m_pSpecularSoftwareTexture;

		enum class LightningMode
		{
			ObservedArea,
			Specular,
			Diffuse,
			Combined
		};

		bool m_ShowDepthBuffer{ false };
		bool m_AllowRotation{ true };
		bool m_AllowNormalMaps{ true };
		bool m_AllowBoundingBox{ false };

		std::vector<SoftwareMesh> meshes_world{};
		Vector3 m_LightDirection = { 0.577f, -0.577f, 0.577f };
		LightningMode m_CurrentLightningMode{ LightningMode::Combined };
		CullingMode m_CullingMode{ CullingMode::None };

		//DIRECTX
		HRESULT InitializeDirectX();

		Camera m_Camera{};
		
		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};
		IDXGISwapChain* m_pSwapChain{};
		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Resource* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};

		Mesh* m_pMesh{};
		Mesh* m_pFireMesh{};

		Texture* m_pDiffuseTexture{};
		Texture* m_pNormalTexture{};
		Texture* m_pSpecularTexture{};
		Texture* m_pGlossinessTexture{};
		Texture* m_pFireDiffuseTexture{};

		SamplingStates m_CurrentSamplingState{ SamplingStates::point };
		bool m_UseDirectX{ true };
		bool m_DrawFireFX{ true };
	};
}
