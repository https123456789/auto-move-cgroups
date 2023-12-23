BUILD_DIR ?= build
SRC := main.c config.c netlink.c signals.c cgroup.c place.c
OFILES := $(SRC:.c=.o)
OFILES_PREFIXED := $(addprefix $(BUILD_DIR)/src/,$(SRC:.c=.o))
CFLAGS := -Wall -Werror -lcgroup

ifeq ($(PREFIX),)
    PREFIX := /usr/bin
endif

all: $(BUILD_DIR)/auto-move-cgroups
	chown root $<
	chmod u+s $<

.PHONY: run
run: all
	./$(BUILD_DIR)/auto-move-cgroups

.PHONY: install
install: all
	install -m 4755 -o root -g root $(BUILD_DIR)/auto-move-cgroups $(PREFIX)/auto-move-cgroups

$(BUILD_DIR)/auto-move-cgroups: $(OFILES_PREFIXED)
	gcc -o $@ $(CFLAGS) $^

$(BUILD_DIR)/src/%.o: src/%.c
	mkdir -p $(dir $@)
	gcc -o $@ $(CFLAGS) -c $<
