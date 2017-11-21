#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace cv;


// char itoa(char in){
// 	if(in > 9 && in < 16){
// 		return 'A'+(in-10);
// 	}
// 	return '0'+in;
// }

void colorExtraction(Mat *src, Mat *dst, int code,
					 int ch1Lower, int ch1Upper,
					 int ch2Lower, int ch2Upper,
					 int ch3Lower, int ch3Upper
	){
	Mat colorImage;
	int lower[3];
	int upper[3];

	Mat lut = Mat(256, 1, CV_8UC3);
	cvtColor(*src, colorImage, code);

	lower[0] = ch1Lower;
	lower[1] = ch2Lower;
	lower[2] = ch3Lower;
	upper[0] = ch1Upper;
	upper[1] = ch2Upper;
	upper[2] = ch3Upper;

	// for (int i=0; i<256; i++) {
	// 	for (int k=0; k<3; k++) {
	// 		if (lower[k] <= upper[k]) {
	// 			if( (lower[k] <= i) && (i <= upper[k])){
	// 				lut.data[i*lut.step+1] = 255;
	// 			} else {
	// 				lut.data[i*lut.step+1] = 0;
	// 			}
	// 		} else {
	// 			if( (lower[k] >= i) || (i >= upper[k])){
	// 				lut.data[i*lut.step+1] = 255;
	// 			} else {
	// 				lut.data[i*lut.step+1] = 0;
	// 			}
	// 		}
	// 	}
	// }
	// LUT(colorImage, lut, colorImage);
	// vector<Mat> planes;
	// split(colorImage, planes);

	// Mat maskImage;
	// bitwise_and(planes[0], planes[1], maskImage);
	// bitwise_and(maskImage, planes[2], maskImage);

	// Mat maskedImage;
	// src->copyTo(maskedImage, maskImage);
	// *dst = maskedImage;

	Mat tmp = Mat(src->rows, src->cols, CV_8UC3);
	uchar hsv[3];
	for (int i=0; i<src->cols; i++) {
		for (int j=0; j<src->rows; j++) {
			hsv[0] = colorImage.at<Vec3b>(j, i)[0];
			hsv[1] = colorImage.at<Vec3b>(j, i)[1];
			hsv[2] = colorImage.at<Vec3b>(j, i)[2];
			if (lower[0] <= upper[0]) {
				if(((lower[0] <= hsv[0]) && (hsv[0] <= upper[0]))
				   && ((lower[1] <= hsv[1]) && (hsv[1] <= upper[1]))
				   && ((lower[2] <= hsv[2]) && (hsv[2] <= upper[2]))){
					tmp.at<uchar>(j, i*3) = 255;
					tmp.at<uchar>(j, i*3+1) = 255;
					tmp.at<uchar>(j, i*3+2) = 255;
				} else {
					tmp.at<uchar>(j, i*3) = 0;
					tmp.at<uchar>(j, i*3+1) = 0;
					tmp.at<uchar>(j, i*3+2) = 0;
				}
			} else {
				if(((lower[0] <= hsv[0]) || (hsv[0] <= upper[0]))
				   && ((lower[1] <= hsv[1]) && (hsv[1] <= upper[1]))
				   && ((lower[2] <= hsv[2]) && (hsv[2] <= upper[2]))){
					tmp.at<uchar>(j, i*3) = 255;
					tmp.at<uchar>(j, i*3+1) = 255;
					tmp.at<uchar>(j, i*3+2) = 255;
				} else {
					tmp.at<uchar>(j, i*3) = 0;
					tmp.at<uchar>(j, i*3+1) = 0;
					tmp.at<uchar>(j, i*3+2) = 0;
				}
			}
		}
	}
	*dst = tmp;
	return;
}

int isExistWall(Mat &mat, int sizeX, int sizeY, int posX, int posY, int side){
	const int dst_width = mat.cols;
	const int dst_height = mat.rows;
	const double divX = (double)dst_width / sizeX;
	const double divY = (double)dst_height / sizeY;
	const int widthWallColumn = divX / 10;
	const int widthWallRow = divY / 10;
	int flag = 1;
	int count = 0;
	Vec3b bgr;
	int x, y;
	
	if(side == 0){ //right
		for (int i=1; i<4; i++) {
			count = 0;
			y = (int)(divY * (posY + (double)(i)/4));
			x = (int)(divX * (posX+1) - widthWallColumn);
			for (int j=0; j<3*widthWallColumn; j++, x++) {
				bgr = mat.at<Vec3b>(y, x);
				if(bgr[1] > 100){
					++ count;
					// mat.at<Vec3b>(y, x)[0] = 0;
					// mat.at<Vec3b>(y, x)[1] = 0;
					// mat.at<Vec3b>(y, x)[2] = 255;
				}
			}
			if(count < widthWallColumn * 0.1){
				flag = 0;
				break;
			}
		}
	} else { //down
		for (int i=1; i<4; i++) {
			count = 0;
			x = (int)(divX * (posX + (double)i/4));
			y = (int)(divY * (posY+1) - widthWallRow);
			for (int j=0; j<3*widthWallRow; j++, y++) {
				bgr = mat.at<Vec3b>(y, x);
				if(bgr[1] > 100){
					++ count;
					// mat.at<Vec3b>(y, x)[0] = 0;
					// mat.at<Vec3b>(y, x)[1] = 0;
					// mat.at<Vec3b>(y, x)[2] = 255;
				}
			}
			if(count < widthWallRow * 0.1){
				flag = 0;
				break;
			}
		}
	}
	return flag;
}

int main(int argc, char **argv){
	if(argc != 6){
		cout << "HowToUse: " << argv[0] << " srcImage dstImage mazeData x y" << endl;
		return -1;
	}

	const string src_image = argv[1];
	const string dst_image = argv[2];
	const char *mazedata = argv[3];
	const int mazeX = atoi(argv[4]);
	const int mazeY = atoi(argv[5]);

	Mat_<Vec3b> img1 = imread(src_image);
	if(!img1.data){
		cout << "can't find" << src_image << endl;
		return -1;
	}

	ofstream ofs(mazedata);

	Mat img2;
	colorExtraction(&img1, &img2, CV_BGR2HSV, 160, 50, 45, 255, 30, 255);

	ofs << "+";
	for (int i=0; i<mazeX; i++) {
		ofs << "---+";
	}
	ofs << endl << "|";

	for (int i=0; i<mazeY; i++) {
		for (int j=0; j<mazeX-1; j++) {
			ofs << "   ";
			if(isExistWall(img2, mazeX, mazeY, j, i, 0))
				ofs << "|";
			else
				ofs << " ";
		}
		ofs << "   |" << endl << "+";
		if(i != mazeY-1){
			for (int j=0; j<mazeX; j++) {
				if(isExistWall(img2, mazeX, mazeY, j, i, 1))
					ofs << "---+";
				else
					ofs << "   +";
			}
			ofs << endl << "|";
		}
	}

	for (int i=0; i<mazeX; i++) {
		ofs << "---+";
	}
	ofs << endl;
	ofs << endl;

	if(mazeX == 16 && mazeY == 16){
		unsigned char tmp = 0;
		ofs << "map[][]=" << endl;
		ofs << "{";
		for (int i=0; i<16; i++) {
			ofs << "{";
			for (int j=0; j<16; j++) {
				tmp = 0;
				if((j == 15) || isExistWall(img2, 16, 16, j, i, 0))
					tmp += 2;
				if((i == 15) || isExistWall(img2, 16, 16, j, i, 1))
					tmp += 8;
				char num[5];
				sprintf(num, "%d", tmp);
				ofs << num;
				if(j != 15) ofs << ",";
			}
			ofs << "}," << endl;
		}
		ofs << "};";
	} else if(mazeX == 32 && mazeY == 32){
		unsigned char tmp = 0;
		ofs << "map[][]=" << endl;
		ofs << "{";
		for (int i=0; i<32; i++) {
			ofs << "{";
			for (int j=31; j>=0; j--) {
				tmp = 0;
				if((i == 31) || isExistWall(img2, 32, 32, i, j, 0))
					tmp += 2;
				if((j == 31) || isExistWall(img2, 32, 32, i, j, 1))
					tmp += 8;
				if((j == 0) || isExistWall(img2, 32, 32, i, j-1, 1))
					tmp += 1;
				if((i == 0) || isExistWall(img2, 32, 32, i-1, j, 0))
					tmp += 4;
				char num[5];
				sprintf(num, "%d", tmp);
				ofs << num;
				// ofs << itoa(tmp, 10);
				if(j != 0) ofs << ",";
			}
			if (i == 31) {
				ofs << "}" << endl;
			} else {
				ofs << "}," << endl;
			}
		}
		ofs << "};";
	}

	
	ofs.close();
	imwrite(dst_image, img2);

	return 0;
}

