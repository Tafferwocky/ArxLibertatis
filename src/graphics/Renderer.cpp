
#include "graphics/Renderer.h"
#include "graphics/GraphicsUtility.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/data/TextureContainer.h"

Renderer * GRenderer;

TextureStage * Renderer::GetTextureStage(unsigned int textureStage) {
	return (textureStage < m_TextureStages.size()) ? m_TextureStages[textureStage] : NULL;
}

void Renderer::ResetTexture(unsigned int textureStage) {
	GetTextureStage(textureStage)->ResetTexture();
}

void Renderer::SetTexture(unsigned int textureStage, Texture * pTexture) {
	GetTextureStage(textureStage)->SetTexture(pTexture);
}

void Renderer::SetTexture(unsigned int textureStage, TextureContainer * pTextureContainer) {
	
	if(pTextureContainer && pTextureContainer->m_pTexture) {
		GetTextureStage(textureStage)->SetTexture(pTextureContainer->m_pTexture);
	} else {
		GetTextureStage(textureStage)->ResetTexture();
	}
}

Renderer::~Renderer() {
	for(size_t i = 0; i < m_TextureStages.size(); ++i) {
		delete m_TextureStages[i];
	}
}

void Renderer::SetViewMatrix(const Vec3f & position, const Vec3f & dir, const Vec3f & up) {
	
	EERIEMATRIX mat;
	Util_SetViewMatrix(mat, position, dir, up);
	
	SetViewMatrix(mat);
}
