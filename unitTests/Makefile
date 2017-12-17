TESTS=$(shell find . -maxdepth 1  -type d -name "Test*" | sort -V)
.PHONY:	${TESTS}  all

all clean: ${TESTS}

${TESTS}:
	@${MAKE} -C $@ $(MAKECMDGOALS)

