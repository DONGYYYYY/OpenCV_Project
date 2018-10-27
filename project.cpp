#include  <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <stdio.h>


typedef unsigned char BYTE;

int gCameraFlag = 1;

void mouseHandlerFunc(int event, int x, int y, int flags, void *param);
int moving_image(IplImage*img1, IplImage*img2, IplImage*dst);
int find_image(IplImage*img1, IplImage*whereis , int count , bool sign);


void main()
{
	IplImage * image = 0; // IplImage구조체로 image라는 포인터를 생성 이것으로 이미지를 받고 처리함.
	CvCapture* capture; // 동영상으로 나타낼 캡쳐 
	int param = 3;
	CvVideoWriter *writer1, *writer2, *writer3; // 저장하기위한 변수 선언
	int key = 0;
	int cnt = 0;
	int count = 0;
	char name[10];
	char buffer[40];
	char *tempPicture = "First Image";
	double now;
	bool sign = FALSE;
	double time = 0;
	capture = cvCaptureFromCAM(0); //  0 번째 연결된 카메라로부터 컨트롤을 얻어온다.

	cvNamedWindow(tempPicture, CV_WINDOW_AUTOSIZE); // 윈도우 생성(사진 촬용을 위한)
	cvSetMouseCallback(tempPicture, mouseHandlerFunc, &param); // 마우스 이벤트
	
	printf("사진을 찍으시겠습니다.\n왼쪽 클릭 = 사진 재촬영 시작 \n오른쪽 클릭 = 결정\n");
	while (!0) { // 카메라 실행 종료하기 전까지 반복
		if (gCameraFlag) { // gCameraFlag에 따라서 이벤트 발생
			image = cvQueryFrame(capture); 
			cvShowImage(tempPicture, image); // 이미지 보여주기.
		}
		key = cvWaitKey(10);
		if (key == 27) // ESC클릭시 종료
			break;
	}
	cvSaveImage("TEMP이미지.jpg", image); // 이미지 저장
	cvDestroyWindow(tempPicture);
	key = 0;
	image = 0;
	image = cvQueryFrame(capture);

	IplImage * temp = cvCreateImage(cvGetSize(image), 8, 3);
	IplImage * min = cvCreateImage(cvGetSize(image), 8, 3);
	IplImage* whereis = cvCreateImage(cvGetSize(image), 8, 3);
	writer1 = cvCreateVideoWriter("Result1.avi", CV_FOURCC('D', 'I', 'V', 'X'), 15, cvSize(temp->width, temp->height), 1); // 저장 방식
	writer2 = cvCreateVideoWriter("Result2.avi", CV_FOURCC('D', 'I', 'V', 'X'), 15, cvSize(temp->width, temp->height), 1);
	writer3 = cvCreateVideoWriter("Result3.avi", CV_FOURCC('D', 'I', 'V', 'X'), 15, cvSize(temp->width, temp->height), 1);

	
	while (1) // 동영상 촬영
	{

		image = cvQueryFrame(capture);

		//CvCapture 로부터 프레임을 얻어 image 라는 IplImage객체에 넣습니다
		if (moving_image(image, temp, min) == 1) //moving_image함수를 호출 리턴값이 1이면 if문 실행
		{
			cnt++; //매 프레임마다 카운트하여 이 카운트를 파일명에 넣습니다.
			itoa(cnt, name, 10);//저장되는사진이름은1,2,3,식으로카운트번호
			sprintf(buffer, "D:/cctv/%s.jpg", name);
			cvSaveImage(buffer, image); //지정된 파일명으로 저장합니다. (움직였을때 모습을 파일에 저장)
		}
		now = time / 10;
		if (now == 0) {
			printf("빨간불 입니다. \n");
			sign = FALSE;
		}
		else if (now < 30) { // 30보다 작으면 빨간불
			sign = FALSE;
		}
		else if (now == 30.0) {
			sign = TRUE;
			printf("초록불이 되었습니다.\n");
		}
		else if (now >= 30.0 && now <= 45.0) { // 30~ 45 사이에 (15초 동안)초록불
			sign = TRUE;
		}
		else { // 넘으면 time 초기화
			printf("빨간불 입니다. \n");
			time = 0;
		}
		time++; // time증가 (초록불 대기 시간)
		count = find_image(image, whereis , count , sign); // find_image 함수 호출 및 리턴값 count에 대입
		cvCopy(image, temp); // 바로전 이미지와 비교하기 위해서 temp에 image값(사진)을 저장
						
		
		cvShowImage("Original", image); // 기본 동영상 보여주기
		cvLine(min, cvPoint((image->width) / 2, 0), cvPoint((image->width / 2), image->height), cvScalar(255,255, 255), 1, 1); // 선그리기
		cvShowImage("Min-Show", min); // 영상의 차연산 보여주는 동영상
		cvLine(whereis, cvPoint((image->width) / 2, 0), cvPoint((image->width / 2), image->height), cvScalar(255, 255, 255), 1, 1);
		cvShowImage("Find-show", whereis); // 물체 이동하는것을 표시하는 것을 보여주는 동영상
		cvWriteFrame(writer1, temp); // 동영상저장
		cvWriteFrame(writer2, min); // 동영상저장
		cvWriteFrame(writer3, whereis); // 동영상저장
		key = cvWaitKey(33); //프레임을 받아오는 속도 33/1000 약 초당 30프레임
		if (key == 27) { //  esc누르면 종료
			break;
		} 
	}
	//image_different();
	cvReleaseCapture(&capture); //메모리해제
	cvReleaseImage(&temp);
	cvReleaseImage(&min);
	cvReleaseImage(&whereis);
	cvDestroyAllWindows();
}

int moving_image(IplImage*img1, IplImage*img2, IplImage*min) //차연산을 통해서 움직임이 있는지 확인을 하기 위한 함수 
	{
	int idx;
	int Red, Blue, Green;
		
	int cnt = 0;
	// 한프레임내의 픽셀을 도는 2중 for문
	for (int i = 0; i<img1->height; i++) //이미지차연산
	{
		for (int j = 0; j <img1->width; j++)
		{
			
			idx = (i * img1->widthStep +3 * j);
			Blue = abs(((BYTE)img1->imageData[idx]) - ((BYTE)img2->imageData[idx]));
			Green = abs(((BYTE)img1->imageData[idx + 1]) - ((BYTE)img2->imageData[idx+1]));
			Red = abs(((BYTE)img1->imageData[idx + 2]) - ((BYTE)img2->imageData[idx+2]));
			//한 프레임내에서 R,G,B모두 40이상의 색상차이를 보이면 이 수를 카운트해서(cnt++)
			//한프레임 내에서 총 몇개의 픽셀이 변화를 일으켰는지 셉니다.
			if (Blue>40 && Green>40 && Red>40 && j >= img1->width / 2) // RGB 가모두40이상차이날경우
			{
				min->imageData[idx] = (BYTE)img1->imageData[idx];
				min->imageData[idx + 1] = (BYTE)img1->imageData[idx + 1];
				min->imageData[idx + 2] = (BYTE)img1->imageData[idx + 2];
				cnt++; //카운트함
				
			}
			//그 외에 40이상 차이나지 않는 픽셀의 경우엔 0으로 채워서 검은색으로 만들어 차이를 보여주기 위함
			else
			{
				min->imageData[idx] = 0;
				min->imageData[idx + 1] = 0;
				min->imageData[idx + 2] = 0;
			}
		}
	}




	if (cnt>1000) // cnt > 1000 이면 1을 리턴  
		return 1;


	//외에는 0을 리턴

	return 0;


}


void mouseHandlerFunc(int event, int x, int y, int flags, void *param) // 마우스 이벤트 처리
{
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN: // 왼쪽 클릭시 촬영 시작
		gCameraFlag = 1;
		break;
	case CV_EVENT_RBUTTONDOWN: // 오른쪽 클릭시 촬영 중지
		gCameraFlag = 0;
		break;
	}

}

int find_image(IplImage*img, IplImage*whereis , int count , bool sign)
{
	int Red, Green, Blue;
	int col = 0, width = 0, row = 0, height = 0;
	int idx = 0;
	IplImage *image;
	char *temp = "TEMP이미지.jpg";
	image = cvLoadImage(temp);
	int draw_x, draw_y, n_cnt;
	draw_x = 0;
	draw_y = 0;
	n_cnt = 1;
	for (int i = 0; i < img->height; i++) //이미지차연산
	{
		for (int j = 0; j < img->widthStep; j += img->nChannels)
		{

			idx = (i * img->widthStep + j);
			Blue = abs(((BYTE)img->imageData[idx]) - ((BYTE)image->imageData[idx]));
			Green = abs(((BYTE)img->imageData[idx + 1]) - ((BYTE)image->imageData[idx + 1]));
			Red = abs(((BYTE)img->imageData[idx + 2]) - ((BYTE)image->imageData[idx + 2]));
			//한프레임 내에서 총 몇개의 픽셀이 변화를 일으켰는지 셉니다.
			if (Blue >= 40 && Green >= 40 && Red >= 40 && j >= img->widthStep/2) // BGR 40이상차이날경우
			{
				draw_x += j / 3;
				draw_y += i;
				if(j>img->widthStep/2)
				n_cnt++;

			}
			whereis->imageData[idx + 0] = img->imageData[idx + 0];
			whereis->imageData[idx + 1] = img->imageData[idx + 1];
			whereis->imageData[idx + 2] = img->imageData[idx + 2];
		}

	}
	draw_x /= n_cnt;
	draw_y /= n_cnt;
	if (n_cnt > 1000&&sign == FALSE) {
		cvCircle(whereis, cvPoint(draw_x, draw_y), 50, cvScalar(0, 255, 0), 2);
		count++;
		if(count%30 == 0)
		printf("[%d]위험하오니 물러서 주시길 바랍니다\n",count/30);
	}
	else { 
		count = 0;
	}
	return count;
}


