/**************************************
* Original code: Kristofer Janukiewicz
* Github: https://github.com/KristoferJanukiewicz/Neural-Network-Approach-for-Optical-Character-Recognition/blob/master/Preprocessing.cpp
* Dataset (0-9, x, y): http://www.ee.surrey.ac.uk/CVSSP/demos/chars74k/ 2016-09-28
**************************************/

#include <stdlib.h>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <string.h>
#include <fstream>
using namespace std;
using namespace cv;


////////////////////////////////////////
#define TRAINING_SAMPLES 7			//Number of samples in training dataset
#define ATTRIBUTES 16				//Number of pixels per sample.
#define TEST_SAMPLES 7				//Number of samples in test dataset
#define CLASSES 16					//Number of distinct characters.
#define INPUT_PATH "./dataset"
#define OUTPUT_PATH_TRAINING "./preprocessed_output/trainingset.txt"
#define OUTPUT_PATH_TESTING "./preprocessed_output/testset.txt"
////////////////////////////////////////

int counter = 0;

void scaleDownImage(Mat &originalImg, Mat &scaledDownImage)
{
	for (int x = 0; x<ATTRIBUTES; x++)
	{
		for (int y = 0; y<ATTRIBUTES; y++)
		{
			int yd = ceil((float)(y*originalImg.cols / ATTRIBUTES));
			int xd = ceil((float)(x*originalImg.rows / ATTRIBUTES));
			scaledDownImage.at<uchar>(x, y) = originalImg.at<uchar>(xd, yd);

		}
	}
}

void cropImage(Mat &originalImage, Mat &croppedImage)
{
	int row = originalImage.rows;
	int col = originalImage.cols;
	int tlx, tly, bry, brx;	//t=top r=right b=bottom l=left
	tlx = tly = bry = brx = 0;
	float suml = 0;
	float sumr = 0;
	int flag = 0;

	/**************************top edge***********************/
	for (int x = 1; x<row; x++)
	{
		for (int y = 0; y<col; y++)
		{
			if (originalImage.at<uchar>(x, y) == 0)
			{

				flag = 1;
				tly = x;
				break;
			}

		}
		if (flag == 1)
		{
			flag = 0;
			break;
		}
	}
	/*******************bottom edge***********************************/
	for (int x = row - 1; x>0; x--)
	{
		for (int y = 0; y<col; y++)
		{
			if (originalImage.at<uchar>(x, y) == 0)
			{

				flag = 1;
				bry = x;
				break;
			}
		}
		if (flag == 1)
		{
			flag = 0;
			break;
		}

	}
	/*************************left edge*******************************/

	for (int y = 0; y<col; y++)
	{
		for (int x = 0; x<row; x++)
		{
			if (originalImage.at<uchar>(x, y) == 0)
			{

				flag = 1;
				tlx = y;
				break;
			}
		}
		if (flag == 1)
		{
			flag = 0;
			break;
		}
	}

	/**********************right edge***********************************/

	for (int y = col - 1; y>0; y--)
	{
		for (int x = 0; x<row; x++)
		{
			if (originalImage.at<uchar>(x, y) == 0)
			{

				flag = 1;
				brx = y;
				break;
			}
		}
		if (flag == 1)
		{
			flag = 0;
			break;
		}
	}
	int width = brx - tlx;
	int height = bry - tly;
	Mat crop(originalImage, Rect(tlx, tly, brx - tlx, bry - tly));
	if (width == 0 || height == 0){
		croppedImage = originalImage.clone();
	}
	else{
		croppedImage = crop.clone();
	}	
}

void convertToPixelValueArray(Mat &img, int pixelarray[])
{
	int i = 0;
	for (int x = 0; x<ATTRIBUTES; x++)
	{
		for (int y = 0; y<ATTRIBUTES; y++)
		{
			pixelarray[i] = (img.at<uchar>(x, y) == 255) ? 1 : 0;
			i++;
		}
	}
}


string convertInt(int number)
{
	stringstream ss;//create a stringstream
	ss << number;//add number to the stream
	return ss.str();//return a string with the contents of the stream
}

void readFile(std::string datasetPath, int samplesPerClass, std::string outputfile)
{
	fstream file(outputfile, ios::out);
	string folderName[CLASSES] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
								"y", "equals", "plus", "minus", "times", "division"};

	for (int sample = 1; sample <= samplesPerClass; sample++)
	{
		for (int folder = 0; folder < CLASSES; folder++)
		{   //creating the file path string
			std::string imagePath = datasetPath + "/" + folderName[folder] + "/img" +  convertInt(sample) + ".png";
			//std::cout << "imagePath: " << imagePath << std::endl;
			//reading the image
			
			Mat img = imread(imagePath, 0);

			Mat output;
			//Applying gaussian blur to remove any noise
			GaussianBlur(img, output, Size(5, 5), 0);
			//thresholding to get a binary image
			threshold(output, output, 50, 255, 0);
			
			//declaring mat to hold the scaled down image
			Mat scaledDownImage(ATTRIBUTES, ATTRIBUTES, CV_8U, Scalar(0));
			//declaring array to hold the pixel values in the memory before it written into file
			int pixelValueArray[ATTRIBUTES*ATTRIBUTES];

			//cropping the image.
			cropImage(output, output);
			//imshow("Test", output);
			//cvWaitKey(1);
			//reducing the image dimension to 16X16
			scaleDownImage(output, scaledDownImage);

			//reading the pixel values.
			convertToPixelValueArray(scaledDownImage, pixelValueArray);
			//writing pixel data to file
			for (int d = 0; d<ATTRIBUTES*ATTRIBUTES; d++){
				file << pixelValueArray[d] << ",";
			}
			//writing the label to file
			file << folder << "\n";
		}
	}
	file.close();
}

int main()
{
	cout << "Reading the training set...\n";
	readFile(INPUT_PATH, TRAINING_SAMPLES, OUTPUT_PATH_TRAINING);
	cout << "Reading the test set...\n";
	readFile(INPUT_PATH, TEST_SAMPLES, OUTPUT_PATH_TESTING);
	cout << "operation completed\n";
	return 0;
}