CFLAGS=-ggdb3 -Wall -O0 -I..
LFLAGS= -lm
BINS=tree-search tree-search2 tree-search3 \
	tree-create tree-create2 tree-create-failures tree-create-failures2 \
	tree-delete tree-delete2 tree-delete3 tree-delete4 tree-delete5 \
	tree-delete6 \
	set1 set2 set3

all: ${BINS}

clean:
	rm ${BINS}

tree-create: tree-create.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-create tree-create.c ../btree.c ../set.c ${LFLAGS}

tree-create-failures: tree-create-failures.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-create-failures tree-create-failures.c ../btree.c ../set.c ${LFLAGS}

tree-create-failures2: tree-create-failures2.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-create-failures2 tree-create-failures2.c ../btree.c ../set.c ${LFLAGS}

tree-create2: tree-create2.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-create2 tree-create2.c ../btree.c ../set.c ${LFLAGS}

tree-search: tree-search.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-search tree-search.c ../btree.c ../set.c ${LFLAGS}

tree-search2: tree-search2.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-search2 tree-search2.c ../btree.c ../set.c ${LFLAGS}

tree-search3: tree-search3.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-search3 tree-search3.c ../btree.c ../set.c ${LFLAGS}

tree-delete: tree-delete.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-delete tree-delete.c ../btree.c ../set.c ${LFLAGS}

tree-delete2: tree-delete2.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-delete2 tree-delete2.c ../btree.c ../set.c ${LFLAGS}

tree-delete3: tree-delete3.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-delete3 tree-delete3.c ../btree.c ../set.c ${LFLAGS}

tree-delete4: tree-delete4.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-delete4 tree-delete4.c ../btree.c ../set.c ${LFLAGS}

tree-delete5: tree-delete5.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-delete5 tree-delete5.c ../btree.c ../set.c ${LFLAGS}

tree-delete6: tree-delete6.c ../btree.c ../btree.h
	gcc ${CFLAGS} -o tree-delete6 tree-delete6.c ../btree.c ../set.c ${LFLAGS}

set1: set1.c ../set.c ../set.h
	gcc ${CFLAGS} -o set1 set1.c ../set.c ${LFLAGS}

set2: set2.c ../set.c ../set.h
	gcc ${CFLAGS} -o set2 set2.c ../set.c ${LFLAGS}

set3: set3.c ../set.c ../set.h
	gcc ${CFLAGS} -o set3 set3.c ../set.c ${LFLAGS}
