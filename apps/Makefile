SUBDIRS = shell lab studio notepad

all:
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir all; done

clean:
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
	rm -f lib/*.o
