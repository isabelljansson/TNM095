
CXXFLAGS = -c -I/usr/local/Cellar/opencv3/HEAD-e0435d3_4/include/

LFLAGS= -L/usr/local/Cellar/opencv3/HEAD-e0435d3_4/lib/ -lm -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_videoio -lopencv_imgcodecs

all: output preprocessing training

clean:
	-$(RM) *.o output training

output: main.o
	g++ -o $@ main.o $(LFLAGS)

preprocessing: preprocessing.o
	g++ -o $@ preprocessing.o $(LFLAGS)

training: training.o
	g++ -o $@ training.o $(LFLAGS)
	
%.o:%.c
	g++ -o $@ $(CXXFLAGS) $^