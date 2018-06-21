#Undefine this for release.
!define RELEASE

SRCDIR = SRC
OBJDIR = OBJ

.c.obj: .AUTODEPEND
!ifdef RELEASE 
        wcc386 $[@ /6r /s /orb /d1 /fo=$(OBJDIR)\$^&
!else
        wcc386 $[@ /6r /s /orb /d2 /fo=$(OBJDIR)\$^&
!endif

.obj : $(OBJDIR)
.c : $(SRCDIR)
.h : $(SRCDIR)

SNOBJS = &
        sn.obj &
        graphics.obj &
        debug.obj &
        einfo.obj &
        init.obj &
        controls.obj &
        config.obj &
        demo.obj &
        map2d.obj &
        levels.obj &
        mirrors.obj &
        movers.obj &
        player.obj &
        maps.obj &
	epiend.obj &
        stbar.obj &
        triggers.obj &
        saveg.obj &
        menus.obj &
        dat.obj &
        sound.obj &
        sfx.obj &
        kbd.obj &
        joys.obj &
        timer.obj &
        seq.obj &
        qaf.obj &
        str.obj &
        warp.obj &
        weapon.obj &
        ai.obj &
        
SHAWEDOBJS = &
        shawed.obj &
        config.obj &
        mstub.obj &
        dat.obj &

QAFEDITOBJS = &
        qafedit.obj &
        qaf.obj &
        debug.obj &
        dat.obj &

SEQEDITOBJS = &
        seqedit.obj &
        seq.obj &
        debug.obj &
        dat.obj &

STRGENOBJS = &
        strgen.obj &

all : sn.exe shawed.exe strgen.exe qafedit.exe seqedit.exe

sn.exe : $(SNOBJS) engine.lib smix.lib mmd.lib sn.lnk
        wlink @$^*
        
shawed.exe : $(SHAWEDOBJS) engine.lib shawed.lnk
        wlink @$^*

strgen.exe : $(STRGENOBJS) strgen.lnk
        wlink @$^*

qafedit.exe : $(QAFEDITOBJS) qafedit.lnk engine.lib
        wlink @$^*

seqedit.exe : $(SEQEDITOBJS) seqedit.lnk engine.lib
        wlink @$^*

sn.lnk : makefile
        %create $^@
        %append $^@ NAME $^&
        %append $^@ DEBUG ALL
        for %i in ($(SNOBJS)) do %append $^@ FILE $(OBJDIR)\%i
        %append $^@ LIB engine.lib
        %append $^@ LIB smix.lib
	%append $^@ LIB mmd.lib
        %append $^@ OPTION
        %append $^@ STACK=8K,
        %append $^@ MAP=$(OBJDIR)\$^&.map
        
shawed.lnk : makefile
        %create $^@
        %append $^@ NAME $^&
        %append $^@ DEBUG ALL
        for %i in ($(SHAWEDOBJS)) do %append $^@ FILE $(OBJDIR)\%i
        %append $^@ LIB engine.lib
        %append $^@ OPTION
        %append $^@ STACK=8K,
        %append $^@ MAP=$(OBJDIR)\$^&.map
        
strgen.lnk : makefile
        %create $^@
        %append $^@ NAME $^&
        %append $^@ DEBUG ALL
        for %i in ($(STRGENOBJS)) do %append $^@ FILE $(OBJDIR)\%i
        %append $^@ OPTION
        %append $^@ STACK=8K,
        %append $^@ MAP=$(OBJDIR)\$^&.map
                
qafedit.lnk : makefile
        %create $^@
        %append $^@ NAME $^&
        %append $^@ DEBUG ALL
        for %i in ($(QAFEDITOBJS)) do %append $^@ FILE $(OBJDIR)\%i
        %append $^@ LIB engine.lib
        %append $^@ OPTION
        %append $^@ STACK=8K,
        %append $^@ MAP=$(OBJDIR)\$^&.map
                
seqedit.lnk : makefile
        %create $^@
        %append $^@ NAME $^&
        %append $^@ DEBUG ALL
        for %i in ($(SEQEDITOBJS)) do %append $^@ FILE $(OBJDIR)\%i
        %append $^@ LIB engine.lib
        %append $^@ OPTION
        %append $^@ STACK=8K,
        %append $^@ MAP=$(OBJDIR)\$^&.map
