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
all: clean server ../client_api.a
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
server: gamecore.a api.a server.o -lncurses

# Specify the build rule for the static library
# just move to parent's directory and change it's name
../client_api.a: api.a
	mv $< $@ 

# Target for building static library gamecore.a (Build subdirectory)
gamecore.a: 
	$(MAKE) -C gamecore

# Target for building static library api.a (Build subdirectory)
api.a:
	$(MAKE) -C api

# Specify the dependencies for each object
server.o: server.c server_api.h gamecore.h -lpthread

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
	rm -f *.o *.a *.zip server
	for dir in gamecore api; do \
		$(MAKE) -C $$dir clean; \
	done

################################################################
#				Command to create a zip file
################################################################
zip:
	zip ../server.zip *.a *.c *.h gamecore api Makefile

