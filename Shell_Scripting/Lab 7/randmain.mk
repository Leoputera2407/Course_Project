# Make x86-64 random byte generators.


# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

# Change the -O2 to -Og or -O0 to ease runtime debugging.
# -O2 is often better for compile-time diagnostics, though.
OPTIMIZE = -Og # was O2

CC = gcc
LFLAGS= -ldl -Wl,-rpath=$(PWD)
CFLAGS = $(OPTIMIZE) -g3 -Wall -Wextra -march=native -mtune=native -mrdrnd

# randmain.mk contains instructions for building
# randmain, randlibhw.so, and randlibsw.so.

default: randmain randlibhw.so randlibsw.so

randmain: randmain.o randcpuid.o
	$(CC) $(CFLAGS) $(LFLAGS) randcpuid.o randmain.o -o randmain
randmain.o: randmain.c
	$(CC) $(CFLAGS)  -c randmain.c -o randmain.o
randcpuid.o: randcpuid.c randcpuid.h
	$(CC) $(CFLAGS)  -c randcpuid.c -o randcpuid.o


randlibhw.so: randlibhw.c
	$(CC) $(CFLAGS) -shared -fPIC randlibhw.c -o randlibhw.so

randlibsw.so: randlibsw.c
	$(CC) $(CFLAGS) -shared -fPIC randlibsw.c -o randlibsw.so

