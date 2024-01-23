COMP=clang
OPTIONS=-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-but-set-variable -O3
DEBUG=-fsanitize=address,undefined -g3

SOURCES=src/*
OUTPUT=amp
LIBS=-lmpv -lglfw -lGL -ldrawtext
INCLUDE=-Iinclude/

final:
	${COMP} ${OPTIONS} ${SOURCES} ${LIBS} ${INCLUDE} -o ${OUTPUT}

debug:
	${COMP} ${OPTIONS} ${DEBUG} ${SOURCES} ${LIBS} ${INCLUDE} -o ${OUTPUT}
