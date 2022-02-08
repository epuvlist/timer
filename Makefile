# Makefile for timer_proto

project = timer

$(project) : $(project).o
	g++ -Wall -o $(project) $(project).o

$(project).o : $(project).cpp
	g++ -Wall -c $(project).cpp

clean:
	rm $(project).o $(project)


