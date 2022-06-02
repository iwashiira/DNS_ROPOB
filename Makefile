CFLAGS=-std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

dns_ropob:	$(OBJS)
		$(CC) -o dns_ropob_generator $(OBJS) $(LDFLAGS)

$(OBJS): dns_ropob.h

test:	dns_ropob_generator
		./test.sh

clean:	
	rm -f dns_ropob_generator *.o *~ tmp* ./input/dns_ropob ./input/dns_ropob*.s ./input/dns_ropob.obj

.PHONY: test clean
