objs = main.o
cc   = g++
flags = -W -Wall -g 
target = s.exe

$(target): $(objs)
	$(cc) $(flags) $(objs) -o $(target)

clean:
	rm $(target) $(objs)

