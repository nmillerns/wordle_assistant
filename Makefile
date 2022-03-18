all:	wordle_assistant

wordle_assistant:	wordle_assistant.o
		$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@

.c.o:
		$(CC) $(CFLAGS) -c $*.c

clean:
	-rm *.o wordle_assistant
