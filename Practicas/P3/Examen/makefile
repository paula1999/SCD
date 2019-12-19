.SUFFIXES:
.PHONY:    pc,pc2,pcm,pcmf,pcml,pcmf2,pcml2,fp,f,fi,fc,clean

compilador := mpicxx -std=c++11
flagsc     := -g -Wall


fi: filosofos-interb_exe
	mpirun -np 10 ./$<

f: filosofos_exe
	mpirun -np 10 ./$<

fc: camarero4_exe
	mpirun -np 11 ./$<


%_exe: %.cpp
	$(compilador) $(flagsc) -o $@ $<

clean:
	rm -rf *_exe *.dSYM
