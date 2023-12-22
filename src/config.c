#include <stdlib.h>
#include "config.h"

struct config generate_config(void) {
    struct config c;
    struct config_group_def *browser = malloc(sizeof(struct config_group_def));

    // Setup the browser group to select processes who's name matches 'firefox-developer-edition'
    browser->name = "browsers";
    browser->target = "firefox-developer-edition";
    browser->next = NULL;

    c.groups = browser;
    c.group_count = 1;

    return c;
}
