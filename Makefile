BUILD_DIR ?= build
SRC := main.c config.c netlink.c signals.c cgroup.c
OFILES := $(SRC:.c=.o)
OFILES_PREFIXED := $(addprefix $(BUILD_DIR)/src/,$(SRC:.c=.o))
CFLAGS := -Wall -Werror -lcgroup

all: $(BUILD_DIR)/auto-move-cgroups
	sudo chown root $<
	sudo chmod u+s $<

.PHONY: run
run: all
	./$(BUILD_DIR)/auto-move-cgroups

$(BUILD_DIR)/auto-move-cgroups: $(OFILES_PREFIXED)
	gcc -o $@ $(CFLAGS) $^

$(BUILD_DIR)/src/%.o: src/%.c
	mkdir -p $(dir $@)
	gcc -o $@ $(CFLAGS) -c $<
