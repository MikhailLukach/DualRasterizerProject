#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{};
		float fovAngle{ 45.f }; //90.f
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float aspectRatio{};

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{};
		float totalYaw{};

		float Near{ 1.f };
		float Far{ 1000.f };

		Matrix invViewMatrix{}; //onb
		Matrix viewMatrix{}; //view, world to camera
		Matrix projectionMatrix{};

		void Initialize(float _aspectRatio, float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f })
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;

			aspectRatio = _aspectRatio;
		}

		void CalculateViewMatrix()
		{
			right = { Vector3::Cross(Vector3::UnitY, forward).Normalized() };
			up = { Vector3::Cross(forward, right).Normalized() };
			invViewMatrix = { Vector4{right, 0.f}, Vector4{up, 0.f}, Vector4{forward, 0.f}, Vector4{origin, 1.f} };
			viewMatrix = Matrix::Inverse(invViewMatrix);
		}

		void CalculateProjectionMatrix()
		{
			Vector4 vectorX{ 1.f / (aspectRatio * fov), 0.f, 0.f, 0.f };
			Vector4 vectorY{ 0.f, 1.f / fov, 0.f, 0.f };
			Vector4 vectorZ{ 0.f, 0.f, Far / (Far - Near),1.f };
			Vector4 vectorW{ 0.f, 0.f,  (-(Far * Near)) / (Far - Near), 0.f };
			projectionMatrix = { vectorX, vectorY, vectorZ, vectorW };
		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			float movementSpeed{ 10.f }; //10.f
			float upDownSpeed{ 5.f }; //5.f
			float rotationYSpeed{ 2.f }; //2.f
			float rotationXSpeed{ 1.f }; //1.f
			float rotationCompSpeed{ 0.5f }; //0.5f
			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin += right * -movementSpeed * deltaTime;
				//std::cout << "A is pressed" << std::endl;
			}
			else if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				origin += right * movementSpeed * deltaTime;
			}
			else if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
			{
				origin += forward * movementSpeed * deltaTime;
			}
			else if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
			{
				origin += forward * -movementSpeed * deltaTime;
			}
			if(pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				//std::cout << "pressed" << std::endl;
				movementSpeed *= 5.f;
				upDownSpeed *= 5.f;
				rotationXSpeed *= 5.f;
				rotationYSpeed *= 5.f;
				rotationCompSpeed *= 5.f;
			}

			if (mouseState == SDL_BUTTON(1))
			{
				origin.z += -mouseY * rotationYSpeed * deltaTime;
				totalYaw += mouseX * rotationXSpeed * deltaTime;
			}
			else if (mouseState == SDL_BUTTON(3))
			{
				totalPitch += mouseY * rotationCompSpeed * deltaTime;
				totalYaw += mouseX * rotationCompSpeed * deltaTime;
			}
			else if (mouseState == (SDL_BUTTON(1) | SDL_BUTTON(3)))
			{
				origin.y += mouseY * upDownSpeed * deltaTime;
			}
			Matrix totalRotation{};
			totalRotation = Matrix::CreateRotation(totalPitch, totalYaw, 0.f);
			forward = totalRotation.TransformVector(Vector3::UnitZ).Normalized();
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}