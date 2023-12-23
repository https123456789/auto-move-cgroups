#include <stdlib.h>
#include "config.h"

struct config generate_config(void) {
    struct config c;
    struct config_group_def *browser = malloc(sizeof(struct config_group_def));
    struct config_group_def *zsh = malloc(sizeof(struct config_group_def));

    // Setup the browser group to select processes who's name matches 'firefox-developer-edition'
    browser->name = "browsers";
    browser->target = "/usr/lib/firefox-developer-edition/firefox";
    browser->next = zsh;

    zsh->name = "shells";
    zsh->target = "/usr/bin/zsh";
    zsh->next = NULL;

    c.groups = browser;

    return c;
}
