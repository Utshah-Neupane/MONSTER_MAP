TARGET_EXEC := monster_map

CC := gcc
LDFLAGS := -lm
CPPFLAGS :=
CFLAGS := -Wall

BUILD_DIR := ./build
SRC_DIR := ./src

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

RELEASE := $(BUILD_DIR)/$(TARGET_EXEC)

debug: CFLAGS += -g
debug: $(RELEASE)


$(RELEASE): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ -g

clean:
	rm -rf $(BUILD_DIR)
