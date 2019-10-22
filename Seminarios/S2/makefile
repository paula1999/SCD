.SUFFIXES:
.PHONY: x1, x2, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors
hmonsrcs:= HoareMonitor.h HoareMonitor.cpp Semaphore.h Semaphore.cpp

prodcons2_su_exe: prodcons2_su.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp Semaphore.cpp
prodconsVariosFIFO_sc_exe: prodconsVariosFIFO_sc.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<
prodconsVariosLIFO_sc_exe: prodconsVariosFIFO_sc.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<
prodconsVariosFIFO_su_exe: prodconsVariosFIFO_su.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp
prodconsVariosLIFO_su_exe: prodconsVariosLIFO_su.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp

clean:
	rm -f *_exe
