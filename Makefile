
CFLAGS = -Wall -c -I/usr/local/include/

LFLAGS= -L/usr/local/lib/ -lm -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video

all: output 

clean:
	-$(RM) *.o output 

output: main.o
	g++ -I/usr/local/include/ -o $@ main.o $(LFLAGS)
	
%.o:%.c
	g++ -I/usr/local/include/ $(CFLAGS) -o $@ $^