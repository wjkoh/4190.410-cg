hw4: hw4.cpp
	# Assuming GLEW was installed to /opt/local
	g++ -o hw4 hw4.cpp \
		-I. \
		-I/opt/local/include -L/opt/local/lib \
		-framework OpenGL -framework GLUT -lGLEW
	./hw4

win:
	cl /Fohello-gl-dummy.obj /c hello-gl-dummy.c
	link /out:hello-gl-dummy.exe hello-gl-dummy.obj \
		opengl32.lib glut32.lib glew32.lib
