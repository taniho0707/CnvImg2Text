#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

static int clickFlags = 0;
static Point2f src_pt[]={
	Point2f(0, 0),
	Point2f(0, 0),
	Point2f(0, 0),
	Point2f(0, 0)
};


void myMouseCallback(int event, int x, int y, int flags, void *param){
	switch(event){
	case EVENT_LBUTTONUP:
		if(clickFlags > 3) return;
		else {
			src_pt[clickFlags].x = x;
			src_pt[clickFlags].y = y;
			++ clickFlags;
		}
		break;
	}
	return;
}

void findMaze(const Mat_<Vec3b> &img, Point2f *pos){
	// 乱数を発生させ，特定の一点から走査を始め，マスク画像を作る
	// 黒→黒，黒→白，黒→赤，白→白，白→赤，赤→赤のみの走査を認める	
	
	auto newimg = img.clone();
	Mat singleimg(newimg.rows, newimg.cols, CV_8UC1);

	cout << "width:" << newimg.cols << ", height:" << newimg.rows << endl;

	// 赤色のみの抽出
	cvtColor(newimg, newimg, CV_RGB2HSV);
	for(int x=0; x<newimg.cols; ++x){
		for(int y=0; y<newimg.rows; ++y){
			auto h = newimg.at<Vec3b>(y, x)[0];
			auto s = newimg.at<Vec3b>(y, x)[1];
			auto v = newimg.at<Vec3b>(y, x)[2];
			if(h > 100 && h < 160){// && s > 150 && v < 250){
				newimg.at<Vec3b>(y, x)[0] = 0;
				newimg.at<Vec3b>(y, x)[1] = 0;
				newimg.at<Vec3b>(y, x)[2] = 255;
				singleimg.at<uint8_t>(y, x) = 255;
			} else {
				newimg.at<Vec3b>(y, x)[0] = 0;
				newimg.at<Vec3b>(y, x)[1] = 0;
				newimg.at<Vec3b>(y, x)[2] = 0;
				singleimg.at<uint8_t>(y, x) = 0;
			}
		}
	}
	cvtColor(newimg, newimg, CV_HSV2RGB);

	// // 縮小処理でノイズを除去
	// for(int i=0; i<3; ++i){
	// 	erode(newimg, newimg, Mat());
	// 	erode(singleimg, singleimg, Mat());
	// }

	// エッジ検出
	Canny(singleimg, singleimg, 50, 200, 3);

	Mat lineimg(newimg.rows, newimg.cols, CV_8UC(3));
	// vector<Vec4i> lines;
	// HoughLinesP( singleimg, lines, 1, CV_PI/180, 80, 30, 10 );
	// for( size_t i = 0; i < lines.size(); i++ ){
	// 	line( lineimg, Point(lines[i][0], lines[i][1]),
	// 		  Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
	// }

	vector<Vec2f> lines;
	HoughLines( singleimg, lines, 1, CV_PI/180, 150 );

	for( size_t i = 0; i < lines.size(); i++ ){
		float rho = lines[i][0];
		float theta = lines[i][1];
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		Point pt1(cvRound(x0 + 1000*(-b)),
				  cvRound(y0 + 1000*(a)));
		Point pt2(cvRound(x0 - 1000*(-b)),
				  cvRound(y0 - 1000*(a)));
		line( lineimg, pt1, pt2, Scalar(0,0,255), 3, 8 );
	}

	imwrite("out_line.jpg", lineimg);
	imwrite("out_single.jpg", singleimg);
	imwrite("out_orig.jpg", newimg);
}

int main(int argc, char **argv){
	if(argc != 11){
		cout << "HowToUse: " << argv[0] << " srcImage dstImage LUx LUy LDx LDy RDx RDy RUx RUy" << endl;
		cout << "(x, y) = (0, 0) to open GUI" << endl;
		cout << "(x, y) = (1, 1) to recognize maze automaticaly" << endl;
		return -1;
	}

	const string src_image = argv[1];
	const string dst_image = argv[2];

	const int LUx = atoi(argv[3]);
	const int LUy = atoi(argv[4]);
	const int LDx = atoi(argv[5]);
	const int LDy = atoi(argv[6]);
	const int RDx = atoi(argv[7]);
	const int RDy = atoi(argv[8]);
	const int RUx = atoi(argv[9]);
	const int RUy = atoi(argv[10]);

	Mat_<Vec3b> img1 = imread(src_image);
	if(!img1.data){
		cout << "can't find" << src_image << endl;
		return -1;
	}

	const int dst_width = img1.cols;
	const int dst_height = img1.rows;

	Vec3b zero(0, 0, 0);
	Mat_<Vec3b> img2(dst_height, dst_width, zero);

	if(LUx + LUy + LDx + LDy + RDx + RDy + RUx + RUy == 0){
		namedWindow("srcmaze", CV_WINDOW_KEEPRATIO);
		imshow("srcmaze", img1);
		setMouseCallback("srcmaze", myMouseCallback, (void *)&img1);
		waitKey(0);
		destroyAllWindows();
	} else if(LUx + LUy + LDx + LDy + RDx + RDy + RUx + RUy == 8){
		findMaze(img1, &src_pt[0]);
	} else {
		src_pt[0].x = LUx;
		src_pt[0].y = LUy;
		src_pt[1].x = LDx;
		src_pt[1].y = LDy;
		src_pt[2].x = RDx;
		src_pt[2].y = RDy;
		src_pt[3].x = RUx;
		src_pt[3].y = RUy;
	}

	const Point2f dst_pt[]={
		Point2f(0, 0),
		Point2f(0, dst_height-1),
		Point2f(dst_width-1, dst_height-1),
		Point2f(dst_width-1, 0)
	};

	const Mat homography_matrix = getPerspectiveTransform(src_pt, dst_pt);

	warpPerspective(img1, img2, homography_matrix, img1.size());
	imwrite(dst_image, img2);


	return 0;
}



