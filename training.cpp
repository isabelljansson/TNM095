/*
*	Bra länk om neuron network http://neuralnetworksanddeeplearning.com/chap1.html
*
*/

/*
THREE LAYERS IN THE NETWORK
- INPUT: 256 neurons - each image is 16*16 pixels -> 16*16 = 256 neurons
- HIDDEN: 16 neurons
- OUTPUT: 16 neurons, one for each class.

CLASSES
0-9:  	Numbers
10:		y
11:		=
12: 	+
13:		-
14:		* (x)
15:		/

*/

#include "opencv2/opencv.hpp"
#include "opencv2/ml.hpp"

#include <stdio.h>
#include <fstream>

using namespace std;
using namespace cv;
using namespace ml;

////////////////////////////////////////
#define CLASSES 16								//Number of distinct labels.
#define TRAINING_SAMPLES 7						//Number of samples in training dataset
#define ALL_TRAINING_SAMPLES (TRAINING_SAMPLES * CLASSES)       //All samples in training dataset
#define ATTRIBUTES 16							// Number of pixels per sample (16*16)
#define ALL_ATTRIBUTES (ATTRIBUTES * ATTRIBUTES)  // All pixels per sample.
#define TEST_SAMPLES 7						//Number of samples in test dataset
#define ALL_TEST_SAMPLES (TEST_SAMPLES * CLASSES)				//All samples in test dataset
#define INPUT_PATH_TRAINING "./preprocessed_output/trainingset.txt"
#define INPUT_PATH_TESTING "./preprocessed_output/testset.txt"
#define OUTPUT_PATH_XML "./param.xml"
////////////////////////////////////////


/********************************************************************************
Function created by Kristofer Janukiewicz
Github: https://github.com/KristoferJanukiewicz/Neural-Network-Approach-for-Optical-Character-Recognition/blob/master/Training.cpp

This function will read the csv files(training and test dataset) and convert them
into two matrices. classes matrix have 10 columns, one column for each class label. If the label of nth row in data matrix
is, lets say 5 then the value of classes[n][5] = 1.
Kristofer Janukiewicz
Github: https://github.com/KristoferJanukiewicz/Neural-Network-Approach-for-Optical-Character-Recognition/blob/master/Training.cpp
********************************************************************************/
void read_dataset(char *filename, cv::Mat &data, cv::Mat &classes, int total_samples)
{
	int label;
	float pixelvalue;
	//open the file
	FILE* inputfile = fopen(filename, "r");

	//read each row of the csv file
	for (int row = 0; row < total_samples; row++)
	{
		//for each attribute in the row
		for (int col = 0; col <= ALL_ATTRIBUTES; col++)
		{
			//if its the pixel value.
			if (col < ALL_ATTRIBUTES){

				fscanf(inputfile, "%f,", &pixelvalue);
				data.at<float>(row, col) = pixelvalue;
			}
			//if its the label
			else if (col == ALL_ATTRIBUTES){
				//make the value of label column in that row as 1.
				fscanf(inputfile, "%i", &label);
				classes.at<float>(row, label) = 1.0;
			}
		}
	}

	fclose(inputfile);
}




int main()
{
	//cout << "info opencv\n" << getBuildInformation() << endl;
	cout << "Training started..\n";

	// Create matrices
	Mat training_set(ALL_TRAINING_SAMPLES, ALL_ATTRIBUTES, CV_32F);//Training samples
	Mat training_set_classifications(ALL_TRAINING_SAMPLES, CLASSES, CV_32F); //index of each traning sample
	Mat test_set(ALL_TEST_SAMPLES, ALL_ATTRIBUTES, CV_32F); //Test samples
	Mat test_set_classifications(ALL_TEST_SAMPLES, CLASSES, CV_32F); //index of each test sample

	// Load training and test sets
	read_dataset(INPUT_PATH_TRAINING, training_set, training_set_classifications, ALL_TRAINING_SAMPLES);
	read_dataset(INPUT_PATH_TESTING, test_set, test_set_classifications, ALL_TEST_SAMPLES);
	
	// Intitialize the three layers
	Mat layers(3, 1, CV_32S); //3 rows, 1 col, 32 bit signed ints
	layers.row(0) = Scalar(ALL_ATTRIBUTES);
	layers.row(1) = Scalar(ATTRIBUTES);
	layers.row(2) = Scalar(CLASSES);
	//layers.at<int>(0, 0) = ALL_ATTRIBUTES;//input layer, 256 neurons
	//layers.at<int>(1, 0) = ATTRIBUTES;//hidden layer, 16 neurons
	//layers.at<int>(2, 0) = CLASSES;//output layer, 16 neurons

	// Create network
	// See http://docs.opencv.org/2.4/modules/ml/doc/neural_networks.html for more info
	// Activation function: Sigmoid function (default) with parameters a and b
	// a: 0.6
	// b: 1.0
	Ptr<ANN_MLP> nnetwork = ANN_MLP::create();
	nnetwork->setLayerSizes(layers);

	/*ANN_MLP::Params params = ANN_MLP::Params::Params(layers, ANN_MLP::SIGMOID_SYM, 1, 1, 
			TermCriteria( TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, 0.01 ), 
			ANN_MLP::Params::BACKPROP, 0.1, 0.1);*/

	//nnetwork->StatModel::train(training_set, params);


	nnetwork->setActivationFunction(ANN_MLP::SIGMOID_SYM, 1, 1);
	

	
	//Find weights w and biases b that minimazes the cost function


	cout << "Training completed..\n";

	return 0;
}
