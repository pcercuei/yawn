
TARGET = yawk
CFLAGS = -W -Wall -O2
LDFLAGS = -s
OBJS = yawk.o

all: $(TARGET)
$(TARGET): $(OBJS)

clean:
	-rm -f $(OBJS) yawk
