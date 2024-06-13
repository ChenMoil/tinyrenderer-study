#include "tgaimage.h"
#include "model.h"
#include "DrawMath.h"
#include <chrono>
#include <iostream>


int main(int argc, char** argv) {
	TGAImage image(1000, 1000, TGAImage::RGB);
	TGAImage texImage, nImage;
	texImage.read_tga_file("diablo3_pose_diffuse.tga");
	texImage.flip_vertically();
	nImage.read_tga_file("diablo3_pose_nm.tga");
	nImage.flip_vertically();
	/*Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
	DrawTriangle(t0[1], t0[0], t0[2], image, red);
	DrawTriangle(t1[0], t1[1], t1[2], image, white);
	DrawTriangle(t2[0], t2[1], t2[2], image, green);*/

	Model model("diablo3_pose.obj", &texImage, &nImage);
	model.DrawModel(image, {0, 0, 1.5}, {0, 0, 0}, {0, 1 ,0}, {0, 0, 1});


	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	
	return 0;
}
