</$objtype/mkfile

TARG=purplechess
NAMES=$TARG square
OFILES=${NAMES:%=%.$O}
HFILES=purple.h
LIBS=libguiparts
INCLUDEFILES=libguiparts/guiparts.h
CLEANFILES=
MANFILES=

LDFLAGS=
YFLAGS=-d
AFLAGS=
BIN=/$objtype/bin
LIB=`{echo /$objtype/lib/$LIBS.a}
INCLUDES=`{for(i in $INCLUDEFILES) echo /sys/include/`{basename $i}}
MAN=/sys/man

default:V: all

lib/$objtype:
	mkdir -p lib/$objtype

include:
	mkdir -p include

/$objtype/lib/%.a:
	cd $stem
	mk
	cp $stem.a$O ../lib/$objtype/$stem.a

/sys/include/%.h: 
	cp $INCLUDEFILES include

$O.out: $OFILES
	$LD $LDFLAGS -o $target $prereq

%.$O:	$HFILES	$INCLUDES $LIB

%.$O:	%.c
	$CC $CFLAGS $stem.c

$BIN/$TARG:	$O.out
	cp $prereq $BIN/$TARG

%.acid: %.$O $HFILES
	$CC $CFLAGS -a $stem.c >$target

$MAN/([^/\.]+)/([^/\.]+):R:	\2.\1.man
	echo $prereq $target

all:V: include lib/$objtype
	rfork n
	bind -b include /sys/include
	bind -b lib/$objtype /$objtype/lib
	mk $O.out

install:V: man
	mk all
	if(! test -e /sys/games/lib/chess/queen.bit){
		mkdir /sys/games/lib/chess
		dircp masks /sys/games/lib/chess
	}
	cp $O.out $BIN/$TARG

installall:V:
	for(objtype in $CPUS)
		mk install

allall:V:
	for(objtype in $CPUS)
		mk all

clean:V:
	rm -f *.[$OS] [$OS].out y.tab.? lex.yy.c y.debug y.output $TARG $CLEANFILES
	rm -rf lib include
	for(lib in $LIBS) @{
		cd $lib
		mk clean
	}

nuke:V:
	rm -f *.[$OS] [$OS].out y.tab.? lex.yy.c y.debug y.output *.acid $TARG $CLEANFILES
	rm -rf lib include
	for(lib in $LIBS) @{
		cd $lib
		mk nuke
	}

safeinstall:V: $O.out
	test -e $BIN/$TARG && mv $BIN/$TARG $BIN/_$TARG
	cp $prereq $BIN/$TARG

update:V:
	update $UPDATEFLAGS $UPDATE

safeinstallall:V:
	for (objtype in $CPUS)
		mk safeinstall

man:V:
	for(i in $MANFILES){
		echo $i
#		PAGE=`{echo $i | sed ''s/([^.]+)\.([^.]+)\.man/\2\/\1/g''}
		mk $MKFLAGS $MAN/$PAGE
	}
