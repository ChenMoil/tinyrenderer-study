#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	void initVector(std::string filePath); //初始化容器
	void DrawFaceGouraud(Vec3f* vertices, Vec2f* tex, Vec3f* n, TGAImage& image, Vec3f camaraPos, Vec3f lookPos, Vec3f up, Vec3f light_dir);
	void DrawFacePhong(Vec3f* vertices, Vec2f* tex, Vec3f* n, TGAImage& image, Vec3f camaraPos, Vec3f lookPos, Vec3f up, Vec3f light_dir);
	std::vector<Vec3f> m_vertices;  //点数组
	std::vector<Vec3f> m_n;  //法向量数组
	std::vector<std::vector<Vec3i>> m_faces;     //面数组
	std::vector<Vec2f> m_tex;     //纹理数组
public:
	TGAImage* texImage;                  //材质
	TGAImage* nImage;
	void DrawModel(TGAImage& image,TGAColor& color);
	void DrawModel(TGAImage& image, Vec3f camaraPos, Vec3f lookPos, Vec3f up, Vec3f light_dir);
	Model(std::string filePath);
	Model(std::string filePath, TGAImage* tex, TGAImage* n);
};

#endif //__MODEL_H__
