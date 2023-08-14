#!/bin/bash

NTHREADS=18
CXX="g++"
CXXFLAGS="-gdwarf-2 -Wall -std=c++20"
LD="g++"
LDFLAGS="`pkg-config --libs glfw3 glew zlib` -lopengl32 -lniflib"

cd src
DIRS=($(find ./ -type d | sed -e 's/^\.\///'))
cd ..

mkdir -p out

for dir in "${DIRS[@]}"
do
	mkdir -p out/$dir
done

SOURCES=($(find src | grep '.*/.*\.\(c\|cpp\|cc\|c++\)$'))
OBJECTS=()

for src in "${SOURCES[@]}"
do
	obj=$src
	obj="$(sed 's@^[^/]*/@@' <<< "$obj")"
	obj="out/${obj}.o"
	OBJECTS+=("$obj")
done

function build {
	obj=$1
	obj="$(sed 's@^[^/]*/@@' <<< "$obj")"
	obj="out/${obj}.o"
	echo "compiling $src -> $obj";
	${CXX} ${CXXFLAGS} -o $obj -c $1
}

for src in "${SOURCES[@]}"
do
	((i=i%NTHREADS)); ((i++==0)) && wait
	build $src &
done

wait

ob="$(printf "%s " "${OBJECTS[@]}")"

echo "linking   $ob -> ./nv"
${LD} ${CXXFLAGS} -o ./nv $ob ${LDFLAGS}
