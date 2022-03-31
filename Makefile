./bin/gb_parse: ./src/gb_parse.c ./src/gb_parse.c
	mkdir -p ./bin
	gcc src/gb_parse.c -o ./bin/gb_parse

clean:
	rm ./bin/*
