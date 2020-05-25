CC = clang
FRAMEWORKS = -framework CoreFoundation -framework IOKit

all: read-xkeys write-xkeys
.PHONY: all

read-xkeys:
	$(CC) $(FRAMEWORKS) read-xkeys.c -o bin/read-xkeys

write-xkeys:
	$(CC) $(FRAMEWORKS) write-xkeys.c -o bin/write-xkeys

clean:
	rm -f bin/read-xkeys bin/write-xkeys
