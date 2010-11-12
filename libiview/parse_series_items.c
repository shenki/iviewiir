#include <neon/ne_xml.h>
#include <stdio.h>
#include <assert.h>
#include "iview.h"

#define _GNU_SOURCE
#include <string.h>
#undef _GNU_SOURCE

struct iv_item_list {
    size_t len;
    struct iv_item *head;
};

enum item_parse_state {
    PS_RSS = 1,
    PS_CHANNEL,
    PS_ITEM,
    PS_ID,
    PS_TITLE,
    PS_URL,
    PS_DESCRIPTION,
    PS_THUMBNAIL,
    PS_DATE,
    PS_RATING,
    PS_LINK,
    PS_HOME
};

static int accept_start_rss(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("rss", name)) {
        return NE_XML_DECLINE;
    }
    return PS_RSS;
}

static int accept_start_channel(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("channel", name)) {
        return NE_XML_DECLINE;
    }
    return PS_CHANNEL;
}

static int accept_start_item(void *userdata, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("item", name)) {
        return NE_XML_DECLINE;
    }
    struct iv_item_list *item_list = (struct iv_item_list *)userdata;
    if(!(item_list->head = realloc(item_list->head,
                    ++(item_list->len)*sizeof(struct iv_item)))) {
        perror("realloc");
        return -1;
    }
    memset(&(item_list->head[item_list->len-1]), 0, sizeof(struct iv_item));
    return PS_ITEM;
}

static int accept_start_id(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace, const char *name, const char **attrs IV_UNUSED) {
    if(0 != strcmp("http://www.abc.net.au/tv/mrss", nspace)) {
        return NE_XML_DECLINE;
    }
    if(0 != strcmp("id", name)) {
        return NE_XML_DECLINE;
    }
    return PS_ID;
}

static int accept_cdata_id(void *userdata, int state IV_UNUSED,
        const char *cdata, size_t len IV_UNUSED) {
    struct iv_item_list *item_list = (struct iv_item_list *)userdata;
    /* possible portability issues with strndup */
    (item_list->head[item_list->len-1]).id = atoi(cdata);
    return 0;
}

static int accept_start_title(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("title", name)) {
        return NE_XML_DECLINE;
    }
    return PS_TITLE;
}

static int accept_cdata_title(void *userdata, int state IV_UNUSED,
        const char *cdata, size_t len) {
    struct iv_item_list *item_list = (struct iv_item_list *)userdata;
    /* possible portability issues with strndup */
    (item_list->head[item_list->len-1]).title = strndup(cdata, len);
    return 0;
}

static int accept_start_url(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("videoAsset", name)) {
        return NE_XML_DECLINE;
    }
    return PS_URL;
}

static int accept_cdata_url(void *userdata, int state IV_UNUSED,
        const char *cdata, size_t len) {
    struct iv_item_list *item_list = (struct iv_item_list *)userdata;
    /* possible portability issues with strndup */
    (item_list->head[item_list->len-1]).url = strndup(cdata, len);
    return 0;
}

static int accept_start_description(void *userdata IV_UNUSED,
        int parent IV_UNUSED, const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("description", name)) {
        return NE_XML_DECLINE;
    }
    return PS_DESCRIPTION;
}

static int accept_cdata_description(void *userdata, int state IV_UNUSED,
        const char *cdata, size_t len) {
    struct iv_item_list *item_list = (struct iv_item_list *)userdata;
    /* possible portability issues with strndup */
    (item_list->head[item_list->len-1]).description = strndup(cdata, len);
    return 0;
}

static int accept_start_thumbnail(void *userdata, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name, const char **attrs) {
    if(0 != strcmp("thumbnail", name)) {
        return NE_XML_DECLINE;
    }
    struct iv_item_list *item_list = (struct iv_item_list *)userdata;
    /* possible portability issues with strndup */
    (item_list->head[item_list->len-1]).thumbnail = strdup(attrs[1]);
    return PS_THUMBNAIL;
}

static int accept_start_date(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("transmitDate", name)) {
        return NE_XML_DECLINE;
    }
    return PS_DATE;
}

static int accept_cdata_date(void *userdata, int state IV_UNUSED,
        const char *cdata, size_t len) {
    struct iv_item_list *item_list = (struct iv_item_list *)userdata;
    /* possible portability issues with strndup */
    (item_list->head[item_list->len-1]).date = strndup(cdata, len);
    return 0;
}

static int accept_start_rating(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("rating", name)) {
        return NE_XML_DECLINE;
    }
    return PS_RATING;
}

static int accept_cdata_rating(void *userdata, int state IV_UNUSED,
        const char *cdata, size_t len) {
    struct iv_item_list *item_list = (struct iv_item_list *)userdata;
    /* possible portability issues with strndup */
    (item_list->head[item_list->len-1]).rating = strndup(cdata, len);
    return 0;
}

static int accept_start_link(void *userdata, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name, const char **attrs) {
    if(0 != strcmp("player", name)) {
        return NE_XML_DECLINE;
    }
    struct iv_item_list *item_list = (struct iv_item_list *)userdata;
    /* possible portability issues with strndup */
    (item_list->head[item_list->len-1]).link = strdup(attrs[1]);
    return PS_LINK;
}

static int accept_start_home(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("linkURL", name)) {
        return NE_XML_DECLINE;
    }
    return PS_HOME;
}

static int accept_cdata_home(void *userdata, int state IV_UNUSED,
        const char *cdata, size_t len) {
    struct iv_item_list *item_list = (struct iv_item_list *)userdata;
    /* possible portability issues with strndup */
    (item_list->head[item_list->len-1]).home = strndup(cdata, len);
    return 0;
}

ssize_t iv_parse_series_items(char *buf, size_t len, struct iv_item **items) {
    struct iv_item_list item_list = { .len = 0, .head = NULL };
    if(!(item_list.head = malloc(++(item_list.len)*sizeof(struct iv_item)))) {
        perror("malloc");
        return -1;
    }
    memset(&(item_list.head[0]), 0, sizeof(struct iv_item));
    ne_xml_parser *item_parser = ne_xml_create();
    ne_xml_push_handler(item_parser, accept_start_rss,
            NULL, NULL, NULL);
    ne_xml_push_handler(item_parser, accept_start_channel,
            NULL, NULL, NULL);
    ne_xml_push_handler(item_parser, accept_start_item,
            NULL, NULL, (void *)&item_list);
    ne_xml_push_handler(item_parser, accept_start_id,
            accept_cdata_id, NULL, (void *)&item_list);
    ne_xml_push_handler(item_parser, accept_start_title,
            accept_cdata_title, NULL, (void *)&item_list);
    ne_xml_push_handler(item_parser, accept_start_url,
            accept_cdata_url, NULL, (void *)&item_list);
    ne_xml_push_handler(item_parser, accept_start_description,
            accept_cdata_description, NULL, (void *)&item_list);
    ne_xml_push_handler(item_parser, accept_start_thumbnail,
            NULL, NULL, (void *)&item_list);
    ne_xml_push_handler(item_parser, accept_start_date,
            accept_cdata_date, NULL, (void *)&item_list);
    ne_xml_push_handler(item_parser, accept_start_rating,
            accept_cdata_rating, NULL, (void *)&item_list);
    ne_xml_push_handler(item_parser, accept_start_link,
            NULL, NULL, (void *)&item_list);
    ne_xml_push_handler(item_parser, accept_start_home,
            accept_cdata_home, NULL, (void *)&item_list);
    int result = ne_xml_parse(item_parser, buf, len);
    ne_xml_parse(item_parser, buf, 0);
    ne_xml_destroy(item_parser);
    if(0 != result) {
        return -IV_ESAXPARSE;
    }
    *items = item_list.head;
    return item_list.len;
}
