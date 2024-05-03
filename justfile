name := "main"

watch := "find | entr -c zsh -i -c"

run:
	{{watch}} 'gcc {{name}}.c -o {{name}} -Wall -Wpedantic && cat example.xml | ./{{name}}'
