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
//using namespace std;
//using namespace cv;


////////////////////////////////////////
#define TRAINING_SAMPLES 52			//Number of samples in training dataset
#define ATTRIBUTES 16				//Number of pixels per sample.
#define TEST_SAMPLES 3				//Number of samples in test dataset
#define CLASSES 16					//Number of distinct characters.
#define INPUT_PATH_TRAINING "./dataset"
#define INPUT_PATH_TESTING "./testset"
#define OUTPUT_PATH_TRAINING "./preprocessed_output/trainingset.txt"
#define OUTPUT_PATH_TESTING "./preprocessed_output/testset.txt"
////////////////////////////////////////

int counter = 0;

void scaleDownImage(cv::Mat &originalImg, cv::Mat &scaledDownImage)
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

void cropImage(cv::Mat &originalImage, cv::Mat &croppedImage)
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
	cv::Mat crop(originalImage, cv::Rect(tlx, tly, brx - tlx, bry - tly));
	if (width == 0 || height == 0){
		croppedImage = originalImage.clone();
	}
	else{
		croppedImage = crop.clone();
	}	
}

void convertToPixelValueArray(cv::Mat &img, int pixelarray[])
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


std::string convertInt(int number)
{
	std::stringstream ss;//create a stringstream
	ss << number;//add number to the stream
	return ss.str();//return a string with the contents of the stream
}

void readFile(std::string datasetPath, int start, int samplesPerClass, std::string outputfile)
{
	std::fstream file(outputfile, std::ios::out);
	std::string folderName[CLASSES] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
								"y", "equals", "plus", "minus", "times", "division"};

	for (int sample = start; sample <= samplesPerClass; sample++)
	{
		for (int folder = 0; folder < CLASSES; folder++)
		{   
		//creating the file path string
			std::string imagePath;
			std::ostringstream im, fold;
			fold << std::setw( 3 ) << std::setfill( '0' ) << (folder+1);
		    im << std::setw( 3 ) << std::setfill( '0' ) << sample;

		    if ( (folderName[folder] == "plus" ||
		    	folderName[folder] == "minus" ||
		    	folderName[folder] == "equals" ||
		    	folderName[folder] == "division")  ) {

		    	if (sample < 7)
			    	imagePath = datasetPath + "/" + folderName[folder] + "/img" +  convertInt(sample) + ".png";
			    else
			    	imagePath = datasetPath + "/" + folderName[folder] + "/empty.png";
		    }
			else 
				imagePath = datasetPath + "/" + folderName[folder] + "/img" +  fold.str() + "-" + im.str() + ".png";

			//std::cout << "imagePath: " << imagePath << std::endl;
			
			//reading the image
			cv::Mat img = cv::imread(imagePath, 0);

			cv::Mat output;
			//Applying gaussian blur to remove any noise

			cv::GaussianBlur(img, output, cv::Size(5, 5), 0);

			//thresholding to get a binary image
			cv::threshold(output, output, 50, 255, 0);
			
			//declaring mat to hold the scaled down image
			cv::Mat scaledDownImage(ATTRIBUTES, ATTRIBUTES, CV_8U, cv::Scalar(0));
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
	std::cout << "Reading the training set...\n";
	readFile(INPUT_PATH_TRAINING, 4, 55, OUTPUT_PATH_TRAINING);
	std::cout << "Reading the test set...\n";
	readFile(INPUT_PATH_TESTING, 1, 3, OUTPUT_PATH_TESTING);
	std::cout << "operation completed\n";
	return 0;
}