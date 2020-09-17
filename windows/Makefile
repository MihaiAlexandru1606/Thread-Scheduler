build: lib

lib: linked_list.obj priority_queue.obj so_scheduler.obj
	link /nologo /dll /out:libscheduler.dll /implib:libscheduler.lib linked_list.obj priority_queue.obj so_scheduler.obj

linked_list.obj : .\data_structure\linked_list\linked_list.c
	cl /nologo /MD /W3 /D_CRT_SECURE_NO_DEPRECATE /DDLL_EXPORTS /I ./utils /c .\data_structure\linked_list\linked_list.c

priority_queue.obj : .\data_structure\priority_queue\priority_queue.c
	cl /nologo /MD /W3 /D_CRT_SECURE_NO_DEPRECATE /DDLL_EXPORTS /I .\utils /I .\data_structure\linked_list /c .\data_structure\priority_queue\priority_queue.c

so_scheduler.obj : .\scheduler\so_scheduler.c
	cl /nologo /MD /W3 /D_CRT_SECURE_NO_DEPRECATE /DDLL_EXPORTS /I .\utils /I .\data_structure\linked_list /I .\data_structure\priority_queue /c .\scheduler\so_scheduler.c

.PHONY : clean

clean:
	del *.obj *.dll *.lib *.exe *.exp