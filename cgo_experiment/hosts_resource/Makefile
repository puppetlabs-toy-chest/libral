.PHONY: clean

TARGET=wrapper

$(TARGET): wrapper.o
	echo "wtf huh what?"

wrapper.o:
	g++ -c -std=c++11 -I/usr/local/include/ -I/usr/include/libxml2/ wrapper.cpp

clean:
	rm -f *.o $(TARGET)



