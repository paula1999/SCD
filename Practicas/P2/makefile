.SUFFIXES:
.PHONY: x1, x2, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors
hmonsrcs:= HoareMonitor.h HoareMonitor.cpp Semaphore.h Semaphore.cpp

fumadoresSU_exe: fumadoresSU.cpp 
	$(compilador) $(opcionesc) $(hmonsrcs) -o $@ $<
barbero_exe: Barbero.cpp
	$(compilador) $(opcionesc) $(hmonsrcs) -o $@ $<
barberia2_exe: barberia2.cpp
	$(compilador) $(opcionesc) $(hmonsrcs) -o $@ $<

clean:
	rm -f *_exe
