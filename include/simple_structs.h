#ifndef SIMPLE_STRUCTS_H
#define SIMPLE_STRUCTS_H

typedef struct {
	int from;
	int to;
} Fdsimple;

typedef struct {
	int from;
	const char* to;
	int append;
} Nfsimple;

#endif
