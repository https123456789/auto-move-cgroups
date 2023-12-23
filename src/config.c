#include <regex.h>
#include <stdlib.h>
#include "config.h"

struct config generate_config(void) {
    struct config c;
    struct config_group_def *browser = malloc(sizeof(struct config_group_def));

    browser->name = "browser";
    browser->target = "(\\/usr\\/lib\\/firefox.*\\/firefox)|(\\/opt\\/google\\/chrome.*\\/chrome)";
    browser->next = NULL;
    if (regcomp(&browser->target_exp, browser->target, REG_EXTENDED)) {
        perror("regcomp");
        exit(1);
    }

    c.groups = browser;

    return c;
}
