target:
	g++ -ggdb -c src/*.cpp
	mv *.o obj/
	g++ -o rf_compress obj/*.o

clean:
	rm obj/*.o
