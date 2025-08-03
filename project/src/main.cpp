#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX mode - Mikhail Lukach 2DAE19GD",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool printFPS = false;
	bool isLooping = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
				//if (e.key.keysym.scancode == SDL_SCANCODE_X)
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
				{
					std::cout << "F1 pressed" << std::endl;
					if(pRenderer->GetUsesDirectX() == false)
					{
						SDL_SetWindowTitle(pWindow, "DirectX mode - Mikhail Lukach 2DAE19GD");
						pRenderer->SetUsesDirectX(true);
						std::cout << "Active mode: DirectX" << std::endl;
					}
					else
					{
						SDL_SetWindowTitle(pWindow, "Software Rasterizer mode - Mikhail Lukach 2DAE19GD");
						pRenderer->SetUsesDirectX(false);
						std::cout << "Active mode: Software Rasterizer" << std::endl;
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					std::cout << "F2 pressed" << std::endl;
					if (pRenderer->GetAllowRotation() == false)
					{
						pRenderer->SetAllowRotation(true);
					}
					else
					{
						pRenderer->SetAllowRotation(false);
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					std::cout << "F3 pressed" << std::endl;
					if (pRenderer->GetDrawFireFX() == false)
					{
						pRenderer->SetDrawFireFX(true);
					}
					else
					{
						pRenderer->SetDrawFireFX(false);
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					std::cout << "F4 pressed" << std::endl;
					pRenderer->ToggleSamplingStates();
					int curNum =  static_cast<int>(pRenderer->GetSamplingState());
					switch(curNum)
					{
					case 0:
						std::cout << "SamplingState: Point" << std::endl;
						break;
					case 1:
						std::cout << "SamplingState: Linear" << std::endl;
						break;
					case 2:
						std::cout << "SamplingState: Antisotropic" << std::endl;
						break;
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					std::cout << "F5 pressed" << std::endl;
					pRenderer->CycleLightingMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F6)
				{
					std::cout << "F6 pressed" << std::endl;
					pRenderer->ToggleNormalMap();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F7)
				{
					std::cout << "F7 pressed" << std::endl;
					pRenderer->ToggleDepthBuffer();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					std::cout << "F8 pressed" << std::endl;
					pRenderer->ToggleBoundingBox();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F9)
				{
					std::cout << "F9 pressed" << std::endl;
					pRenderer->ToggleCullingModes();
					int curNum = static_cast<int>(pRenderer->GetCullingMode());
					switch (curNum)
					{
					case 0:
						std::cout << "CullingMode: None" << std::endl;
						break;
					case 1:
						std::cout << "CullingMode: FrontFace" << std::endl;
						break;
					case 2:
						std::cout << "CullingMode: Backface" << std::endl;
						break;
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					std::cout << "F10 pressed" << std::endl;
					if (pRenderer->GetUseUniformColor() == false)
					{
						pRenderer->SetUseUniformColour(true);
					}
					else
					{
						pRenderer->SetUseUniformColour(false);
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					std::cout << "F11 pressed" << std::endl;
					if(printFPS == false)
					{
						printFPS = true;
					}
					else
					{
						printFPS = false;
					}
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			if(printFPS == true)
			{
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}