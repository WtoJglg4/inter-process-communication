build-rw:
	g++ -o bin/readwriter cmd/readwriter/main.cpp

build-encoder:
	g++ -o bin/encoder cmd/encoder/main.cpp

run: build-rw build-encoder
	./bin/encoder tests/testfile1 tests/secret
	./bin/encoder tests/testfile2 tests/secret