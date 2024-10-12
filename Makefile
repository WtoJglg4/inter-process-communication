build-reader:
	g++ -o bin/reader cmd/reader/main.cpp

build-encoder:
	g++ -o bin/encoder cmd/encoder/main.cpp

run: build-reader build-encoder
	./bin/encoder tests/testfile1 tests/secret
	./bin/encoder result.txt tests/secret