gccopts=-Wno-format-extra-args
incfiles=$(addprefix src/,common.c files.c label.c asm.c parse.c vars.c expr.c cond.c stmnt.c dclrtn.c include.c)
mainfile=src/c02.c
outfile=c02
c02: ${incfiles} ${mainfile} 
	gcc ${gccopts} ${incfiles} ${mainfile} -o ${outfile}
clean:
	rm ${outfile}
