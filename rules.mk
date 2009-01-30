
######################################################
## standart targets + recursion rule

.DEFAULT_GOAL := all
.PHONY : all clean distclean
.PHONY : install subdirs $(SUBDIRS) svnignore svnwarning
all clean distclean install svnignore : subdirs FORCE
# this one to prevent printing make[1]: Nothing to be done for `all'
FORCE:
	@#
subdirs : $(SUBDIRS)
$(SUBDIRS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)
unexport SUBDIRS
export DESTDIR


######################################################
## make output customization

# quiet -  'Q = @'
# normal - 'Q ='
Q := 
summary = @echo "$(1)" $(subst $(TOPDIR)/,,$(CURDIR)/)$(2)
ifeq ($Q,@)
MAKEFLAGS += --no-print-directory
endif

######################################################
## Compilation flags

##
# always recommended: warnings on, and path to #include <config.h>
CFLAGS += -Wall -I$(TOPDIR)
# select between debug or release build
# debug - debug symbols, no optimization, no striping
# release - striped, -O2 optimized code
ifeq ($(DEBUG),enabled)
CFLAGS += -g
STRIP = @true
else
CFLAGS += -O2
STRIP = strip
endif


######################################################
## Compilation rules

# SRCS - list of C source files to compile
ifneq (,$(SRCS))
OBJS += $(SRCS:.c=.o)
DEPS +=$(SRCS:.c=.d)
CLEANLIST += $(SRCS:.c=.o) $(SRCS:.c=.d)
endif



%.o : %.c
	$(call summary,CC  ,$@)
	$Q$(CC) $(CFLAGS) -c -o $@ $<


%.d : %.c
	$(call summary,DEP ,$@)
	$Q$(CC) $(CFLAGS) -M $< | sed 's,\(.*\)\.o[ :]*,$(@D)/\1.o $@ : ,g' > $@

% : %.in
	$(call summary,TEXT,$@)
	@#to have same perm
	$Qsed -f $(TOPDIR)/subst.sed  < $< > $@
	$Qchmod `stat --printf=%a $<` $@


######################################################
## Linkage rules

# binary
ifneq (,$(BINTARGET))
all : $(BINTARGET)
$(BINTARGET) : $(OBJS)
	$(call summary,LD  ,$@)
	$Q$(CC) $(OBJS) -o $@ $(LDFLAGS)
	$Q$(STRIP) $@

CLEANLIST += $(BINTARGET)
endif


# shared library lib*.so
ifneq (,$(LIBTARGET))
all : $(LIBTARGET)
$(LIBTARGET) : $(OBJS)
	$(call summary,LD  ,$@)
	$Q$(CC) $(OBJS) -o $@ $(LDFLAGS) -shared
	$Q$(STRIP) $@

CLEANLIST += $(BINTARGET)
endif

# ar archive
ifneq (,$(ARTARGET))
all : $(ARTARGET)
$(ARTARGET) : $(OBJS)
	@echo AR   $@
	$(call summary,CC  ,$@)
	@$(AR) r $@ $(OBJS)

CLEANLIST += $(ARTARGET)
endif


######################################################
## Clean rules

ifneq (,$(CLEANLIST))
clean:
	rm -f $(CLEANLIST)
endif

INS += $(wildcard *.in)
DISTCLEANLIST += $(INS:.in=)
ifeq ($(TOPDIR),$(CURDIR))
DISTCLEANLIST += config.h config.mk subst.sed
endif
distclean : clean
	rm -f $(DISTCLEANLIST)


######################################################
## Dependancy

ifeq ($(DEPENDENCY),enabled)
ifeq (,$(MAKECMDGOALS))
MAKECMDGOALS=all
endif
ifneq ($(findstring all, $(MAKECMDGOALS)),)
-include $(DEPS)
endif
endif


######################################################
## svn:ignore property automatiozation

ifeq (0,$(MAKELEVEL))
svnignore : svnwarning
ifneq ($(findstring svnignore, $(MAKECMDGOALS)),)
$(SUBDIRS) : svnwarning
endif
endif

svnwarning :
	@echo 
	@echo " Make is about to set svn:ignore property for every directory in a project"
	@echo " and update a repository. Property's value will consist from files" 
	@echo " that would be normaly deleted by clean or distclean target."
	@echo 
	@echo " Note: It's recommended to commit all changes and update a working copy beforehand."
	@echo " Note: If in doubt - press Ctrl-C."
	@echo 
	@read -p " Continue [y/N] ? " -e -t 5 ok; echo; [ "$$ok" == "y" -o "$$ok" == "Y" ]

svnignore:
	@prop=prop-$$$$.txt; \
	for i in $(DISTCLEANLIST) $(CLEANLIST); do echo "$$i"; done > $$prop;  \
	cat $$prop; \
	svn propset svn:ignore --file $$prop .; \
	rm -f $$prop


######################################################
## tar'ing the project

tar :
	cd $(TOPDIR)
	$(MAKE) distclean
	scripts/mk_tar


install=@$(TOPDIR)/scripts/install.sh
