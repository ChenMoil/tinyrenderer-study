#pragma once
#include"tgaimage.h"
#include"geometry.h"

void DrawLine(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color);
void DrawLine(Vec2i t0, Vec2i t1, TGAImage& image, const TGAColor& color);
void DrawTriangleGouraud(Vec3i* vertices, Vec2f* tex, float* intensity, TGAImage& image, TGAImage& texImage);
void DrawTrianglePhong(Vec3f* vertices, Matrix normalMVP, Matrix shadowMVP, Vec2f* tex, Vec3f* n, Vec3f light_dir, TGAImage& image, TGAImage& texImage, TGAImage& nImage);
void GetViewMatrix(Matrix& res, Vec3f camaraPos, Vec3f lookPos, Vec3f up);
void GetLightViewMatrix(Matrix& res, Vec3f lightPos, Vec3f targetPos);
Matrix GetLightProjectionMatrix(float left, float right, float bottom, float top, float near, float far);
Vec3f GetBarycentric(Vec3f* pts, Vec3f P);
Vec3f GetBarycentric(Vec3i* pts, Vec3f P);


class IShader {
public:
    virtual ~IShader() = 0;
    virtual Vec3i vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

class GouraudShading : IShader
{
public:
    Vec3f varying_intensity;

private:


    // 通过 IShader 继承
    Vec3i vertex(int iface, int nthvert) override;

    bool fragment(Vec3f bar, TGAColor& color) override;

};