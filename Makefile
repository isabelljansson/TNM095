
CXXFLAGS = -c -I/usr/local/include/

LFLAGS= -L/usr/local/lib/ 
#-lm -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_videoio

all: output preprocessing training

clean:
	-$(RM) *.o output 

output: main.o
	g++ -o $@ main.o $(LFLAGS)

preprocessing: preprocessing.o
	g++ -o $@ preprocessing.o $(LFLAGS)

training: training.o
	g++ -o $@ training.o $(LFLAGS)
	
%.o:%.c
	g++ -o $@ $(CXXFLAGS) $^