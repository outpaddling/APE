all:    ape apekeys apeascii ape.html apekeys.html apeascii.html

ape:    ape.1
	nroff -man ape.1 > ape
	
ape.html:   ape.1
	man_to_html ape.1 > ape.html

apekeys:        apekeys.1
	nroff -man apekeys.1 > apekeys

apekeys.html:   apekeys.1
	man_to_html apekeys.1 > apekeys.html

apeascii:       apeascii.1
	nroff -man apeascii.1 > apeascii

apeascii.html:  apeascii.1
	man_to_html apeascii.1 > apeascii.html

clean:
	rm -f *.nr *.html

