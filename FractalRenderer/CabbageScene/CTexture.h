#ifndef _CABBAGE_SCENE_CTEXTURE_H_INCLUDED_
#define _CABBAGE_SCENE_CTEXTURE_H_INCLUDED_

#include <string>

#include <GL/glew.h>

#include <SSize2.h>
#include "CImage.h"

class STextureCreationFlags
{

public:

	STextureCreationFlags();

	bool MipMaps;
	GLenum Wrap;
	GLenum Filter;
	GLenum PixelInternalFormat;
	GLenum PixelFormat;
	GLenum PixelType;

	void apply() const;

};

class CTexture
{

    GLuint TextureHandle;
	SSize2 Size;

public:

    CTexture(CImage * Image, STextureCreationFlags const Flags = STextureCreationFlags());
	CTexture(int const Width, int const Height, bool const Alpha, STextureCreationFlags const Flags = STextureCreationFlags());
	CTexture(SPosition2 const & size, bool const Alpha, STextureCreationFlags const Flags = STextureCreationFlags());
	CTexture(GLuint const textureHandle);

	~CTexture();

    GLuint const getTextureHandle() const;

	SSize2 const & getSize() const;
    int const getWidth() const;
    int const getHeight() const;

	bool const isValid() const;

};

#endif
