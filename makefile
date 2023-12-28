COMP=clang
OPTIONS=-Wall -Wextra -Werror -Wno-unused-parameter -O3
DEBUG=-fsanitize=address,undefined -g3

SOURCES=main.c
OUTPUT=amp
LIBS=-lmpv

final:
	${COMP} ${OPTIONS} ${LIBS} ${SOURCES} -o ${OUTPUT}

debug:
	${COMP} ${OPTIONS} ${LIBS} ${DEBUG} ${SOURCES} -o ${OUTPUT}
