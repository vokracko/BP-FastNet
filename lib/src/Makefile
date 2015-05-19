REGEX=dfa
LPM=tbm
STRIDE=5

PHONY: lpm lpm regex .FORCE

all: lpm regex pm
	ar -M < generate.a

lpm: .FORCE
	make -C lpm/ all ALG=$(LPM) STRIDE=$(STRIDE)

pm: .FORCE
	make -C pm/ all

regex: .FORCE
	make -C regex/ all ALG=$(REGEX)


clean:
	rm -f fastnet.a 2>/dev/null
	make -C lpm/ clean
	make -C pm/ clean
	make -C regex/ clean
	make -C common/ clean

.FORCE:
