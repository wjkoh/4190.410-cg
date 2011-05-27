CXX=g++-mp-4.5

hw5: hw5.cpp
	ctags *
	# Assuming GLEW was installed to /opt/local
	# -ftree-vectorize is on -O3
	$(CXX) -o hw5 hw5.cpp object.cpp triangle.cpp polyhedron.cpp \
		-std=c++0x \
		-O3 -pipe -march=native \
		-ffast-math -ftree-vectorize -ftree-vectorizer-verbose=1 \
		-I. -I/opt/local/include -L/opt/local/lib \
		-framework OpenGL -framework GLUT #-lGLEW
	nice -n -10 ./hw5

win:
	cl /Fohello-gl-dummy.obj /c hello-gl-dummy.c
	link /out:hello-gl-dummy.exe hello-gl-dummy.obj \
		opengl32.lib glut32.lib glew32.lib

.PHONY: hw5

