
TARGET = yawn
CFLAGS = -W -Wall -O2
LDFLAGS = -s
OBJS = yawn.o

all: $(TARGET)
$(TARGET): $(OBJS)

clean:
	-rm -f $(OBJS) $(TARGET)
