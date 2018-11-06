all: editor

commit: editor git-commit

editor: editor.cc
	g++ -o editor editor.cc

.PHONY: git-commit
git-commit:
	git add .
	git commit
	git push
