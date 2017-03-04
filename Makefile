objs = main.o fileType.o httpCommon.o GetRequest.o PostRequest.o UrlCode.o
libs = -llog4cplus -llog -I/usr/local/lib
cc   = g++
flags = -W -Wall -g 
target = s.exe

$(target): $(objs)
	$(cc) $(flags) $(objs) $(libs) -o $(target)

main.o : main.cpp
	$(cc) $(flags) -c main.cpp

fileType.o : fileType.cpp
	$(cc) $(flags) -c fileType.cpp

httpCommon.o : httpCommon.cpp
	$(cc) $(flags) -c httpCommon.cpp

GetRequest.o : GetRequest.cpp
	$(cc) $(flags) -c GetRequest.cpp

PostRequest.o : PostRequest.cpp
	$(cc) $(flags) -c PostRequest.cpp

UrlCode.o : UrlCode.cpp
	$(CC) $(flags) -c UrlCode.cpp


clean:
	rm $(target) $(objs)

