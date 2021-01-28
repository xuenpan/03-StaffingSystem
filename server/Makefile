OBJS:=ser
FLAGS:=main.o ./src/server.o
CFLAGS:=-I/home/xuen/github/03-StaffingSystem/server/include/

$(OBJS):$(FLAGS)
	$(CC) $^ -o $@ -lpthread -lsqlite3 

.PHONY:clean

clean:
	rm $(OBJS) $(FLAGS)
