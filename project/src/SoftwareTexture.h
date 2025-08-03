#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class SoftwareTexture
	{
	public:
		~SoftwareTexture();

		static SoftwareTexture* LoadFromFile(const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;

	private:
		SoftwareTexture(SDL_Surface* pSurface);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}