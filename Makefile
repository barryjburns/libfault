CFLAGS :=
CPPFLAGS :=
LDFLAGS := -rdynamic
LDLIBS := -ldl -ldisasm

fault:	fault.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

%.o:	%.s
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $<

%.s:	%.c
	$(CC) -S -o $@ $(CPPFLAGS) $(CFLAGS) $<
