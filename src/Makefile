

pack: .FORCE clean
	zip xvokra00.zip -r *

doc: .FORCE
	doxygen doc/config

clean:
	find ./doc/* | grep -v "config" | xargs rm -rf # remove generated documentation
	make -C bench clean
	make -C lib clean
	make -C test clean
	rm -rf *.zip *.tar *tar.gz 2 > /dev/null

.FORCE:
