mseq.exe : mseq.obj mmd.lib
	wlink f mseq.obj l mmd.lib
play.exe : play.obj mmd.lib
	wlink f play.obj l mmd.lib

mseq.obj : mseq.c mmd.h
	wcc386 mseq.c
play.obj : play.c mmd.h
	wcc386 play.c
mmd.lib : fm.obj mmd.obj
	wlib mmd.lib fm.obj mmd.obj
fm.obj : fm.c fm.h
	wcc386 fm.c
mmd.obj : mmd.c mmd.h
	wcc386 mmd.c /4r /s /or
