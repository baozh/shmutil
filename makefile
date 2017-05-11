MAKE := make --no-print-directory

all :
	@$(MAKE) -f shmutil/makefile clean;
	@$(MAKE) -f shmutil/makefile;
	@$(MAKE) -f shmutil/makefile install;
	@$(MAKE) -f example/makefile clean;
	@$(MAKE) -f example/makefile;