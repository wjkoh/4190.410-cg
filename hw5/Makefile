CXX=ccache g++-mp-4.5
OBJS = hw5.o object.o triangle.o polyhedron.o
CXXFLAGS = -std=c++0x \
		-O3 -pipe -march=native \
		-ffast-math -ftree-vectorize -ftree-vectorizer-verbose=1 \
		-framework OpenGL -framework GLUT #-lGLEW
INC=. /opt/local/include
LIB=/opt/local/lib

#hw5: $(OBJS)
#	$(CXX) $(CXXFLAGS) $(OBJS) -o $@
#
##.SUFFIXES: .cpp .o
#
#%.o: %.cpp
#	$(CXX) $(CXXFLAGS) -I$(INC) -L$(LIB) -c $< -o $@

hw5: hw5.cpp
	ctags *
	# Assuming GLEW was installed to /opt/local
	# -ftree-vectorize is on -O3
	$(CXX) -o hw5 hw5.cpp ray_tracer.cpp object.cpp triangle.cpp polyhedron.cpp \
		-std=c++0x \
		-O3 -pipe -march=native \
		-ffast-math -ftree-vectorize -ftree-vectorizer-verbose=1 \
		-I. -I/opt/local/include -L/opt/local/lib \
		-framework OpenGL -framework GLUT #-lGLEW
	open -a Terminal.app ./hw5

win:
	cl /Fohello-gl-dummy.obj /c hello-gl-dummy.c
	link /out:hello-gl-dummy.exe hello-gl-dummy.obj \
		opengl32.lib glut32.lib glew32.lib

.PHONY: hw5

