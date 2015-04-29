PHONY: lpm lpm regex .FORCE

all: lpm regex
	ar -M < generate.a

lpm: .FORCE
	make -C lpm/ all

pm: .FORCE
	make -C pm/ all

regex: .FORCE
	make -C regex/ all


clean:
	rm fastnet.a
	make -C lpm/ clean
	make -C pm/ clean
	make -C regex/ clean

.FORCE:
