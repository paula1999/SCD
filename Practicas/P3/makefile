.SUFFIXES:
.PHONY:    pc,pc2,pcm,pcmf,pcml,pcmf2,pcml2,fp,f,fi,fc,clean

compilador := mpicxx -std=c++11
flagsc     := -g -Wall

pc: prodcons_exe
	mpirun -np  3 ./$<

pc2: prodcons2_exe
	mpirun -np  3 ./$<

pcm: prodcons2_mu_exe
	mpirun -np  10 ./$<

pcme: examenP3-prodcons_exe
	mpirun -np 10 ./$<

pcmf: prodcons2_mu_FIFO_exe
	mpirun -np  10 ./$<

pcmf2: prodcons2_mu_par-impar_FIFO_exe
	mpirun -np  6 ./$<

pcml: prodcons2_mu_LIFO_exe
	mpirun -np  10 ./$<

pcml2: prodcons2_mu_par-impar_LIFO_exe
	mpirun -np  6 ./$<

fp: filosofos-plantilla_exe
	mpirun -np 10 ./$<

fi: filosofos-interb_exe
	mpirun -np 10 ./$<

f: filosofos_exe
	mpirun -np 10 ./$<

fc: filosofos-cam_exe
	mpirun -np 11 ./$<

%_exe: %.cpp
	$(compilador) $(flagsc) -o $@ $<

clean:
	rm -rf *_exe *.dSYM
