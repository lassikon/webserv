#!/bin/bash

cd "./$2"

echo "#pragma once" > $1.hpp
echo >> $1.hpp
echo "class $1 {" >> $1.hpp
echo "private:" >> $1.hpp
echo "  const float example = 42;" >> $1.hpp
echo >> $1.hpp
echo "public:" >> $1.hpp
echo "  $1(void);" >> $1.hpp
echo "  $1(const $1 &other);" >> $1.hpp
echo "  $1 &operator=(const $1 &other);" >> $1.hpp
echo "  ~$1(void);" >> $1.hpp
echo "};" >> $1.hpp
echo >> $1.hpp

echo "#include \"$1.hpp\"" > $1.cpp
echo >> $1.cpp

echo "$1::$1(void) {}" >> $1.cpp
echo "$1::$1(const $1 &other) { (void)other; }" >> $1.cpp
echo "$1 &$1::operator=(const $1 &other) { (void)other; }" >> $1.cpp
echo "$1::~$1(void) {}" >> $1.cpp
echo >> $1.cpp