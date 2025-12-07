################################################################
################################################################
#	The entire server directory is sintetized to a single static
#	library, "client_api.a", that is placed in parent's directory.
################################################################
################################################################


################################################################
#					Targets and Flags
################################################################
# Specify a default target
.PHONY: all
# Specify the target files
all: clean server_and_client

################################################################
#					Directory Dependencies
################################################################
#			$@ refers to the target of the rule
#			$< refers to the first dependency
#			$^ refers to all of the dependencies
################################################################
# Target for building server executable
server_and_client:
	$(MAKE) -C Server
	$(MAKE) -C Client

################################################################
#					  Delete objects
################################################################
clean:
	rm -rf *.o *.a *.zip
	for dir in Server Client; do \
		$(MAKE) -C $$dir clean; \
	done

################################################################
#				Command to create a zip file
################################################################
zip:
	zip ParteB.zip *.a *.h Makefile

