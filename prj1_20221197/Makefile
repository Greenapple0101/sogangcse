# 컴파일러와 플래그 설정
CC=gcc
CFLAGS=-Wall



SRCS=bitmap.c debug.c hash.c hex_dump.c list.c main.c
OBJS=$(SRCS:.c=.o)

# 최종 타겟 실행 파일 이름
TARGET=testlib

# 기본 타겟
all: $(TARGET)

# 메인 타겟 빌드 규칙
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# 오브젝트 파일을 .c 파일로부터 컴파일
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# .c 파일에 대한 의존성 명시, 필요한 헤더 파일 포함
bitmap.o: bitmap.c bitmap.h limits.h
debug.o: debug.c debug.h limits.h
hash.o: hash.c hash.h limits.h
hex_dump.o: hex_dump.c hex_dump.h limits.h
list.o: list.c list.h limits.h
main.o: main.c bitmap.h debug.h hash.h hex_dump.h list.h round.h limits.h

# 'make clean'을 위한 규칙, 빌드 과정에서 생성된 파일 정리
clean:
	rm -f $(TARGET) $(OBJS)

# 가상 타겟 설정
.PHONY: all clean runscript
