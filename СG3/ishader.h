#ifndef __ISHADER_H__
#define __ISHADER_H__

#include "tgaimage.h"
#include "geometry.h"

class IShader {
public:
    virtual ~IShader() {}
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

#endif