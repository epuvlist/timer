# Makefile for timer

project = timer

$(project) : $(project).cpp
	g++ -Wall -o $(project) $(project).cpp

clean:
	rm $(project).o $(project)



