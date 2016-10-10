/* Frågor:

* Preprocessing or only training?
* Do we need a dataset of images of all numbers, operators and X? Or could we define them ourselves?

* 

*/



/*
The neural network has 3 layers.
- one input node per attribute in a sample so 256 input nodes
- 16 hidden nodes (3 hidden nodes?)
- 16 output node, one for each class.


0-9:  	Numbers
10:		X
11:		=
12: 	+
13:		-
14:		*
15:		/
	
16:		( 
17: 	)

*/


#include <stdlib.h>

#include "opencv2/opencv.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <vector>
#include <math.h>
#include <algorithm>

using namespace std;
using namespace cv;
using namespace ml;


#define VARIABLE 10
#define EQUAL 11
#define ADD 12
#define SUB 13
#define MULT 14
#define DIV 15

#define lPARENTHESIS 16
#define rPARENTHESIS 17

////////////////////////////////////////
#define CLASSES 16               // Number of distinct labels.
#define ATTRIBUTES 16             // Number of pixels per sample (16x16)
#define ALL_ATTRIBUTES (ATTRIBUTES*ATTRIBUTES)  // All pixels per sample.
#define INPUT_PATH_XML "./param.xml"
#define CONTOUR_SIZE 35             // Accept found letters bigger than this size
#define SCENE_SIZE_X 800            // Render in this size
#define SCENE_SIZE_Y 600
////////////////////////////////////////

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

std::vector<cv::Rect> detectLetters(cv::Mat img)
{
  std::vector<cv::Rect> boundRect;
  cv::Mat img_gray, img_sobel, img_threshold, img_denoise, element;
  cvtColor(img, img_gray, CV_BGR2GRAY); // Make the image gray
  //fastNlMeansDenoising(img_gray, img_denoise, 10);
  cv::Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 5, 0, cv::BORDER_DEFAULT); // apply sobel-filter
  cv::threshold(img_sobel, img_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY); // threshold it
  element = getStructuringElement(cv::MORPH_RECT, cv::Size(30, 30));
  cv::morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element); //Does the trick
  std::vector< std::vector< cv::Point> > contours;
  cv::findContours(img_threshold, contours, 0, 1);
  std::vector<std::vector<cv::Point> > contours_poly(contours.size());

  for (int i = 0; i < contours.size(); i++)
  if (contours[i].size()>CONTOUR_SIZE)
  {
    cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true);
    cv::Rect appRect(boundingRect(cv::Mat(contours_poly[i])));

    //if (appRect.width>appRect.height)
      boundRect.push_back(appRect);
  }

  return boundRect;
}

bool isOperator(char c)
{
	return (c == '+' || c == '-' || c == '*' || c == '/' || c == '=');
}

bool isOperator(string s)
{
  return (s == "+" || s == "-" || s == "*" || s == "/" || s == "=");
}


void convert(const string & inString, string & outString);
bool isOperand(char c);
bool takesPrecedence(char operatorA, char operatorB);
bool isValid(string);
string format(string);
string addComma(string);
double calc(string[]);
string getAnswer(vector<string> array);
string eval(string);

int main(int argc, char** argv) {


  //read the model from the XML file and create the neural network.
  FileStorage storage(INPUT_PATH_XML, FileStorage::READ);
  Ptr<ANN_MLP> nn = Algorithm::read<ANN_MLP>(storage.root());
  //Ptr<ANN_MLP> nn; // = ANN_MLP::create();
  
  //nn->Algorithm::read(storage);

	VideoCapture cap(0); //default camera

	cout << "Program has started.. \n";

	//test if camera is opened
	if (!cap.isOpened()) {
		cout << "Could not open camera" << endl;
		return -1;
	}

	while(true) {
		Mat frame;
    cap >> frame;
    resize(frame, frame, Size(SCENE_SIZE_X, SCENE_SIZE_Y), 0, 0, INTER_AREA);
		//namedWindow( "Display window", WINDOW_AUTOSIZE );
		//flip(frame, frame, 1); // Mirror image around the y-axis

    //Detect
    std::vector<cv::Rect> letterBBoxes1 = detectLetters(frame);

    if (letterBBoxes1.size() > 0)
    {
      // Loop through the found letters
      for (int i = 0; i < letterBBoxes1.size(); i++)
      {
        Mat test_img = frame(letterBBoxes1[i]);
        Mat output;
        Mat img_gray, img_sobel, img_threshold, element;
        GaussianBlur(test_img, output, cv::Size(5, 5), 0);
        
        cvtColor(output, img_gray, CV_BGR2GRAY);
        //Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 5, 0, cv::BORDER_DEFAULT);
        threshold(img_gray, img_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
        //threshold(img_sobel, img_threshold, 150, 255, 0);
        //element = getStructuringElement(cv::MORPH_RECT, cv::Size(10, 10));
        //morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element); //Does the trick

        //imshow("Test...", img_threshold);
        //cvWaitKey(1);

        Mat scaledDownImage(ATTRIBUTES, ATTRIBUTES, CV_8U, cv::Scalar(0));
        int pixelValueArray[ALL_ATTRIBUTES];
        scaleDownImage(img_threshold, scaledDownImage);
        convertToPixelValueArray(scaledDownImage, pixelValueArray);

        Mat data(1, ALL_ATTRIBUTES, CV_32F);
        for (int i = 0; i < ALL_ATTRIBUTES; i++){
          data.at<float>(0, i) = (float)pixelValueArray[i];
        }

        int maxIndex = 0;
        cv::Mat classOut(1, CLASSES, CV_32F);

        //prediction
        nn->predict(data, classOut);
        float value;
        float maxValue = classOut.at<float>(0, 0);
        for (int index = 1; index<CLASSES; index++)
        {
          value = classOut.at<float>(0, index);
          if (value>maxValue)
          {
            maxValue = value;
            maxIndex = index; //maxIndex is the predicted class.
          }
        }

        // Check what character we found
        string text;
        switch(maxIndex) {
          case VARIABLE:
            text = "y";
            break;
          case EQUAL:
            text = "=";
            break;
          case ADD:
            text = "+";
            break;
          case SUB:
            text = "-";
            break;
          case MULT:
            text = "*";
            break;
          case DIV:
            text = "/";
            break;
          default:
            text = to_string(maxIndex);
            break;
        }

        // Write text (the letter) next to the found letter
        int fontFace = CV_FONT_HERSHEY_SIMPLEX;
        double fontScale = 1;
        int thickness = 2;
        Point textOrg(letterBBoxes1[i].x + letterBBoxes1[i].width / 2, letterBBoxes1[i].y - 12);
        putText(frame, text, textOrg, fontFace, fontScale, Scalar::all(55), thickness, 8);
        rectangle(frame, letterBBoxes1[i], cv::Scalar(0, 255, 0), 3, 8, 0);


        // create a vector for each box (character) and store the predicted value in it
        // if last 10 values in vector = temp --> use temp as the character in equation solver
      }
    }

    imshow("Neo - The Equation Solver", frame);
    cvWaitKey(1);
	}

	char reply;
 
  do {
  	string inString, outString; // local to this loop
    string y;

    cout <<"\n    Enter your expression with No spaces!\n\n";
    cout <<"     e.g. (4+2)*3/2 "<< endl;
    cout <<"    Unknown variables such as 'a' and 'x' are not allowed:\n\n>>";
    cin >> inString;

		// Assume the reading of the equation is done, predicted and put into a string called inString.
		if(isValid(inString)) {
	      convert(format(inString), outString);
		    outString = addComma(outString);  
		    y = eval(outString);

        cout << "\n\nAnswer: y = " << y;

		    cout << endl << "\nDo another (y/n)? ";
		    cin >> reply;
		}
		else {
		    cout<<"*** Syntax error ***\n";
		    cout << endl << "Do another (y/n)? ";
		    cin >> reply;
		     
		}          
  } while(tolower(reply) == 'y');

}


// Confirms validity of expression
bool isValid(string myString) {
  char lastRead = myString[0];
  string array="0123456789y+-*/=()";
  int count = 0, y = 0, equal = 0;

  // Only valid input for first character is y, digit or -
  if(!isdigit(lastRead) && lastRead != 'y' && lastRead != '-' && lastRead != '(') {
    cout << "First character in expression is not allowed \n";
  	return false;
  }

  // logic of expression
  for(int i = 1; i < myString.length(); i++) {
  	if(lastRead == 'y' && (myString[i] == 'y' || isdigit(myString[i]))) {
      cout << "Last character was a letter, letter or digit is not allowed \n";
  		return false;
    }
		else if(isdigit(lastRead) && myString[i] == 'y') {
      cout << "Last character was a digit, letter is not allowed \n";
			return false;
    }
		else if(isOperator(lastRead) && isOperator(myString[i])) {
      if (myString[i] == '-')
        continue;
      else {
        cout << "Can't have two operators in succession \n";
        return false;
      }
    }
		else if(lastRead == '(') {
			count++;
      if (myString[i] == '-' || myString[i] == '+') {
        lastRead = myString[i];
        continue;
      }
			else if(myString[i] == ')' || isOperator(myString[i])) {
        cout << "Disallowed character after left bracket ( \n";
				return false;
      }
		}
    else if(lastRead == ')') {
      count--;
      if(myString[i] == '(' || isdigit(myString[i]) || myString[i] == 'y') {
        cout << "Disallowed character after right bracket ) \n";
        return false;
      }
    }
    lastRead = myString[i];         
  }

  if(myString[myString.length()-1] == ')')
    count--;
  if(count != 0) {
  	cout << "Check parenthesis \n";
  	return false;
  }

  // Confirms there are no other characters but the valid ones
  for(int i = 0; i < myString.length(); i++) {
    if (myString[i] == '=')
      equal++;
    else if (myString[i] == 'y')
      y++;
    for(int j = 0; j < array.length(); j++)
      if(myString[i] == array[j])
        count++;        
  }

  if (equal != y || y > 1 || equal > 1) {
    cout << "Need both = and y in equation or too many y or = in expression \n";
    return false;
  }

  return (count == myString.length());          
}  

// Add delimiter between characters in string
string format(string myString) {
  for(int i = 0; i < myString.length(); i++) {
    if (myString[i] == 'y')
      myString.insert(i+1, "d");
    else if((isdigit(myString[i]) && !isdigit(myString[i+1]))) 
      myString.insert(i+1, "d");
    else if(myString[i] == '-' && (myString[i-1] != 'd' && myString[i-1] != ')'))
      myString.replace(i,1,"m"); 
  }

  return myString;
}

// Convert to stack
void convert(const string & inString, string &outString) {
  stack<char> operatorStack;
  char topSymbol, symbol;
  int k;

  for(k = 0; k < inString.size(); k++) {
    symbol = inString[k];
    if(isOperand(symbol))
      outString = outString + symbol;
    else {
      while(!operatorStack.empty() && takesPrecedence(operatorStack.top(), symbol)) {
        topSymbol = operatorStack.top();
        operatorStack.pop();
        outString = outString + topSymbol;
      }
      if(!operatorStack.empty() && symbol == ')')
        operatorStack.pop();   // discard matching (
      else
        operatorStack.push(symbol);
    }
  }
 
  while(!operatorStack.empty()) {
    topSymbol = operatorStack.top();
    operatorStack.pop();
    outString = outString + topSymbol;
  }
}

// Confirms c is an operand (letter or number)
bool isOperand(char c) {
	return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')));
}
 
// A and B can be an operator or parenthesis, returns true if A takes precedence.
bool takesPrecedence(char operatorA, char operatorB) {
  if(operatorA == '=')
  	return true;
  else if(operatorA == '(')
    return false;
  else if(operatorB == '(')
    return false;
  else if(operatorB == ')')
    return true;
  else if(operatorA == '*' || operatorA == '/')
    return true;
  else if(operatorB == '*' || operatorB == '/')
    return false;
  else
    return true;
}

// Change delimiter to comma
string addComma(string myString) {
  for(int i = 0; i < myString.length(); i++) {
    if(isOperator(myString[i]))
      myString.insert(i+1, ",");
    else if (myString[i] == 'd')
      myString.replace(i, 1, ",");
    else if (myString[i] == 'm')
      myString.replace(i, 1, "-");               
  }  
  return myString; 
}

// Evaluate equation or expression
string eval(string myString) {
  vector<string> v;
  vector<string> array;
  std::vector<string>::iterator it;
  vector<string> C;			// Opposite side of = that y is on
  vector<string> leftSide;	// left side of the y sign
  vector<string> rightSide;	// right side of the y sign
  bool negative = false;

  int indexY = -1, indexE = -1;
  //Evaluate tokens using the "," as a delimiter
  while(myString.find(",", 0) != string::npos) { 
    size_t pos = myString.find(",", 0); 
    v.push_back(myString.substr(0, pos)); //store in vector     
    myString.erase(0, pos + 1);           
  }

  // find index of Y and E, only do swap if we have both in the list
  it = find(v.begin(), v.end(), "y");
  indexY = it - v.begin();
  it = find(v.begin(), v.end(), "=");
  indexE = it - v.begin();
  if(indexY == v.size() && indexE != v.size()) {
    negative = true;
  	indexY = find(v.begin(), v.end(), "-y") - v.begin();
  }

  // There is no equation, return expression
  if(indexE == v.size()) {
    if (negative)
      return getAnswer(v).insert(0, "-");
    else
      return getAnswer(v);

  }
  // Rearrange Y and = index if neighbour case, split vector into 3 parts
  if (*(it-1) == "y" || *(it-1) == "-y" || isdigit(*(it-1)->c_str())) {
  	swap(*it, *(it-1));
  	indexE--;
  	if (*it == "y" || *it == "-y") {
  		indexY = it - v.begin();
    }
  }
  // y = () expression case
  it = v.begin();
  if (*it == "=" && (*(it+1) == "y" || *(it+1) == "-y")) {
    swap(*it, *(it+1));
    indexE++;
    indexY--;
  }

  // Make C, leftSide and rightSide
  if(indexY < indexE) {
    for(std::vector<string>::const_iterator i = v.begin() + indexE + 1; i != v.end(); ++i)
      C.push_back(*i);
    for(std::vector<string>::const_iterator i = v.begin(); i != v.begin() + indexY; ++i)
      leftSide.push_back(*i);
    for(std::vector<string>::const_iterator i = v.begin() + indexY + 1; i != v.begin() + indexE; ++i)
      rightSide.push_back(*i);
  }
  else if (indexY > indexE) {
    for(std::vector<string>::const_iterator i = v.begin(); i != v.begin() + indexE; ++i)
      C.push_back(*i);
    for(std::vector<string>::const_iterator i = v.begin() + indexE + 1; i != v.begin() + indexY; ++i)
      leftSide.push_back(*i);
    for(std::vector<string>::const_iterator i = v.begin() + indexY + 1; i != v.end(); ++i)
      rightSide.push_back(*i);
  }


  string iC = getAnswer(C);
  string iL = getAnswer(leftSide);
  vector<string> pushOpposite;
  bool consecutive = false;
  string temp;

  it = rightSide.begin();
  // y = expression CASE
  if (it == rightSide.end()) {
    if(negative)
      iC.insert(0, "-");
    return iC;
  }
  // + and digit CASE
  else if(*it == "+" || isdigit(*it->c_str())) {
  	array.push_back(iC);
    array.push_back(iL);

    // digit -> * or digit -> / CASE
  	if(isdigit(*it->c_str())) {
      array.pop_back();
      if(*(it+1) == "*") {
        pushOpposite.push_back(*it);
        pushOpposite.push_back("/");
        it++;
        consecutive = true;
      }
      else if (*(it+1) == "/") {
        pushOpposite.push_back(*it);
        pushOpposite.push_back("*");
        it++;
        consecutive = true;
      }
      else
        array.push_back(*it);
      it++;
    }
      
  	for(; it != rightSide.end(); ++it) {
      if(*it == "+") {
        consecutive = false;
        array.push_back("-");
      }
      else if(*it == "-") {
        consecutive = false;
        array.push_back("+");
      }
      else
  		  array.push_back(*it);
      if(consecutive) {
        temp = *(array.end() - 1);
        if(*(it+1) == "*") {
          array.pop_back();
          it++;
          pushOpposite.push_back(temp);
          pushOpposite.push_back("/");
        }
        else if (*(it+1) == "/") {
          array.pop_back();
          it++;
          pushOpposite.push_back(temp);
          pushOpposite.push_back("*");
        }
      }
    }
    if(!pushOpposite.empty())
      array.insert(array.end(), pushOpposite.begin(), pushOpposite.end());
  } 
  // - CASE
  else if(*it == "-") {
  	iC.insert(0, "-");
  	array.push_back(iC);
  	array.push_back(iL);
  	array.push_back("+");
    it++;
  	for(; it != rightSide.end(); ++it) {
  		array.push_back(*it);
  	}
  } 
  // * and / CASE
  else if(*it == "*" || *it == "/") {
    // split up iL
    temp = *(leftSide.end() - 1);
    leftSide.pop_back();
    iL = getAnswer(leftSide);
    pushOpposite.push_back(temp);
    consecutive = true;
    if(*it == "*")
      pushOpposite.push_back("/");
    else if(*it == "/")
      pushOpposite.push_back("*");

    array.push_back(iC);
    array.push_back(iL);
    array.push_back("-");
    it++;
    for(; it != rightSide.end(); ++it) {
      if (*it == "+") {
        consecutive = false;
        array.push_back("-");
      }
      else if (*it == "-") {
        consecutive = false;
        array.push_back("+");
      }
      else
        array.push_back(*it);
      if(consecutive) {
        temp = *(array.end() - 1);
        if(*(it+1) == "*") {
          array.pop_back();
          it++;
          pushOpposite.push_back(temp);
          pushOpposite.push_back("/");
        }
        else if (*(it+1) == "/") {
          array.pop_back();
          it++;
          pushOpposite.push_back(temp);
          pushOpposite.push_back("*");
        }
      }
    }

    if(!pushOpposite.empty())
      array.insert(array.end(), pushOpposite.begin(), pushOpposite.end());

  }

  // Print out final expression
  cout << "y = ";
  it = array.begin();
  for(; it != array.end(); ++it)
    cout << *it << " ";


  // Done with algebra, return answer
  if (negative)
    return getAnswer(array).insert(0, "-");
  else
    return getAnswer(array);

}

// Return a string that is the answer from the sent in vector
string getAnswer(vector<string> array) {
	stack<string> my_stack; //initialise stack
  	string temp[100];
  	double z = 0;
  	if (array.size() == 1)
  		z = stod(array[0]);
  	else {
  		for(int i = 0; i < array.size(); i++) {
	    	if(!isOperator(array[i])) {
	      		my_stack.push(array[i]); //push numbers onto the stack
	    	}
	    	else { //i.e if it encounters an operator
		    	my_stack.push(array[i]);//push operator onto stack
		      	temp[0]= my_stack.top();//store value
		      	my_stack.pop(); //erase from the stack

		      	temp[1]= my_stack.top();//store value
		      	my_stack.pop();//erase from the stack

		      	temp[2]= my_stack.top();//store value
		      	my_stack.pop();//erase from the stack

		      	z = calc(temp);

		      	ostringstream outs;  // Declare an output string stream.
		      	outs << z;   // Convert value into a string.
		      	my_stack.push(outs.str());
			}                
		}	
  	}
  	
	ostringstream outString;
	outString << z;
	return outString.str();
}
  

// Calculate the result of two digits and an operand
double calc(string temp[]) {
  double x = stod(temp[2]);
  double y = stod(temp[1]);
  string b = temp[0];

  if(b == "+")
    return (x + y); 
  else if (b == "-")
    return (x - y);
  else if (b == "*")
    return (x * y);
  else if (b == "/")
    return (x / y);
  else if (b == "=")
    return pow(x, y);
}


