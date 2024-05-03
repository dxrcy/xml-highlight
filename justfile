name := "main"

watch := "find | entr -dc zsh -i -c"

run:
	{{watch}} 'gcc {{name}}.c -o {{name}} -Wall -Wpedantic && cat example.xml | ./{{name}}'
