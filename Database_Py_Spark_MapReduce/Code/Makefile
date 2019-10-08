ifeq ($(shell uname), Darwin)
    OPEN=open
else
    OPEN=xdg-open
endif
pdf-open: open-pdf
open-pdf: pdf
	$(OPEN) report.pdf

pdf: report.pdf

report.pdf: report.tex analysis.py
	pdflatex -halt-on-error -shell-escape report.tex
clean: 
	rm -rf report.pdf report.aux report.log || exit 0
