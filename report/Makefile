# Makefile
# File: "/home/pilla/Artigos/erad2016sheiny/Makefile"
# Created: "Sex, 07 Jan 2005 14:44:03 -0200 (kassick)"
# Updated: "Sex, 06 Nov 2015 17:24:39 -0200 (pilla)"
# $Id: Makefile,v 1.3 2006/12/19 00:47:55 rvkassick Exp $
# Copyright (C) 2005, Rodrigo Virote Kassick <rvkassick@inf.ufrgs.br> 

LATEX = pdflatex -halt-on-error -file-line-error
BIBTEX = bibtex
MAINFILE = sbc-template


#Listas de targets pdfs que devem ser gerados apartir dos fontes em outros formatos

rawimgs := $(wildcard images/*)

svgimgs := $(patsubst %.svg,%.pdf,$(wildcard svg/*.svg))

epsimgs := $(patsubst %.eps,%.pdf,$(wildcard eps/*.eps))

figimgs := $(patsubst %.fig,%.pdf,$(wildcard fig/*.fig))

bibs    := $(wildcard *.bib)

all: $(MAINFILE).pdf

#all: press

view: $(MAINFILE).pdf
	evince $(MAINFILE).pdf

svg/%.pdf: svg/%.svg
	inkscape -z -f $< --export-area-snap  -E /dev/stdout|epstopdf --filter --outfile=$@

#java -jar batik/batik-rasterizer.jar -m application/pdf -d svg $<

eps/%.pdf: eps/%.eps
	epstopdf --outfile=$@ $<

fig/%.pdf: fig/%.fig
	fig2dev -L eps $<|epstopdf --filter --outfile=$@

$(MAINFILE).pdf: *.tex sections/*.tex $(bibs) $(rawimgs) $(svgimgs) $(epsimgs) $(figimgs)
	$(LATEX) $(MAINFILE)
	$(BIBTEX) $(MAINFILE) || echo BIBTEX SUCKS
	$(LATEX) $(MAINFILE)
	$(BIBTEX) $(MAINFILE) || echo BIBTEX SUCKS
	$(LATEX) $(MAINFILE)
	$(LATEX) $(MAINFILE)



*.ps: *.pdf
	pdf2ps -dLanguageLevel=2 $(MAINFILE).pdf -o $(MAINFILE).ps




.PHONY: clean
clean:
	rm -f $(MAINFILE).pdf *.ps svg/*.pdf eps/*.pdf fig/*.pdf *.bbl *.aux *.blg *.log *.lot *.lof || echo "Clean"

