COMP=clang
OPTIONS=-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-but-set-variable -O3
DEBUG=-fsanitize=address,undefined -g3

SOURCES=main.c mpv.c ui.c playlist.c
OUTPUT=amp
LIBS=-lmpv -lglfw -lGL -ldrawtext

final:
	${COMP} ${OPTIONS} ${SOURCES} ${LIBS} -o ${OUTPUT}

debug:
	${COMP} ${OPTIONS} ${DEBUG} ${SOURCES} ${LIBS} -o ${OUTPUT}
