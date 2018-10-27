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
	IplImage * image = 0; // IplImage����ü�� image��� �����͸� ���� �̰����� �̹����� �ް� ó����.
	CvCapture* capture; // ���������� ��Ÿ�� ĸ�� 
	int param = 3;
	CvVideoWriter *writer1, *writer2, *writer3; // �����ϱ����� ���� ����
	int key = 0;
	int cnt = 0;
	int count = 0;
	char name[10];
	char buffer[40];
	char *tempPicture = "First Image";
	double now;
	bool sign = FALSE;
	double time = 0;
	capture = cvCaptureFromCAM(0); //  0 ��° ����� ī�޶�κ��� ��Ʈ���� ���´�.

	cvNamedWindow(tempPicture, CV_WINDOW_AUTOSIZE); // ������ ����(���� �Կ��� ����)
	cvSetMouseCallback(tempPicture, mouseHandlerFunc, &param); // ���콺 �̺�Ʈ
	
	printf("������ �����ðڽ��ϴ�.\n���� Ŭ�� = ���� ���Կ� ���� \n������ Ŭ�� = ����\n");
	while (!0) { // ī�޶� ���� �����ϱ� ������ �ݺ�
		if (gCameraFlag) { // gCameraFlag�� ���� �̺�Ʈ �߻�
			image = cvQueryFrame(capture); 
			cvShowImage(tempPicture, image); // �̹��� �����ֱ�.
		}
		key = cvWaitKey(10);
		if (key == 27) // ESCŬ���� ����
			break;
	}
	cvSaveImage("TEMP�̹���.jpg", image); // �̹��� ����
	cvDestroyWindow(tempPicture);
	key = 0;
	image = 0;
	image = cvQueryFrame(capture);

	IplImage * temp = cvCreateImage(cvGetSize(image), 8, 3);
	IplImage * min = cvCreateImage(cvGetSize(image), 8, 3);
	IplImage* whereis = cvCreateImage(cvGetSize(image), 8, 3);
	writer1 = cvCreateVideoWriter("Result1.avi", CV_FOURCC('D', 'I', 'V', 'X'), 15, cvSize(temp->width, temp->height), 1); // ���� ���
	writer2 = cvCreateVideoWriter("Result2.avi", CV_FOURCC('D', 'I', 'V', 'X'), 15, cvSize(temp->width, temp->height), 1);
	writer3 = cvCreateVideoWriter("Result3.avi", CV_FOURCC('D', 'I', 'V', 'X'), 15, cvSize(temp->width, temp->height), 1);

	
	while (1) // ������ �Կ�
	{

		image = cvQueryFrame(capture);

		//CvCapture �κ��� �������� ��� image ��� IplImage��ü�� �ֽ��ϴ�
		if (moving_image(image, temp, min) == 1) //moving_image�Լ��� ȣ�� ���ϰ��� 1�̸� if�� ����
		{
			cnt++; //�� �����Ӹ��� ī��Ʈ�Ͽ� �� ī��Ʈ�� ���ϸ� �ֽ��ϴ�.
			itoa(cnt, name, 10);//����Ǵ»����̸���1,2,3,������ī��Ʈ��ȣ
			sprintf(buffer, "D:/cctv/%s.jpg", name);
			cvSaveImage(buffer, image); //������ ���ϸ����� �����մϴ�. (���������� ����� ���Ͽ� ����)
		}
		now = time / 10;
		if (now == 0) {
			printf("������ �Դϴ�. \n");
			sign = FALSE;
		}
		else if (now < 30) { // 30���� ������ ������
			sign = FALSE;
		}
		else if (now == 30.0) {
			sign = TRUE;
			printf("�ʷϺ��� �Ǿ����ϴ�.\n");
		}
		else if (now >= 30.0 && now <= 45.0) { // 30~ 45 ���̿� (15�� ����)�ʷϺ�
			sign = TRUE;
		}
		else { // ������ time �ʱ�ȭ
			printf("������ �Դϴ�. \n");
			time = 0;
		}
		time++; // time���� (�ʷϺ� ��� �ð�)
		count = find_image(image, whereis , count , sign); // find_image �Լ� ȣ�� �� ���ϰ� count�� ����
		cvCopy(image, temp); // �ٷ��� �̹����� ���ϱ� ���ؼ� temp�� image��(����)�� ����
						
		
		cvShowImage("Original", image); // �⺻ ������ �����ֱ�
		cvLine(min, cvPoint((image->width) / 2, 0), cvPoint((image->width / 2), image->height), cvScalar(255,255, 255), 1, 1); // ���׸���
		cvShowImage("Min-Show", min); // ������ ������ �����ִ� ������
		cvLine(whereis, cvPoint((image->width) / 2, 0), cvPoint((image->width / 2), image->height), cvScalar(255, 255, 255), 1, 1);
		cvShowImage("Find-show", whereis); // ��ü �̵��ϴ°��� ǥ���ϴ� ���� �����ִ� ������
		cvWriteFrame(writer1, temp); // ����������
		cvWriteFrame(writer2, min); // ����������
		cvWriteFrame(writer3, whereis); // ����������
		key = cvWaitKey(33); //�������� �޾ƿ��� �ӵ� 33/1000 �� �ʴ� 30������
		if (key == 27) { //  esc������ ����
			break;
		} 
	}
	//image_different();
	cvReleaseCapture(&capture); //�޸�����
	cvReleaseImage(&temp);
	cvReleaseImage(&min);
	cvReleaseImage(&whereis);
	cvDestroyAllWindows();
}

int moving_image(IplImage*img1, IplImage*img2, IplImage*min) //�������� ���ؼ� �������� �ִ��� Ȯ���� �ϱ� ���� �Լ� 
	{
	int idx;
	int Red, Blue, Green;
		
	int cnt = 0;
	// �������ӳ��� �ȼ��� ���� 2�� for��
	for (int i = 0; i<img1->height; i++) //�̹���������
	{
		for (int j = 0; j <img1->width; j++)
		{
			
			idx = (i * img1->widthStep +3 * j);
			Blue = abs(((BYTE)img1->imageData[idx]) - ((BYTE)img2->imageData[idx]));
			Green = abs(((BYTE)img1->imageData[idx + 1]) - ((BYTE)img2->imageData[idx+1]));
			Red = abs(((BYTE)img1->imageData[idx + 2]) - ((BYTE)img2->imageData[idx+2]));
			//�� �����ӳ����� R,G,B��� 40�̻��� �������̸� ���̸� �� ���� ī��Ʈ�ؼ�(cnt++)
			//�������� ������ �� ��� �ȼ��� ��ȭ�� �����״��� ���ϴ�.
			if (Blue>40 && Green>40 && Red>40 && j >= img1->width / 2) // RGB �����40�̻����̳����
			{
				min->imageData[idx] = (BYTE)img1->imageData[idx];
				min->imageData[idx + 1] = (BYTE)img1->imageData[idx + 1];
				min->imageData[idx + 2] = (BYTE)img1->imageData[idx + 2];
				cnt++; //ī��Ʈ��
				
			}
			//�� �ܿ� 40�̻� ���̳��� �ʴ� �ȼ��� ��쿣 0���� ä���� ���������� ����� ���̸� �����ֱ� ����
			else
			{
				min->imageData[idx] = 0;
				min->imageData[idx + 1] = 0;
				min->imageData[idx + 2] = 0;
			}
		}
	}




	if (cnt>1000) // cnt > 1000 �̸� 1�� ����  
		return 1;


	//�ܿ��� 0�� ����

	return 0;


}


void mouseHandlerFunc(int event, int x, int y, int flags, void *param) // ���콺 �̺�Ʈ ó��
{
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN: // ���� Ŭ���� �Կ� ����
		gCameraFlag = 1;
		break;
	case CV_EVENT_RBUTTONDOWN: // ������ Ŭ���� �Կ� ����
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
	char *temp = "TEMP�̹���.jpg";
	image = cvLoadImage(temp);
	int draw_x, draw_y, n_cnt;
	draw_x = 0;
	draw_y = 0;
	n_cnt = 1;
	for (int i = 0; i < img->height; i++) //�̹���������
	{
		for (int j = 0; j < img->widthStep; j += img->nChannels)
		{

			idx = (i * img->widthStep + j);
			Blue = abs(((BYTE)img->imageData[idx]) - ((BYTE)image->imageData[idx]));
			Green = abs(((BYTE)img->imageData[idx + 1]) - ((BYTE)image->imageData[idx + 1]));
			Red = abs(((BYTE)img->imageData[idx + 2]) - ((BYTE)image->imageData[idx + 2]));
			//�������� ������ �� ��� �ȼ��� ��ȭ�� �����״��� ���ϴ�.
			if (Blue >= 40 && Green >= 40 && Red >= 40 && j >= img->widthStep/2) // BGR 40�̻����̳����
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
		printf("[%d]�����Ͽ��� ������ �ֽñ� �ٶ��ϴ�\n",count/30);
	}
	else { 
		count = 0;
	}
	return count;
}


