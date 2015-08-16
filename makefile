decafc: decafc.o decafs.o lex.yy.o y.tab.o semantics.o parsetree.o
	g++ -o decafc decafc.o decafs.o lex.yy.o y.tab.o semantics.o parsetree.o

decafc.o: decafc.cc decafs.cc semantics.h parsetree.h
	g++ -c decafc.cc


decafs.o: decafs.cc semantics.h parsetree.h
	g++ -c decafs.cc

semantics.o: semantics.h semantics.cc parsetree.h
	g++ -c semantics.cc

parsetree.o: parsetree.h parsetree.cc
	g++ -c parsetree.cc

y.tab.o: y.tab.c
	g++ -c y.tab.c

lex.yy.o: lex.yy.c
	g++ -c lex.yy.c

y.tab.h: decaf.y
	yacc -d -v decaf.y

y.tab.c: decaf.y
	yacc -d -v decaf.y

lex.yy.c: decaf.l y.tab.h
	flex decaf.l

clean:
	rm -rf *~ lex.yy.* *.o y.tab.* decafp decafc a.out y.output
