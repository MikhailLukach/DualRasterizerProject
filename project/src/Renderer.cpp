#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Texture.h"
#include "SoftwareTexture.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
		

		//Initializing For Software
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_pDepthBufferPixels = new float[m_Width * m_Height];

		m_pSoftwareTexture = SoftwareTexture::LoadFromFile("resources/vehicle_diffuse.png");
		m_pNormalSoftwareTexture = SoftwareTexture::LoadFromFile("resources/vehicle_normal.png");
		m_pGlossSoftwareTexture = SoftwareTexture::LoadFromFile("resources/vehicle_gloss.png");
		m_pSpecularSoftwareTexture = SoftwareTexture::LoadFromFile("resources/vehicle_specular.png");

		SoftwareMesh m_Meshes =
		{
			{
				{{-3.f, 3.f, 2.f}, colors::White, {0.f, 0.f}, {0.f, 0.f, -1.f}, {-1.f, 0.f, 0.f}},
				{{0.f, 3.f, 2.f}, colors::White, {0.5f, 0.f}, {0.f, 0.f, -1.f}, {-1.f, 0.f, 0.f}},
				{{3.f, 3.f, 2.f}, colors::White, {1.f, 0.f}, {0.f, 0.f, -1.f}, {-1.f, 0.f, 0.f}},
				{{-3.f, 0.f, 2.f}, colors::White, {0.f, 0.5f}, {0.f, 0.f, -1.f}, {-1.f, 0.f, 0.f}},
				{{0.f, 0.f, 2.f}, colors::White, {0.5f, 0.5f}, {0.f, 0.f, -1.f}, {-1.f, 0.f, 0.f}},
				{{3.f, 0.f, 2.f}, colors::White, {1.f, 0.5f}, {0.f, 0.f, -1.f}, {-1.f, 0.f, 0.f}},
				{{-3.f, -3.f, 2.f}, colors::White, {0.f, 1.f}, {0.f, 0.f, -1.f}, {-1.f, 0.f, 0.f}},
				{{0.f, -3.f, 2.f}, colors::White, {0.5f, 1.f}, {0.f, 0.f, -1.f}, {-1.f, 0.f, 0.f}},
				{{3.f, -3.f, 2.f}, colors::White, {1.f, 1.f}, {0.f, 0.f, -1.f}, {-1.f, 0.f, 0.f}},
			},
			{
				3, 0, 4,
				0, 1, 4,
				4, 1, 5,
				1, 2, 5,
				6, 3, 7,
				3, 4, 7,
				7, 4, 8,
				4, 5, 8
			},
			  PrimitiveTopology::TriangleList,
		};

		SoftwareMesh worldMesh{};
		Utils::ParseOBJ("resources/vehicle.obj", worldMesh.vertices, worldMesh.indices);

		meshes_world =
		{
			worldMesh
		};

		for (int idx{}; idx < meshes_world.size(); idx++)
		{
			meshes_world[idx].vertices_out = std::vector<Vertex_Out>{ meshes_world[idx].vertices.size() };
		}

		//Initializing For DirectX
		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		std::vector<Vertex_PosColTex> vertices{};
		std::vector<uint32_t> indices{};

		m_Camera.Initialize(float(m_Width) / m_Height, 60.f, { .0f, .0f, 0.f }); //0.f

		m_pDiffuseTexture = new Texture(m_pDevice, "resources/vehicle_diffuse.png");
		m_pNormalTexture = new Texture(m_pDevice, "resources/vehicle_normal.png");
		m_pSpecularTexture = new Texture(m_pDevice, "resources/vehicle_specular.png");
		m_pGlossinessTexture = new Texture(m_pDevice, "resources/vehicle_gloss.png");

		m_pFireDiffuseTexture = new Texture(m_pDevice, "resources/fireFX_diffuse.png");

		Utils::ParseOBJ("resources/vehicle.obj", vertices, indices);
		m_pMesh = new Mesh(m_pDevice, vertices, indices
			, m_pDiffuseTexture, m_pNormalTexture, m_pSpecularTexture, m_pGlossinessTexture
			, std::wstring(L"resources/PosCol3D.fx"), false);
		m_pMesh->Translate(Vector3(0.f, 0.f, 50.f));

		Utils::ParseOBJ("resources/fireFX.obj", vertices, indices);
		m_pFireMesh = new Mesh(m_pDevice, vertices, indices,
			m_pFireDiffuseTexture, m_pFireDiffuseTexture, m_pFireDiffuseTexture, m_pFireDiffuseTexture,
			std::wstring(L"resources/FlatShader.fx"), true);
		m_pFireMesh->Translate(Vector3(0.f, 0.f, 50.f));
	}

	Renderer::~Renderer()
	{
		m_pRenderTargetView->Release();
		m_pRenderTargetBuffer->Release();
		m_pDepthStencilView->Release();
		m_pDepthStencilBuffer->Release();
		m_pSwapChain->Release();
		if(m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		m_pDevice->Release();
		delete m_pMesh;
		delete m_pFireMesh;
		delete m_pDiffuseTexture;
		delete m_pNormalTexture;
		delete m_pSpecularTexture;
		delete m_pGlossinessTexture;
		delete m_pFireDiffuseTexture;

		delete[] m_pDepthBufferPixels;
		delete m_pSoftwareTexture;
		delete m_pGlossSoftwareTexture;
		delete m_pNormalSoftwareTexture;
		delete m_pSpecularSoftwareTexture;
	}

	void Renderer::ToggleSamplingStates()
	{
		int currentState{ static_cast<int>(m_CurrentSamplingState) };
		if (currentState >= 2)
		{
			m_CurrentSamplingState = SamplingStates::point;
			return;
		}
		else
		{
			currentState++;
			if (currentState == 1)
			{
				m_CurrentSamplingState = SamplingStates::linear;
			}
			else if (currentState == 2)
			{
				m_CurrentSamplingState = SamplingStates::antisotropic;
			}
		}
		m_pMesh->SetSampState(static_cast<int>(m_CurrentSamplingState));
	}

	void Renderer::ToggleCullingModes()
	{
		int currentMode{ static_cast<int>(m_CullingMode) };
		if (currentMode >= 2)
		{
			m_CullingMode = CullingMode::None;
			return;
		}
		else
		{
			currentMode++;
			if (currentMode == 1)
			{
				m_CullingMode = CullingMode::FrontFace;
			}
			else if (currentMode == 2)
			{
				m_CullingMode = CullingMode::BackFace;
			}
		}
		m_pMesh->SetCullMode(static_cast<int>(m_CullingMode));
	}

	void Renderer::SetUsesDirectX(bool newState)
	{
		m_UseDirectX = newState;
	}

	void Renderer::SetAllowRotation(bool newState)
	{
		m_AllowRotation = newState;
	}

	void Renderer::SetUseUniformColour(bool newState)
	{
		m_UseUniformColour = newState;
	}

	void Renderer::SetDrawFireFX(bool newState)
	{
		m_DrawFireFX = newState;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);
		if(m_UseDirectX == true)
		{
			if(m_AllowRotation == true)
			{
				m_pMesh->RotateY(m_RotationSpeed * pTimer->GetElapsed() * TO_RADIANS);
				m_pFireMesh->RotateY(m_RotationSpeed * pTimer->GetElapsed() * TO_RADIANS);
			}
		}
		else
		{
			if(m_AllowRotation == true)
			{
				meshes_world[0].worldMatrix = Matrix::CreateRotationY(PI_DIV_2 * pTimer->GetTotal())
					* Matrix::CreateTranslation(0.f, 0.f, 50.f);
				//meshes_world[0].worldMatrix = Matrix::CreateTranslation(0.f, 0.f, 50.f);
			}
		}
	}


	void Renderer::Render()
	{
		if(m_UseDirectX == true)
		{
			HardwareRender();
		}
		else
		{
			SoftwareRender();
		}
	}

	//DirectX functions
	void Renderer::HardwareRender() const
	{
		if (!m_IsInitialized)
			return;

		//1. clear RTV & DSV
		constexpr float color[4] = { 0.39f, 0.59f, 0.93f, 1.f };
		constexpr float uniformColor[4] = { 0.1f, 0.1f, 0.1f, 1.f };
		if(m_UseUniformColour == true)
		{
			m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, uniformColor);
		}
		else
		{
			m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
		}
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. set pipline + invoke draw calls
		m_pMesh->Render(m_pDeviceContext, m_Camera);
		if(m_DrawFireFX == true)
		{
			m_pFireMesh->Render(m_pDeviceContext, m_Camera);
		}

		//3. present backbuffer (swap)
		m_pSwapChain->Present(0, 0);
	}

	SamplingStates Renderer::GetSamplingState()
	{
		return m_CurrentSamplingState;
	}

	bool Renderer::GetUsesDirectX()
	{
		return m_UseDirectX;
	}

	bool Renderer::GetUseUniformColor()
	{
		return m_UseUniformColour;
	}

	bool Renderer::GetDrawFireFX()
	{
		return m_DrawFireFX;
	}

	void Renderer::SoftwareRender()
	{
		//@START
		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);
		if(m_UseUniformColour == false)
		{
			memset(m_pBackBufferPixels, int(0.39f * 255), m_Width * m_Height * sizeof(uint32_t)); //0.39f
		}
		else
		{
			memset(m_pBackBufferPixels, int(0.1f * 255), m_Width * m_Height * sizeof(uint32_t)); //0.1f
		}
		for (int pixelsIdx{}; pixelsIdx < (m_Width * m_Height); ++pixelsIdx)
		{
			m_pDepthBufferPixels[pixelsIdx] = FLT_MAX;
		}
		Matrix projectionMatrix{};
		std::vector<Vertex_Out> vertices_ScreenSpace{};
		for (int meshIdx{}; meshIdx < meshes_world.size(); ++meshIdx)
		{
			std::vector<Vertex_Out> vertices_converted(meshes_world[meshIdx].vertices.size());

			Matrix worldViewProjectionMatrix{ meshes_world[meshIdx].worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix };

			for (int vertIdx{}; vertIdx < meshes_world[meshIdx].vertices.size(); ++vertIdx)
			{
				meshes_world[meshIdx].vertices_out[vertIdx] = Vertex_Out{
					meshes_world[meshIdx].vertices[vertIdx].position.ToPoint4(),
					meshes_world[meshIdx].vertices[vertIdx].color,
					meshes_world[meshIdx].vertices[vertIdx].uv,
					meshes_world[meshIdx].vertices[vertIdx].normal,
					meshes_world[meshIdx].vertices[vertIdx].tangent,
					meshes_world[meshIdx].vertices[vertIdx].viewDirection };

				meshes_world[meshIdx].vertices_out[vertIdx].position = worldViewProjectionMatrix.TransformPoint(meshes_world[meshIdx].vertices_out[vertIdx].position);
				meshes_world[meshIdx].vertices_out[vertIdx].normal = meshes_world[meshIdx].worldMatrix.TransformVector(meshes_world[meshIdx].vertices_out[vertIdx].normal);
				meshes_world[meshIdx].vertices_out[vertIdx].tangent = meshes_world[meshIdx].worldMatrix.TransformVector(meshes_world[meshIdx].vertices_out[vertIdx].tangent);
				meshes_world[meshIdx].vertices_out[vertIdx].viewDirection = (m_Camera.origin -
					meshes_world[meshIdx].worldMatrix.TransformVector(meshes_world[meshIdx].vertices[vertIdx].position)).Normalized();
			}

			vertices_converted = VertexPerspectiveDivide(meshes_world[meshIdx].vertices_out);

			vertices_ScreenSpace = VectorNDCToScreenSpaceConverter(vertices_converted);

			int TriangleSeperator{ 3 };
			int StripVectorLimiter{ 0 };
			if (meshes_world[meshIdx].primitiveTopology == PrimitiveTopology::TriangleStrip)
			{
				TriangleSeperator = 1;
				StripVectorLimiter = 2;
			}

			for (int idx{}; idx < (meshes_world[meshIdx].indices.size() - StripVectorLimiter) / TriangleSeperator; ++idx)
			{
				if (meshes_world[meshIdx].indices[idx * TriangleSeperator] ==
					meshes_world[meshIdx].indices[(idx * TriangleSeperator) + 1] ||
					meshes_world[meshIdx].indices[(idx * TriangleSeperator) + 1] ==
					meshes_world[meshIdx].indices[(idx * TriangleSeperator) + 2])
				{
					continue;
				}
				std::vector<Vertex_Out> currentTriangleVec{};
				if (meshes_world[meshIdx].primitiveTopology == PrimitiveTopology::TriangleStrip)
				{
					if ((idx % 2) > 0)
					{
						currentTriangleVec = { { vertices_ScreenSpace[meshes_world[meshIdx].indices[idx * TriangleSeperator]] },
						{ vertices_ScreenSpace[meshes_world[meshIdx].indices[(idx * TriangleSeperator) + 2]] },
						{ vertices_ScreenSpace[meshes_world[meshIdx].indices[(idx * TriangleSeperator) + 1]] } };
					}
					else
					{
						currentTriangleVec = { { vertices_ScreenSpace[meshes_world[meshIdx].indices[idx * TriangleSeperator]] },
						{ vertices_ScreenSpace[meshes_world[meshIdx].indices[(idx * TriangleSeperator) + 1]] },
						{ vertices_ScreenSpace[meshes_world[meshIdx].indices[(idx * TriangleSeperator) + 2]] } };
					}
				}
				else
				{
					currentTriangleVec = { { vertices_ScreenSpace[meshes_world[meshIdx].indices[idx * TriangleSeperator]] },
					{ vertices_ScreenSpace[meshes_world[meshIdx].indices[(idx * TriangleSeperator) + 1]] },
					{ vertices_ScreenSpace[meshes_world[meshIdx].indices[(idx * TriangleSeperator) + 2]] } };
				}

				const Vector2 v0 = { currentTriangleVec[0].position.x, currentTriangleVec[0].position.y };
				const Vector2 v1 = { currentTriangleVec[1].position.x, currentTriangleVec[1].position.y };
				const Vector2 v2 = { currentTriangleVec[2].position.x, currentTriangleVec[2].position.y };

				float area = Vector2::Cross(v2 - v0, v1 - v0);

				if (m_CullingMode == CullingMode::FrontFace && area < 0.0f)
				{
					continue;
				}
				if (m_CullingMode == CullingMode::BackFace && area > 0.0f)
				{
					continue;
				}

				float currentMinX{ FLT_MAX };
				float currentMaxX{ FLT_MIN };
				float currentMinY{ FLT_MAX };
				float currentMaxY{ FLT_MIN };

				for (int idx1{ }; idx1 < currentTriangleVec.size(); ++idx1)
				{
					currentMinX = std::min(currentTriangleVec[idx1].position.x, currentMinX);
					currentMaxX = std::max(currentTriangleVec[idx1].position.x, currentMaxX);
					currentMinY = std::min(currentTriangleVec[idx1].position.y, currentMinY);
					currentMaxY = std::max(currentTriangleVec[idx1].position.y, currentMaxY);

				}

				if (currentMinX < 0.f)
				{
					currentMinX = 0.f;
				}
				if (currentMinY < 0.f)
				{
					currentMinY = 0.f;
				}
				if (currentMaxX > m_Width)
				{
					currentMaxX = float(m_Width);
				}
				if (currentMaxY > m_Height)
				{
					currentMaxY = float(m_Height);
				}

				std::vector<Vector2> boundingBox{ {currentMinX,currentMinY},{currentMaxX,currentMaxY} };
				//for loops, iterate and update current max or current min for both x and y
				Vector2 vertex0{ currentTriangleVec[0].position.x, currentTriangleVec[0].position.y };
				Vector2 vertex1{ currentTriangleVec[1].position.x, currentTriangleVec[1].position.y };
				Vector2 vertex2{ currentTriangleVec[2].position.x, currentTriangleVec[2].position.y };
				for (int px{ int(std::floor(boundingBox[0].x)) }; px < int(std::ceil(boundingBox[1].x)); ++px) //go from min x to max x
				{
					for (int py{ int(std::floor(boundingBox[0].y)) }; py < int(std::ceil(boundingBox[1].y)); ++py)//here too
					{
						if(m_AllowBoundingBox == true)
						{
							m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(255),
								static_cast<uint8_t>(255),
								static_cast<uint8_t>(255));
							continue;
						}

						ColorRGB finalColor{};
						Vector2 rPixel{ px + 0.5f, py + 0.5f };

						float weight0{ Vector2::Cross(vertex2 - vertex1, rPixel - vertex1) / 2 };
						float weight1{ Vector2::Cross(vertex0 - vertex2, rPixel - vertex2) / 2 };
						float weight2{ Vector2::Cross(vertex1 - vertex0, rPixel - vertex0) / 2 };

						float t{ weight0 + weight1 + weight2 };

						weight0 = weight0 / t;
						weight1 = weight1 / t;
						weight2 = weight2 / t;

						const float V0w = currentTriangleVec[0].position.w;
						const float V1w = currentTriangleVec[1].position.w;
						const float V2w = currentTriangleVec[2].position.w;

						float interpolatedPixelDepth{ 1 / (((1 / V0w) * weight0) + ((1 / V1w) * weight1) + ((1 / V2w) * weight2)) };

						if (weight0 <= 0 || weight0 >= 1)
						{
							continue;
						}
						if (weight1 <= 0 || weight1 >= 1)
						{
							continue;
						}
						if (weight2 <= 0 || weight2 >= 1)
						{
							continue;
						}

						Vertex_Out interpolatedVertex{};

						const Vector2 V0uv = currentTriangleVec[0].uv;
						const Vector2 V1uv = currentTriangleVec[1].uv;
						const Vector2 V2uv = currentTriangleVec[2].uv;
						Vector2 interpolatedUVs{ (((V0uv / V0w) * weight0) + ((V1uv / V1w) * weight1)
							+ ((V2uv / V2w) * weight2)) * interpolatedPixelDepth };

						const float V0z = currentTriangleVec[0].position.z;
						const float V1z = currentTriangleVec[1].position.z;
						const float V2z = currentTriangleVec[2].position.z;
						float interpolatedZDepth = (((V0z / V0w) * weight0) + ((V1z / V1w) * weight1)
							+ ((V2z / V2w) * weight2)) * interpolatedPixelDepth;

						const Vector3 V0n = currentTriangleVec[0].normal;
						const Vector3 V1n = currentTriangleVec[1].normal;
						const Vector3 V2n = currentTriangleVec[2].normal;

						Vector3 interpolatedNormals{ (((V0n / V0w) * weight0) + ((V1n / V1w) * weight1)
							+ ((V2n / V2w) * weight2)) * interpolatedPixelDepth };

						const Vector3 V0t = currentTriangleVec[0].tangent;
						const Vector3 V1t = currentTriangleVec[1].tangent;
						const Vector3 V2t = currentTriangleVec[2].tangent;

						Vector3 interpolatedTangents{ (((V0t / V0w) * weight0) + ((V1t / V1w) * weight1)
							+ ((V2t / V2w) * weight2)) * interpolatedPixelDepth };

						const ColorRGB V0c = currentTriangleVec[0].color;
						const ColorRGB V1c = currentTriangleVec[1].color;
						const ColorRGB V2c = currentTriangleVec[2].color;

						ColorRGB interpolatedColour{ (((V0c / V0w) * weight0) + ((V1c / V1w) * weight1)
							+ ((V2c / V2w) * weight2)) * interpolatedPixelDepth };

						const Vector3 V0v = currentTriangleVec[0].viewDirection;
						const Vector3 V1v = currentTriangleVec[1].viewDirection;
						const Vector3 V2v = currentTriangleVec[2].viewDirection;

						Vector3 interpolatedViewDirect{ (((V0v / V0w) * weight0) + ((V1v / V1w) * weight1)
							+ ((V2v / V2w) * weight2)) * interpolatedPixelDepth };

						interpolatedVertex.position = Vector4{ rPixel.x, rPixel.y, interpolatedZDepth, interpolatedPixelDepth };
						interpolatedVertex.uv = interpolatedUVs;
						interpolatedVertex.normal = interpolatedNormals.Normalized();
						interpolatedVertex.tangent = interpolatedTangents.Normalized();
						interpolatedVertex.color = interpolatedColour;
						interpolatedVertex.viewDirection = interpolatedViewDirect;

						//depthtest
						if (PerformDepthTest(interpolatedZDepth, px, py))
						{
							if (m_ShowDepthBuffer == false)
							{
								finalColor = PixelShading(interpolatedVertex, m_pSoftwareTexture->Sample(interpolatedUVs)
									, interpolatedVertex.normal, interpolatedVertex.tangent, interpolatedUVs, interpolatedViewDirect);
							}
							else
							{
								finalColor = interpolatedZDepth * colors::White;
							}
							finalColor.MaxToOne();

							m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(finalColor.r * 255),
								static_cast<uint8_t>(finalColor.g * 255),
								static_cast<uint8_t>(finalColor.b * 255));
						}
					}
				}
			}
		}

		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	std::vector<Vertex_Out> Renderer::VectorNDCToScreenSpaceConverter(const std::vector<Vertex_Out>& ndcVectors_in)
	{
		std::vector<Vertex_Out> ConvertedVectors{};
		for (int idx1{}; idx1 < ndcVectors_in.size(); idx1++)
		{
			Vertex_Out SingleConverted{};
			SingleConverted.position.x = (((ndcVectors_in[idx1].position.x + 1) / 2) * m_Width);
			SingleConverted.position.y = (((1 - ndcVectors_in[idx1].position.y) / 2) * m_Height);
			SingleConverted.position.z = ndcVectors_in[idx1].position.z;
			SingleConverted.position.w = ndcVectors_in[idx1].position.w;
			SingleConverted.uv = ndcVectors_in[idx1].uv;
			SingleConverted.tangent = ndcVectors_in[idx1].tangent;
			SingleConverted.normal = ndcVectors_in[idx1].normal;
			SingleConverted.viewDirection = ndcVectors_in[idx1].viewDirection;
			ConvertedVectors.push_back(SingleConverted);
		}
		return ConvertedVectors;
	}

	std::vector<Vertex_Out> Renderer::VertexPerspectiveDivide(const std::vector<Vertex_Out>& ndcVectors_in)
	{
		std::vector<Vertex_Out> ConvertedVectors{};
		for (int idx{}; idx < ndcVectors_in.size(); idx++)
		{
			Vertex_Out SingleConverted{};
			SingleConverted.position.x = ndcVectors_in[idx].position.x / ndcVectors_in[idx].position.w;
			SingleConverted.position.y = ndcVectors_in[idx].position.y / ndcVectors_in[idx].position.w;
			SingleConverted.position.z = ndcVectors_in[idx].position.z / ndcVectors_in[idx].position.w;
			SingleConverted.position.w = ndcVectors_in[idx].position.w;
			SingleConverted.uv = ndcVectors_in[idx].uv;
			SingleConverted.tangent = ndcVectors_in[idx].tangent;
			SingleConverted.normal = ndcVectors_in[idx].normal;
			SingleConverted.viewDirection = ndcVectors_in[idx].viewDirection;
			ConvertedVectors.push_back(SingleConverted);
		}
		return ConvertedVectors;;
	}

	bool Renderer::PerformDepthTest(float pixelDepth, int px, int py)
	{
		//x and y pixel
		if (pixelDepth < m_pDepthBufferPixels[px + (py * m_Width)] && pixelDepth > 0.f && pixelDepth < 1.f)
		{
			m_pDepthBufferPixels[px + (py * m_Width)] = pixelDepth;
			return true;
		}
		return false;
	}

	ColorRGB Renderer::PixelShading(const Vertex_Out& v, const ColorRGB& lambertDiffuse, const Vector3& normal, const Vector3& tangent, const Vector2& uv, const Vector3& viewDirect)
	{
		ColorRGB finalColor{};
		ColorRGB specularColor{ m_pSpecularSoftwareTexture->Sample(uv) };
		float phongExponent{ m_pGlossSoftwareTexture->Sample(uv).r };
		float shininess{ 25.f };
		Vector3 biNormal{ Vector3::Cross(normal,tangent) };
		Matrix tangentSpaceAxis{ tangent, biNormal, normal, Vector3{0,0,0} };
		Vector3 normalVector{ (2.f * m_pNormalSoftwareTexture->Sample(uv).r) - 1.f, (2.f * m_pNormalSoftwareTexture->Sample(uv).g) - 1.f
			, (2.f * m_pNormalSoftwareTexture->Sample(uv).b) - 1.f };
		normalVector = tangentSpaceAxis.TransformVector(normalVector);
		float lambertCosine{};
		if (m_AllowNormalMaps == true)
		{
			lambertCosine = Vector3::Dot(-m_LightDirection.Normalized(), normalVector);
		}
		else
		{
			lambertCosine = Vector3::Dot(-m_LightDirection.Normalized(), v.normal.Normalized());
		}
		if (lambertCosine < 0)
		{
			return colors::Black;
		}
		const Vector3 reflect{ (-m_LightDirection.Normalized()) - (2.f * lambertCosine) * normalVector };
		const float cosA{ Vector3::Dot(-reflect,viewDirect) };
		ColorRGB calcPhong{ specularColor * std::powf(cosA, (phongExponent * shininess)) };
		if (cosA < 0.f)
		{
			calcPhong = colors::Black;
		}
		ColorRGB BRDFDiffuse{ (lambertDiffuse * 7.f) / PI };

		if (m_CurrentLightningMode == LightningMode::Combined)
		{
			finalColor = lambertCosine * (BRDFDiffuse + calcPhong);
		}
		else if (m_CurrentLightningMode == LightningMode::ObservedArea)
		{
			finalColor = ColorRGB(lambertCosine, lambertCosine, lambertCosine);
		}
		else if (m_CurrentLightningMode == LightningMode::Specular)
		{
			finalColor = calcPhong;
		}
		else if (m_CurrentLightningMode == LightningMode::Diffuse)
		{
			finalColor = BRDFDiffuse;
		}
		return finalColor;
	}

	void Renderer::CycleLightingMode()
	{
		int currentModeInt{ static_cast<int>(m_CurrentLightningMode) };
		if (currentModeInt >= 3)
		{
			m_CurrentLightningMode = LightningMode::ObservedArea;
			return;
		}
		else
		{
			currentModeInt++;
			if (currentModeInt == 1)
			{
				m_CurrentLightningMode = LightningMode::Specular;
			}
			else if (currentModeInt == 2)
			{
				m_CurrentLightningMode = LightningMode::Diffuse;
			}
			else if (currentModeInt == 3)
			{
				m_CurrentLightningMode = LightningMode::Combined;
			}
		}
	}

	void Renderer::ToggleNormalMap()
	{
		m_AllowNormalMaps = !m_AllowNormalMaps;
	}

	void Renderer::ToggleDepthBuffer()
	{
		m_ShowDepthBuffer = !m_ShowDepthBuffer;
	}

	void Renderer::ToggleBoundingBox()
	{
		m_AllowBoundingBox = !m_AllowBoundingBox;
	}

	bool Renderer::GetAllowRotation()
	{
		return m_AllowRotation;
	}

	CullingMode Renderer::GetCullingMode()
	{
		return m_CullingMode;
	}

	HRESULT Renderer::InitializeDirectX()
	{
		//1) Create Device & DeviceContext
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
			1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
		if (FAILED(result))
			return result;

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;

		//2. Create Swapchain
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;
		
		//get handle (HWND) from the SDL backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_GetVersion(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;

		//3. Create DepthStencil (DS) & DepthStencilView (DSV)
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//view
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;

		//4) create RenderTarget (RT) & RenderTargetView (RTV)
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;

		//5) bind RTV & DSV to Output Merger Stage
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6) set Viewport
		CD3D11_VIEWPORT viewPort{};
		viewPort.Width = static_cast<float>(m_Width);
		viewPort.Height = static_cast<float>(m_Height);
		viewPort.TopLeftX = 0.f;
		viewPort.TopLeftY = 0.f;
		viewPort.MinDepth = 0.f;
		viewPort.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewPort);
		pDxgiFactory->Release();
		return result;
	}
}
