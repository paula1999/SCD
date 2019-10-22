.SUFFIXES:
.PHONY: x1, x2, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors
hmonsrcs:= HoareMonitor.h HoareMonitor.cpp Semaphore.h Semaphore.cpp

prodconsVariosFIFO_sc_exe: prodconsVariosFIFO_sc.cpp
	$(compilador) $(opcionesc)  -o $@ $<

prodconsVariosLIFO_sc_exe: prodconsVariosFIFO_sc.cpp
	$(compilador) $(opcionesc)  -o $@ $<

prodconsVariosFIFO_su_exe: prodconsVariosFIFO_su.cpp 
	$(compilador) $(opcionesc) $(hmonsrcs) -o $@ $<

prodconsVariosLIFO_su_exe: prodconsVariosLIFO_su.cpp
	$(compilador) $(opcionesc) $(hmonsrcs) -o $@ $< 

clean:
	rm -f *_exe
