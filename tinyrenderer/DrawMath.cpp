#include "DrawMath.h"
#include <iostream>
#include <vector>
Matrix V2M(Vec3f& v);
Matrix V2M(Vec3i& v);
Vec3f M2V(Matrix& m);
Vec3i M2Vi(Matrix& m);
Vec3i* vec3f2vec3i(Vec3f* vec);
void DrawLine(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color)
{
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	for (int x = x0; x <= x1; x++) {
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1. - t) + y1 * t;
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}
}
void DrawLine(Vec2i t0, Vec2i t1, TGAImage& image, const TGAColor& color)
{
	int x0 = t0.x;
	int y0 = t0.y;
	int x1 = t1.x;
	int y1 = t1.y;
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	for (int x = x0; x <= x1; x++) {
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1. - t) + y1 * t;
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}
}
void DrawTriangleGouraud(Vec3i* vertices, Vec2f* tex, float* intensity, TGAImage& image, TGAImage& texImage)
{
	int xmax, xmin, ymax, ymin;
	xmax = std::max(vertices[0].x, std::max(vertices[1].x, vertices[2].x));
	xmin = std::min(vertices[0].x, std::min(vertices[1].x, vertices[2].x));
	ymax = std::max(vertices[0].y, std::max(vertices[1].y, vertices[2].y));
	ymin = std::min(vertices[0].y, std::min(vertices[1].y, vertices[2].y));
	Vec3i p;
	for (int i = ymin; i <= ymax; i++)
	{
		for (int j = xmin; j < xmax; j++)
		{
			p.x = j;
			p.y = i;
			Vec3f barycentric = GetBarycentric(vertices, p);
			if (barycentric.x >= 0 && barycentric.y >= 0 && barycentric.z >= 0) //像素点在三角形内部
			{
				Vec2f uv = tex[0] * barycentric.x + tex[1] * barycentric.y + tex[2] * barycentric.z;
				float distance = vertices[0].z * barycentric.x + vertices[1].z * barycentric.y + vertices[2].z * barycentric.z;
				float realIntensity = intensity[0] * barycentric.x + intensity[1] * barycentric.y + intensity[2] * barycentric.z;

				//深度测试
				if (p.x >= 0 && p.x < image.get_width() && p.y >= 0 && p.y < image.get_height() && distance >= image.zBuffer[p.x][p.y] && realIntensity > 0)
				{
					image.zBuffer[p.x][p.y] = distance;
					TGAColor res = texImage.get(uv.x * texImage.get_width(), uv.y * texImage.get_height()) * realIntensity;
					if (res.r == 255 && res.g == 255 && res.b == 255)
					{
						std::cout << "bug" << std::endl;
					}
					image.set(p.x, p.y, res);
				}
			}
		}
	}
}

void DrawTrianglePhong(Vec3f* vec3f, Matrix normalMVP, Matrix shadowMVP, Vec2f* tex, Vec3f* n, Vec3f light_dir, TGAImage& image, TGAImage& texImage, TGAImage& nImage)
{
	Matrix deNormalMVP = normalMVP.inverse();
	for (int i = 0; i < 3; i++)
	{
		Matrix temp = V2M(vec3f[i]);
		temp = normalMVP * temp;
		vec3f[i] = M2V(temp);
	}
	float xmax, xmin, ymax, ymin;
	xmax = std::max(vec3f[0].x, std::max(vec3f[1].x, vec3f[2].x));
	xmin = std::min(vec3f[0].x, std::min(vec3f[1].x, vec3f[2].x));
	ymax = std::max(vec3f[0].y, std::max(vec3f[1].y, vec3f[2].y));
	ymin = std::min(vec3f[0].y, std::min(vec3f[1].y, vec3f[2].y));
	Vec3f p;
	for (int i = ymin; i <= ymax; i++)
	{
		for (int j = xmin; j < xmax; j++)
		{
			p.x = j;
			p.y = i;
			Vec3f barycentric = GetBarycentric(vec3f, p);
			if (barycentric.x >= 0 && barycentric.y >= 0 && barycentric.z >= 0) //像素点在三角形内部
			{
				float distance = vec3f[0].z * barycentric.x + vec3f[1].z * barycentric.y + vec3f[2].z * barycentric.z;
				p.z = distance;
				//求光源坐标
				Matrix ligthVec = V2M(p);
				ligthVec = shadowMVP * deNormalMVP * ligthVec;
				Vec3f lightV3f = M2V(ligthVec);
				if (lightV3f.x < 0 || lightV3f.x >= 1000 || lightV3f.y < 0 || lightV3f.y >= 1000) {
					continue;
				}
				//
				//计算阴影
				float shadowIntensity;
				float z1 = image.lightBuffer[(int)lightV3f.x][(int)lightV3f.y];
				float bias = 20.f; // Adjust bias according to scene requirements
				if (lightV3f.z + bias >= z1 ) {
					shadowIntensity = 1.f;
				}
				else {
					shadowIntensity = 0.3f;
				}
				//
				Vec2f uv = tex[0] * barycentric.x + tex[1] * barycentric.y + tex[2] * barycentric.z;
				
				TGAColor xyz = nImage.get(uv.x * nImage.get_width(), uv.y * nImage.get_height());
				Vec3f realn{ (float)xyz.r / 255 * 2 - 1, (float)xyz.g / 255 * 2 - 1, (float)xyz.b / 255 * 2 - 1 };
				float diffuseIntensity = std::max(light_dir * realn, .0f);
				Vec3f light = light_dir * -1;
				Vec3f returnLight = (realn * 2 * (realn * light) - light).normalize();
				float specularIntensity = std::pow(returnLight * Vec3f{ (float)p.x, (float)p.y, (float)p.z }.normalize(), 6);

				//深度测试
				if (p.x >= 0 && p.x < image.get_width() && p.y >= 0 && p.y < image.get_height() && distance >= image.zBuffer[(int)p.x][(int)p.y])
				{
					image.zBuffer[(int)p.x][(int)p.y] = distance;
					TGAColor res = texImage.get(uv.x * texImage.get_width(), uv.y * texImage.get_height()) * (diffuseIntensity * 0.8 + specularIntensity * 0.8 + 0.05);
					if (res.r == 255 && res.g == 255 && res.b == 255)
					{
						std::cout << "bug" << std::endl;
					}
					image.set(p.x, p.y, res * shadowIntensity);
				}
			}
		}
	}
}

void GetViewMatrix(Matrix& res, Vec3f camaraPos, Vec3f lookPos, Vec3f up)
{
	Matrix move = Matrix::identity(4);
	move[0][3] = -camaraPos.x;
	move[1][3] = -camaraPos.y;
	move[2][3] = -camaraPos.z;
	Matrix rot = Matrix::identity(4);
	Vec3f z = (camaraPos - lookPos).normalize();
	Vec3f x = (up ^ z).normalize();
	Vec3f y = (z ^ x).normalize();

	rot[0][0] = x.x;
	rot[0][1] = x.y;
	rot[0][2] = x.z;
	rot[1][0] = y.x;
	rot[1][1] = y.y;
	rot[1][2] = y.z;
	rot[2][0] = z.x;
	rot[2][1] = z.y;
	rot[2][2] = z.z;

	res = rot * move;
}
void GetLightViewMatrix(Matrix& res, Vec3f lightPos, Vec3f targetPos) {
	// 平移矩阵
	Matrix move = Matrix::identity(4);
	move[0][3] = -lightPos.x;
	move[1][3] = -lightPos.y;
	move[2][3] = -lightPos.z;

	// 旋转矩阵
	Matrix rot = Matrix::identity(4);
	Vec3f z = (lightPos - targetPos).normalize(); // 视线方向
	Vec3f up = { 0, 1, 0 }; // 假设up向量为世界坐标系的Y轴
	if (std::abs(z.y) > 0.99) { // 处理光线方向与up向量接近平行的情况
		up = { 0, 0, 1 }; // 选择另一个up向量
	}
	Vec3f x = (up ^ z).normalize(); // 右方向
	Vec3f y = (z ^ x).normalize(); // 上方向

	rot[0][0] = x.x;
	rot[0][1] = x.y;
	rot[0][2] = x.z;
	rot[1][0] = y.x;
	rot[1][1] = y.y;
	rot[1][2] = y.z;
	rot[2][0] = z.x;
	rot[2][1] = z.y;
	rot[2][2] = z.z;

	res = rot * move;
}
Matrix GetLightProjectionMatrix(float left, float right, float bottom, float top, float near, float far) {
	Matrix proj = Matrix::identity(4);
	proj[0][0] = 2 / (right - left);
	proj[1][1] = 2 / (top - bottom);
	proj[2][2] = -2 / (far - near);
	proj[0][3] = -(right + left) / (right - left);
	proj[1][3] = -(top + bottom) / (top - bottom);
	proj[2][3] = -(far + near) / (far - near);
	return proj;
}
/// <summary>
/// 求重心坐标
/// </summary>
Vec3f GetBarycentric(Vec3f* pts, Vec3f P) {
	Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
	/* `pts` and `P` has integer value as coordinates
	   so `abs(u[2])` < 1 means `u[2]` is 0, that means
	   triangle is degenerate, in this case return something with negative coordinates */
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	return Vec3f((1.f - (u.x + u.y) / u.z), u.y / u.z, u.x / u.z);
}
Vec3f GetBarycentric(Vec3i* pts, Vec3f P) {
	Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
	/* `pts` and `P` has integer value as coordinates
	   so `abs(u[2])` < 1 means `u[2]` is 0, that means
	   triangle is degenerate, in this case return something with negative coordinates */
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	return Vec3f((1.f - (u.x + u.y) / u.z), u.y / u.z, u.x / u.z);
}
Vec3i GouraudShading::vertex(int iface, int nthvert)
{

	return Vec3i();
}

bool GouraudShading::fragment(Vec3f bar, TGAColor& color)
{
	return false;
}
