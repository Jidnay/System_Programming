################################################################
################################################################
#	The entire directory is sintetized to a single static
#	library file, "api.a", that is placed in parent's directory.
################################################################
################################################################


################################################################
#					Targets and Flags
################################################################
# Specify a default target
.PHONY: all
# Specify the target files
all: clean ../api.a
# Specify the flags used
CFLAGS = -std=c99 -o3 -Wall -g

################################################################
#					Directory Dependencies
################################################################
#			$@ refers to the target of the rule
#			$< refers to the first dependency
#			$^ refers to all of the dependencies
################################################################
# Specify the build rule for the static library
../api.a: ../gamecore.a tcp.o display.o
	ar rcs ../api.a $^

# Specify the dependencies for each object
tcp.o: tcp.c api_.h -lpthread
display.o: display.c api_.h

# Target for building static library gamecore.a (Build subdirectory)
../gamecore.a: 
	$(MAKE) -C ../gamecore

################################################################
#					Generic Build rules
################################################################
# Specify the build rule for object files
%.o: %.c 
	gcc -c $(CFLAGS) $< -o $@

################################################################
#					  Delete objects
################################################################
clean:
	rm -f *.o *.a *.zip

################################################################
#				Command to create a zip file
################################################################
zip:
	zip ../api.zip *.c *.h Makefile

