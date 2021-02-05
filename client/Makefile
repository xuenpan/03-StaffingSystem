OBJS:=cli
FLAGS:=main.o ./src/client.o
CFLAGS:=-I/home/xuen/github/03-StaffingSystem/client/include

$(OBJS):$(FLAGS)
	$(CC) $^ -o $@ -lpthread -lsqlite3

.PHONY:clean

clean:
	rm $(OBJS) $(FLAGS)
