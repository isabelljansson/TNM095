/* Frågor:

* Preprocessing or only training?
* Do we need a dataset of images of all numbers, operators and X? Or could we define them ourselves?

* 

*/



/*
The neural network has 3 layers.
- one input node per attribute in a sample so 256 input nodes
- 16 hidden nodes
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

using namespace std;
using namespace cv;


#define VARIABLE 10
#define EQUAL 11
#define ADD 12
#define SUB 13
#define MULT 14
#define DIV 15

#define lPARENTHESIS 16
#define rPARENTHESIS 17


bool isNumber(int read) {
	return read < VARIABLE;
}

bool isVariable(int read) {
	return read == VARIABLE;
}

bool isOperator(int read) {
	return (read > VARIABLE && read < lPARENTHESIS);
}

/* Returns 
True if:
* read = left parenthesis and bracket is closed
* read = right parenthesis and bracket is open
False if:
* read = left parenthesis and bracket is open
* read = right parenthesis and bracket is closed
*/
bool isParenthesis(int read, bool open) {
	if(!open)
		return read == lPARENTHESIS;
	else
		return read == rPARENTHESIS;

}

void toggle(bool open) {
	open = !open;
}


int main(int argc, char** argv)
{
	bool parOpen = false;
	int lastRead = -1;
	int read = -1;

	if (isVariable(lastRead)) {
		// allowed
		isOperator, isParenthesis

		// disallowed
		isNumber, isVariable

	}
	else if(isNumber(lastRead)) {
		// allowed for current read
		isNumber, isOperator, isParenthesis

		// disallowed
		isVariable
	}
	else if(isOperator(lastRead)) {
		// allowed for current read
		isNumber, isVariable, isParenthesis

		// disallowed
		isOperator
	}
	else if(isParenthesis(lastRead, parOpen)) {
		toggle(parOpen);
		// allowed for current read
		isNumber, isVariable

		// disallowed
		isParenthesis, isOperator,
	}


	// when done writing
	if(!parOpen) {
		// Error: Expecting closing parenthesis
	}

}




