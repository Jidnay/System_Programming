################################################################
################################################################
#	The entire directory is sintetized to a single static library
#	file, "client_api.a", that is placed in parent's directory.
################################################################
################################################################


################################################################
#					Targets and Flags
################################################################
# Specify a default target
.PHONY: all
# Specify the target files
all: clean client
# Specify the flags used
CFLAGS = -std=c99 -o3 -Wall -g

################################################################
#					Directory Dependencies
################################################################
#			$@ refers to the target of the rule
#			$< refers to the first dependency
#			$^ refers to all of the dependencies
################################################################
# Target for building server executable
client: ../client_api.a client.o -lncurses

# Target for building static library client_api.a (Build subdirectory)
../client_api.a:
	$(MAKE) -C ../server

# Specify the dependencies for each object
client.o: client.c ../client_api.h -lpthread

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
	rm -f *.o *.zip client

################################################################
#				Command to create a zip file
################################################################
zip:
	zip ../client.zip *.c *.h Makefile

