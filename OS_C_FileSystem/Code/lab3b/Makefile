#NAME: Hanif Leoputera Lim
#EMAIL: hanifleoputeralim@gmail.com
#ID: 504971751


default: build
build:
	rm -f lab3b
	ln lab3b_compiler lab3b
	chmod +x lab3b
dist: default
	tar -cvzf lab3b-504971751.tar.gz lab3b.py lab3b_compiler Makefile README
test:
	chmod +x test.sh
	./test.sh 504971751
clean:
	rm -f lab3b lab3b-504971751.tar.gz