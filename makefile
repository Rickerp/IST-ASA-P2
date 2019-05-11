CC 		= gcc
CFLAGS 	= -O3 -ansi -Wall -pedantic -g -lm
FILE 	= main.c
OUT		= $(FILE:%.c=%.run)
all:
	${CC} ${CFLAGS} ${FILE} -o ${OUT}
