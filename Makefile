CC = clang
FRAMEWORKS = -framework CoreFoundation -framework IOKit

all: read-xkeys write-xkeys hid-dump-report
.PHONY: all

read-xkeys:
	$(CC) $(FRAMEWORKS) read-xkeys.c -o bin/read-xkeys

write-xkeys:
	$(CC) $(FRAMEWORKS) write-xkeys.c -o bin/write-xkeys

hid-dump-report:
	$(CC) $(FRAMEWORKS) -lncurses hid-dump-report.c -o bin/hid-dump-report

clean:
	rm -f bin/read-xkeys bin/write-xkeys
