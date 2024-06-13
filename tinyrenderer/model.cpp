#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"
#include "DrawMath.h"
Matrix V2M(Vec3f& v)
{
	Matrix m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1;
	return m;
}
Matrix V2M(Vec3i& v)
{
	Matrix m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1;
	return m;
}
Vec3f M2V(Matrix& m)
{
	return Vec3f({ m[0][0] / m[3][0] ,m[1][0] / m[3][0], m[2][0] / m[3][0] });
}
Vec3i M2Vi(Matrix& m) 
{
	return Vec3i({ (int)(m[0][0] / m[3][0]) ,(int)(m[1][0] / m[3][0]), (int)(m[2][0] / m[3][0]) });
}
Vec3i* vec3f2vec3i(Vec3f* vec) {
	Vec3i* res = (Vec3i*)vec;
	for (int i = 0; i < 3; i++)
	{
		res[i].x = (int)vec[i].x;
		res[i].y = (int)vec[i].y;
		res[i].z = (int)vec[i].z;
	}
	return res;
}
std::vector<std::string> split(const std::string& s, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}
std::vector<Vec3i> parseFace(const std::string& faceStr) {
	std::vector<Vec3i> face;
	std::vector<std::string> vertices = split(faceStr.substr(2), ' '); // 去掉前导的 "f "

	for (const std::string& vertex : vertices) {
		std::vector<std::string> indices = split(vertex, '/');
		if (indices.size() == 3) {
			int v = std::stoi(indices[0]) - 1;  // 转换成从0开始的索引
			int vt = std::stoi(indices[1]) - 1;
			int vn = std::stoi(indices[2]) - 1;
			face.emplace_back(v, vt, vn);
			int i = 0;
		}
	}

	return face;
}
void Model::initVector(std::string filePath)
{
	// 创建流对象	
	std::ifstream ifs;
	//打开文件并判断是否打开成功如果成功则读取数据
	ifs.open(filePath, std::ios::in);
	const char* buff[100];
	if (ifs.is_open())
	{
		std::string line;
		while (std::getline(ifs, line)) {
			if (line.substr(0, 2) == "v ") {
				std::stringstream ss(line.substr(2));
				float x, y, z;
				ss >> x >> y >> z;
				m_vertices.emplace_back(x, y, z);
			}
			if (line.substr(0, 3) == "vt ") {
				std::stringstream ss(line.substr(2));
				float x, y;
				ss >> x >> y;
				m_tex.emplace_back(x, y);
			}
			if (line.substr(0, 2) == "f ") {
				std::vector<Vec3i> face = parseFace(line);
				m_faces.emplace_back(face);
			}
			if (line.substr(0, 3) == "vn ") {
				std::stringstream ss(line.substr(2));
				float x, y, z;
				ss >> x >> y >> z;
				m_n.emplace_back(x, y, z);
			}
		}
	}
	else
	{
		std::cout << "文件打开失败" << std::endl;
	}
	//关闭文件
	ifs.close();

}
void Model::DrawFaceGouraud(Vec3f* vertices, Vec2f* tex, Vec3f* n, TGAImage& image, Vec3f camaraPos, Vec3f lookPos, Vec3f up, Vec3f light_dir)
{
	Vec3f& v0 = vertices[0];
	Vec3f& v1 = vertices[1];
	Vec3f& v2 = vertices[2];
	
	int width = image.get_width();
	int height = image.get_height();

	Matrix ModelView;
	GetViewMatrix(ModelView, camaraPos, lookPos, up);
	Matrix Projection = Matrix::identity(4);
	Projection[3][2] = -1.f / (camaraPos - lookPos).norm();
	Matrix ViewPort = Matrix::identity(4);
	ViewPort[0][0] = width / 2;
	ViewPort[0][3] = width / 2;
	ViewPort[1][1] = height / 2;
	ViewPort[1][3] = height / 2;
	ViewPort[2][2] = (height + width) / 2;

	for (int i = 0; i < 3; i++)
	{
		Matrix temp = V2M(vertices[i]);
		temp = ViewPort * Projection * ModelView * temp;
		vertices[i] = M2V(temp);
	}
	float intensity[3]{ n[0] * light_dir , n[1] * light_dir , n[2] * light_dir };
	DrawTriangleGouraud(vec3f2vec3i(vertices), tex, intensity, image, *texImage);
}
void Model::DrawFacePhong(Vec3f* vertices, Vec2f* tex, Vec3f* n, TGAImage& image, Vec3f camaraPos, Vec3f lookPos, Vec3f up, Vec3f light_dir)
{
	int width = image.get_width();
	int height = image.get_height();

	/////shadow
	Vec3f v0 = vertices[0];
	Vec3f v1 = vertices[1];
	Vec3f v2 = vertices[2];
	Vec3f vecs[3]{ v0, v1, v2 };
	Matrix shadowModelView;
	GetLightViewMatrix(shadowModelView, { 0, 0, 1.5 }, { 0, 0, 0 });
	Matrix shadowProjection = Matrix::identity(4);
	shadowProjection[3][2] = -1.f / (camaraPos - lookPos).norm();
	Matrix shadowViewPort = Matrix::identity(4);
	shadowViewPort[0][0] = width / 2;
	shadowViewPort[0][3] = width / 2;
	shadowViewPort[1][1] = height / 2;
	shadowViewPort[1][3] = height / 2;
	shadowViewPort[2][2] = (height + width) / 2;
	Matrix shadowMvp = shadowViewPort * shadowModelView;
	for (int i = 0; i < 3; i++)
	{
		Matrix temp = V2M(vecs[i]);
		temp = shadowMvp * temp;
		vecs[i] = M2V(temp);
	}
	float xmax, xmin, ymax, ymin;
	xmax = std::max(vecs[0].x, std::max(vecs[1].x, vecs[2].x));
	xmin = std::min(vecs[0].x, std::min(vecs[1].x, vecs[2].x));
	ymax = std::max(vecs[0].y, std::max(vecs[1].y, vecs[2].y));
	ymin = std::min(vecs[0].y, std::min(vecs[1].y, vecs[2].y));
	Vec3f p;
	//TGAImage outPutShadowImage(width, height, TGAImage::RGB);
	for (int i = ymin; i <= ymax; i++)
	{
		for (int j = xmin; j < xmax; j++)
		{
			p.x = j;
			p.y = i;
			Vec3f barycentric = GetBarycentric(vecs, p);
			if (barycentric.x >= 0 && barycentric.y >= 0 && barycentric.z >= 0) //像素点在三角形内部
			{
				float distance = vecs[0].z * barycentric.x + vecs[1].z * barycentric.y + vecs[2].z * barycentric.z;
				//深度测试c
				if (p.x >= 0 && p.x < image.get_width() && p.y >= 0 && p.y < image.get_height() && distance > image.lightBuffer[j][i])
				{
					image.lightBuffer[j][i] = distance;
					//outPutShadowImage.set(i, j, TGAColor(-distance, -distance, -distance, 255)); // Convert depth to grayscale for visualization
				}
			}
		}
	}
	//outPutShadowImage.flip_vertically();
	//outPutShadowImage.write_tga_file("outPutShadowImage.tga");
	/////shadow
	
	/////normal mvp
	Matrix ModelView;
	GetViewMatrix(ModelView, camaraPos, lookPos, up);
	Matrix Projection = Matrix::identity(4);
	Projection[3][2] = -1.f / (camaraPos - lookPos).norm();
	Matrix ViewPort = Matrix::identity(4);
	ViewPort[0][0] = width / 2;
	ViewPort[0][3] = width / 2;
	ViewPort[1][1] = height / 2;
	ViewPort[1][3] = height / 2;
	ViewPort[2][2] = (height + width) / 2;
	Matrix normalMvp = ViewPort * Projection * ModelView;
	/////normal mvp

	DrawTrianglePhong(vertices, normalMvp, shadowMvp, tex, n, light_dir, image, *texImage, *nImage);
}
void Model::DrawModel(TGAImage& image, Vec3f camaraPos, Vec3f lookPos, Vec3f up, Vec3f light_dir)
{
	for (int i = 0; i < m_faces.size(); i++)
	{
		Vec3f vertices[3]{ m_vertices[m_faces[i][0].x], m_vertices[m_faces[i][1].x], m_vertices[m_faces[i][2].x]};
		Vec2f tex[3]{m_tex[m_faces[i][0].y], m_tex[m_faces[i][1].y], m_tex[m_faces[i][2].y] };
		Vec3f n[3]{ m_n[m_faces[i][0].z], m_n[m_faces[i][1].z], m_n[m_faces[i][2].z] };
		DrawFacePhong(vertices, tex, n, image, camaraPos, lookPos, up, light_dir);
	}
}

Model::Model(std::string filePath)
{
	initVector(filePath);
}

Model::Model(std::string filePath, TGAImage* tex, TGAImage* n)
	:texImage(tex), nImage(n)
{
	initVector(filePath);
}
