
all:lib

lib: .FORCE
	make -C src/ all

test: .FORCE
	make -C test/

bench: .FORCE
	make -C bench/

pack: .FORCE clean
	zip xvokra00.zip -r *

doc: .FORCE
	doxygen doc/config

clean: .FORCE
	find ./doc/* | grep -v "config" | xargs rm -rf # remove generated documentation
	make -C bench/ clean
	make -C src/ clean
	make -C test/ clean
	rm -rf *.zip *.tar *tar.gz 2 > /dev/null

.FORCE:
