CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -D_XOPEN_SOURCE

all : master

master : master.o helpers.o parent.o reduceworker.o mapworker.o linkedlist.o word_freq.o 
	$(CC) $(CFLAGS) -o mapreduce master.o helpers.o parent.o reduceworker.o mapworker.o linkedlist.o word_freq.o

master.o : master.c helpers.h parent.h reduceworker.h mapworker.h linkedlist.h 
	$(CC) $(CFLAGS) -c master.c

parent.o : parent.c parent.h helpers.h mapreduce.h
	$(CC) $(CFLAGS) -c parent.c

mapworker.o : mapworker.c mapworker.h mapreduce.h word_freq.o helpers.h
	$(CC) $(CFLAGS) -c mapworker.c

reduceworker.o : reduceworker.c reduceworker.h mapreduce.h linkedlist.h
	$(CC) $(CFLAGS) -c reduceworker.c

helpers.o : helpers.c helpers.h 
	$(CC) $(CFLAGS) -c helpers.c

word_freq.o : word_freq.c linkedlist.h mapreduce.h
	$(CC) $(CFLAGS) -c word_freq.c

linkedlist.o : linkedlist.c linkedlist.h mapreduce.h
	$(CC) $(CFLAGS) -c linkedlist.c

clean :
	$(RM) $(TARGET)