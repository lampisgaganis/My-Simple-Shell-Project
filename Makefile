object=mysh.o
main : $(object)
	g++ -Wall -Werror $(object) -o mysh
clean : 
	rm main $(objects)