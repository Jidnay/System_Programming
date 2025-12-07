################################################################
################################################################
#	The entire directory is sintetized to a single static library
#	file, "gamecore.a", that is placed in parent's directory.
################################################################
################################################################


################################################################
#					Targets and Flags
################################################################
# Specify a default target
.PHONY: all
# Specify the target files
all: clean ../gamecore.a
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
../gamecore.a: list.o utils.o worlds_dynamic.o
	ar -rcs ../gamecore.a $^

# Specify the dependencies for each object
list.o: list.c gamecore_.h
utils.o: utils.c gamecore_.h -lncurses
worlds_dynamic.o: worlds_dynamic.c gamecore_.h

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
	zip ../gamecore.zip *.c *.h Makefile

