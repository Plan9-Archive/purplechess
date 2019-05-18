</$objtype/mkfile
BIN=/$objtype/bin

TARG=purplechess
OFILES=\
	purplechess.$O\
	square.$O\
	guipart.$O\

HFILES=purple.h\
	guiparts.h\

UPDATE=\
	mkfile\
	$HFILES\
	${OFILES:%.$O=%.c}\

</sys/src/cmd/mkone

install:V:
	if(! test -e /sys/games/lib/chess/queen.bit){
		mkdir /sys/games/lib/chess
		dircp masks /sys/games/lib/chess
	}
	echo 'installed'
