
CFLAGS = 

LFLAGS= 

all: output 

clean:
	-$(RM) *.o output 

output: main.o
	g++ -o $@ main.o $(LFLAGS)
	
%.o:%.c
	g++ -cpp -o $@ $^ $(CFLAGS)