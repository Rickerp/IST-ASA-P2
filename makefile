CC 		= gcc
CFLAGS 	= -Wall -pedantic -g
FILE 	= main.c
OUT		= $(FILE:%.c=%.run)
all:
	${CC} ${CFLAGS} ${FILE} -o ${OUT}
