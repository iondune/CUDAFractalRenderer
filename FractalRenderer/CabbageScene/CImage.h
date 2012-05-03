#ifndef _CABBAGE_SCENE_CIMAGE_H_INCLUDED_
#define _CABBAGE_SCENE_CIMAGE_H_INCLUDED_

#include <string>

#include <GL/glew.h>

#include "../CabbageCore/SVector2.h"

class CImage
{

    friend class CImageLoader;

    char * ImageData;
    int Width;
    int Height;

	bool Alpha;

    CImage(char * imageData, int width, int height, bool const alpha = false);

public:

    ~CImage();

    int const getWidth() const;
    int const getHeight() const;
    char const * const getImageData() const;
	bool const hasAlpha() const;

};

#endif
