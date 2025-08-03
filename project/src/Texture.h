#pragma once

struct ID3D11Device;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace dae
{
	class Texture final
	{
	public:
		Texture(ID3D11Device* pDevice, const std::string& path);
		~Texture();
		ID3D11ShaderResourceView* GetShaderResourceView();

		Texture(const Texture&) = delete;
		Texture(Texture&&) noexcept = delete;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) noexcept = delete;
	private:
		ID3D11Texture2D* m_pResource{};
		ID3D11ShaderResourceView* m_pSRV{};
	};
}