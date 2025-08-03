#include "SoftwareTexture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	SoftwareTexture::SoftwareTexture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	SoftwareTexture::~SoftwareTexture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	SoftwareTexture* SoftwareTexture::LoadFromFile(const std::string& path)
	{
		return new SoftwareTexture(IMG_Load(path.c_str()));
	}

	ColorRGB SoftwareTexture::Sample(const Vector2& uv) const
	{
		uint8_t r{};
		uint8_t b{};
		uint8_t g{};
		Vector2 pixel{ uv.x * m_pSurface->w, uv.y * m_pSurface->h };
		if (pixel.x < 0.f)
		{
			pixel.x = 0.f;
		}
		if (pixel.y < 0.f)
		{
			pixel.y = 0.f;
		}
		if (pixel.x > m_pSurface->w)
		{
			pixel.x = m_pSurface->w;
		}
		if (pixel.y > m_pSurface->h)
		{
			pixel.y = m_pSurface->h;
		}
		uint32_t samplePixel = m_pSurfacePixels[int(std::round(pixel.x)) + (int(std::round(pixel.y)) * m_pSurface->w)];
		SDL_GetRGB(samplePixel, m_pSurface->format, &r, &g, &b);
		return ColorRGB(r / 255.f, g / 255.f, b / 255.f);
	}
}