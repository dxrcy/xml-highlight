run:
	runs main.c gcc %f -o %n -Wall '&&' cat example.xml '|' ./%n
