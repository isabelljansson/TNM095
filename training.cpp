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
#define CLASSES 7								//Number of distinct labels.
#define TRAINING_SAMPLES 3						//Number of samples in training dataset
#define ALL_TRAINING_SAMPLES (TRAINING_SAMPLES * CLASSES)       //All samples in training dataset
#define ATTRIBUTES 16							// Number of pixels per sample (16*16)
#define ALL_ATTRIBUTES (ATTRIBUTES * ATTRIBUTES)  // All pixels per sample.
#define TEST_SAMPLES 3						//Number of samples in test dataset
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
	

	// Create matrices for training
	Mat training_set(ALL_TRAINING_SAMPLES, ALL_ATTRIBUTES, CV_32F);//Training samples
	Mat training_set_classifications(ALL_TRAINING_SAMPLES, CLASSES, CV_32F); //index of each traning sample
	Mat train_responses( ALL_TRAINING_SAMPLES, CLASSES, CV_32F ); //store result weightage of each class

	// Create matrices for testing
	Mat test_set(ALL_TEST_SAMPLES, ALL_ATTRIBUTES, CV_32F); //Test samples
	Mat test_set_classifications(ALL_TEST_SAMPLES, CLASSES, CV_32F); //index of each test sample
	Mat class_result(1, CLASSES, CV_32F);

	// Load training and test sets
	// Store data in matrices training_set and test_set
	read_dataset(INPUT_PATH_TRAINING, training_set, training_set_classifications, ALL_TRAINING_SAMPLES);
	read_dataset(INPUT_PATH_TESTING, test_set, test_set_classifications, ALL_TEST_SAMPLES);

	// Intitialize the three layers
	Mat layers(3, 1, CV_32SC1); //3 rows, 1 col, 32 bit signed ints
	layers.row(0) = Scalar(ALL_ATTRIBUTES); //input layer, 256 neurons
	layers.row(1) = Scalar(ATTRIBUTES); //hidden layer, 16 neurons
	layers.row(2) = Scalar(CLASSES); //output layer, 16 neurons

	// Create network
	// See http://docs.opencv.org/2.4/modules/ml/doc/neural_networks.html for more info
	// Activation function: Sigmoid function (default) with parameters a and b
	// a: 0.6
	// b: 1.0
	Ptr<ANN_MLP> nn = ANN_MLP::create();
	nn->setActivationFunction(ANN_MLP::SIGMOID_SYM, 0.6, 1.0);
	nn->setTrainMethod(ANN_MLP::BACKPROP);
	nn->setBackpropMomentumScale(0.1);
	nn->setBackpropWeightScale(0.1);
	nn->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, (int)1000, 1e-6));
	nn->setLayerSizes(layers);

	cout << "Starting training...\n";
	Ptr<TrainData> tdata = TrainData::create(training_set, ROW_SAMPLE, train_responses);
	nn->train(tdata);
	cout << "Training completed...\n";

	// Save the model into an xml file
	cout << "Saving model to xml file...\n";
	FileStorage storage(OUTPUT_PATH_XML, FileStorage::WRITE);
	nn->write(storage);

	// Test neuron network by predicting the classes for test_set
	cv::Mat test_sample;
	int correct_class = 0;//count of correct classifications
	int wrong_class = 0;//count of wrong classifications
	//classification matrix gives the count of classes to which the samples were classified.
	int classification_matrix[CLASSES][CLASSES] = { {} };

	cout << "Testing neuron network...\n";
	for (int tsample = 0; tsample < ALL_TEST_SAMPLES; tsample++) {

		// Extract a sample
		test_sample = test_set.row(tsample);

		// Try to predict its class 
		// class_result will store the weights from each class
		nn->predict(test_sample, class_result);

		// Find the best class in class_result (the heighest weight)
		int maxIndex = 0;
		float value = 0.0f;
		float maxValue = class_result.at<float>(0, 0);
		for (int index = 1; index < CLASSES; index++)
		{
			value = class_result.at<float>(0, index);
			if (value>maxValue)
			{
				maxValue = value;
				maxIndex = index;
			}
		}
		//cout << "Testing Sample: " << tsample << " -> Class result: " << maxIndex << "\n";
		//Now compare the predicted class to the actural class. if the prediction is correct then\
		            //test_set_classifications[tsample][ maxIndex] should be 1.
		//if the classification is wrong, note that.
		if (test_set_classifications.at<float>(tsample, maxIndex) != 1.0f) {
			// if they differ more than floating point error => wrong class
			wrong_class++;
			//find the actual label 'class_index'
			for (int class_index = 0; class_index<CLASSES; class_index++) {
				if (test_set_classifications.at<float>(tsample, class_index) == 1.0f) {
					classification_matrix[class_index][maxIndex]++;// A class_index sample was wrongly classified as maxindex.
					break;
				}
			}
		}
		else {
			// otherwise correct
			correct_class++;
			classification_matrix[maxIndex][maxIndex]++;
		}
	}
	cout << "\n\nResults on the testing dataset\n"
		 << "Correct classification: " << correct_class << "  (" 
		 << (double)(correct_class * 100 / ALL_TEST_SAMPLES) << "%)\n"
		 << "Wrong classification: " << wrong_class << "  (" 
		 << (double)(wrong_class * 100 / ALL_TEST_SAMPLES) << "%)\n";
	

	

	return 0;
}
