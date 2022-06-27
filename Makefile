all::

FIND=find
AR=ar
XARGS=xargs
SED=sed
CAT=cat
M4=m4
ECHO=echo
CMP=cmp
CP=cp
GETVERSION=git describe

PREFIX=/usr/local

VERSION:=$(shell $(GETVERSION) 2> /dev/null)

## Provide a version of $(abspath) that can cope with spaces in the
## current directory.
myblank:=
myspace:=$(myblank) $(myblank)
MYCURDIR:=$(subst $(myspace),\$(myspace),$(CURDIR)/)
MYABSPATH=$(foreach f,$1,$(if $(patsubst /%,,$f),$(MYCURDIR)$f,$f))

sinclude $(call MYABSPATH,config.mk)
sinclude edges-env.mk

binaries.c += testunrast
testunrast_obj += testunrast
testunrast_obj += $(edges_mod)

headers += edges.h
headers += edges_version.h

libraries += edges
edges_mod += edges

cDOCS += VERSION
DOCS += README
DOCS += HISTORY
DOCS += COPYING

SOURCES:=$(filter-out $(headers),$(shell $(FIND) src/obj \( -name "*.c" -o -name "*.h" \) -printf '%P\n'))


riscos_zips += edges
edges_appname=!EdgesLib
edges_rof += Docs/COPYING,fff
edges_rof += Docs/VERSION,fff
edges_rof += Docs/README,fff
edges_rof += !Boot,feb
edges_rof += $(call riscos_src,$(SOURCES))
edges_rof += $(call riscos_hdr,$(headers))
edges_rof += $(call riscos_lib,$(libraries))

include binodeps.mk

all:: installed-libraries

lc=$(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

install:: install-libraries install-headers

ifneq ($(filter true t y yes on 1,$(call lc,$(ENABLE_RISCOS))),)
install:: install-riscos
all:: riscos-zips
endif

$(BINODEPS_OUTDIR)/riscos/!EdgesLib/!Help,fff: README.md
	$(MKDIR) "$(@D)"
	$(CP) "$<" "$@"

$(BINODEPS_OUTDIR)/riscos/!EdgesLib/COPYING,fff: LICENSE.txt
	$(MKDIR) "$(@D)"
	$(CP) "$<" "$@"

$(BINODEPS_OUTDIR)/riscos/!EdgesLib/VERSION,fff: VERSION
	$(MKDIR) "$(@D)"
	$(CP) "$<" "$@"

ifneq ($(VERSION),)
prepare-version::
	@$(MKDIR) tmp/
	@$(ECHO) $(VERSION) > tmp/VERSION

tmp/VERSION: | prepare-version
VERSION: tmp/VERSION
	@$(CMP) -s '$<' '$@' || $(CP) '$<' '$@'
endif

tmp/obj/edges_version.h: src/obj/edges_version.h.m4 VERSION
	@$(PRINTF) '[version header %s]\n' "$(file <VERSION)"
	@$(MKDIR) '$(@D)'
	@$(M4) -DVERSION='`$(file <VERSION)'"'" < '$<' > '$@'

tidy::
	@$(FIND) . -name "*~" -delete


# Set this to the comma-separated list of years that should appear in
# the licence.  Do not use characters other than [0-9,] - no spaces.
YEARS=2001,2005-6,2012

update-licence:
	$(FIND) . -name ".svn" -prune -or -type f -print0 | $(XARGS) -0 \
	$(SED) -i 's/Copyright (C) [0-9,-]\+  Steven Simpson/Copyright (C) $(YEARS)  Steven Simpson/g'

distclean:: blank
	$(RM) VERSION
