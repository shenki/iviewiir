#include <assert.h>
#include <errno.h>
#include <libxml/parser.h>
#include <stdlib.h>
#include <string.h>

#include "iview.h"
#include "internal.h"

/* Example categories document
<?xml version="1.0" encoding="utf-8"?>
<categories>
        <category id="index">
                <name>A - Z</name>
                <category id="a-c" index="true">
                        <name>A - C</name>
                </category>
                <category id="d-k" index="true">
                        <name>D - K</name>

                </category>
                <category id="l-p" index="true">
                        <name>L - P</name>
                </category>
                <category id="q-z" index="true">
                        <name>Q - Z</name>
                </category>
                <category id="0-9" index="true">

                        <name>0 - 9</name>
                </category>
        </category>
        <category id="arts" genre="true">
                <name><![CDATA[Arts & Culture]]></name>
                <category id="music">
                        <name>Music</name>
                </category>

                <category id="people">
                        <name>People</name>
                </category>
                <category id="performance">
                        <name>Performance</name>
                </category>
                <category id="reviews">
                        <name>Reviews</name>

                </category>
        </category>
        <category id="docs" genre="true">
                <name>Documentary</name>
                <category id="bio">
                        <name>Biography</name>
                </category>
                <category id="history">

                        <name>History</name>
                </category>
                <category id="nature">
                        <name>Nature</name>
                </category>
                <category id="science">
                        <name>Science</name>

                </category>
        </category>
        <category id="comedy" genre="true">
                <name>Comedy</name>
                <category id="cult">
                        <name>Cult</name>
                </category>
                <category id="satire">

                        <name>Satire</name>
                </category>
                <category id="sitcom">
                        <name>Sit-com</name>
                </category>
                <category id="sketch">
                        <name>Sketch</name>

                </category>
        </category>
        <category id="drama" genre="true">
                <name>Drama</name>
                <category id="crime">
                        <name>Crime</name>
                </category>
                <category id="historical">

                        <name>Historical</name>
                </category>
                <category id="romance">
                        <name>Romance</name>
                </category>
                <category id="sci-fi">
                        <name>Sci-fi</name>

                </category>
        </category>
        <!--
        <category id="kids" genre="true">
                <name>Kids</name>
                <category id="kidsdrama">
                        <name>Drama</name>
                </category>
                <category id="kazam">
                        <name>Kazam</name>
                </category>
                <category id="lol">
                        <name>LOL</name>
                </category>
                <category id="realworld">
                        <name>Real World</name>
                </category>
                <category id="toons">
                        <name>Toons</name>
                </category>
        </category>
        -->
        <category id="pre-school" genre="true">
                <name>ABC 4 Kids</name>
        </category>
        <category id="lifestyle" genre="true">
                <name>Lifestyle</name>

                <category id="games">
                        <name>Games</name>
                </category>
                <category id="reality">
                        <name>Reality</name>
                </category>
        </category>
        <category id="news" genre="true">

                <name><![CDATA[News & Current Affairs]]></name>
        </category>
        <category id="panel" genre="true">
                <name><![CDATA[Panel & Discussion]]></name>
        </category>
        <category id="sport" genre="true">
                <name>Sport</name>
        </category>

        <category id="abc1">
                <name>ABC1</name>
        </category>
        <category id="abc2">
                <name>ABC2</name>
        </category>
        <category id="abc3">
                <name>ABC3</name>

        </category>
        <category id="abc4">
                <name>ABC News 24</name>
        </category>
        <category id="original">
                <name>iView Exclusives</name>
        </category>
        <category id="shopdownload" genre="true">

                <name>Shop Downloads</name>
                <category id="shop-arts">
                        <name>Arts</name>
                </category>
                <category id="shop-comedy">
                        <name>Comedy</name>
                </category>
                <category id="shop-docs">

                        <name>Documentary</name>
                </category>
                <category id="shop-drama">
                        <name>Drama</name>
                </category>
                <category id="shop-kids">
                        <name>Kids</name>

                </category>
                <category id="shop-lifestyle">
                        <name>Lifestyle</name>
                </category>
                <category id="shop-news">
                        <name>News</name>
                </category>
                <category id="shop-panel">

                        <name>Panel</name>
                </category>
                <category id="shop-sport">
                        <name>Sport</name>
                </category>
        </category>
        <category id="featured">
                <name>Featured Programs</name>

        </category>
        <category id="recent">
                <name>Recently Added</name>
        </category>
        <category id="last-chance">
                <name>Last Chance</name>
        </category>
</categories>
*/

enum parse_state { ps_start, ps_categories, ps_category, ps_name, ps_end };
struct category_parse_ctx {
    enum parse_state *state;
    int state_idx;
    int state_idx_max;
    int return_value;
    struct iv_category *current;
};

/* returns 0 on succes, non-zero on failure. */
static short push_state(struct category_parse_ctx *ctx, enum parse_state state) {
    // Check if we need to increase the size of our stack
    if(ctx->state_idx + 1 > ctx->state_idx_max) {
        enum parse_state *new_stack =
            realloc(ctx->state, (ctx->state_idx_max + 2) * sizeof(int));
        if(!new_stack) {
            ctx->return_value = errno;
            return 1;
        };
        ctx->state = new_stack;
        ctx->state_idx_max++;
    }
    // Push the new state
    ctx->state[++ctx->state_idx] = state;
    return 0;
}

static void pop_state(struct category_parse_ctx *ctx) {
    // Don't worry about shrinking the stack, we're going to free the whole
    // allocation after we're finished anyway.
    ctx->state_idx--;
}

static enum parse_state peek_state(struct category_parse_ctx *ctx) {
    return ctx->state[ctx->state_idx];
}

static void attrs_handler(struct category_parse_ctx *ctx,
        const xmlChar **attrs) {
    int i;
    for(i=0; attrs[i]; i+=2) {
        if(!xmlStrcmp(BAD_CAST("id"), attrs[i])) {
            ctx->current->id = (char *)xmlStrdup(attrs[i+1]);
        } else if(!xmlStrcmp(BAD_CAST("index"), attrs[i])) {
            ctx->current->mgmt->index =
                !xmlStrcmp(BAD_CAST("true"), attrs[i+1]);
        } else if(!xmlStrcmp(BAD_CAST("genre"), attrs[i])) {
            ctx->current->mgmt->genre =
                !xmlStrcmp(BAD_CAST("true"), attrs[i+1]);
        } else {
            IV_DEBUG("Unknown attribute: %s=%s\n",
                    (const char *)attrs[i], (const char *)attrs[i+1]);
        }
    }
}

// Handle parsing of the nasty n-ary tree. Luckily, in terms of the 'category'
// tag the tree is at most two deep. This assumption makes the code a bit less
// insane.
static void start_element(void *_ctx, const xmlChar *name,
        const xmlChar **attrs) {
    struct category_parse_ctx *ctx = (struct category_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) { return; }
    if(!xmlStrcmp(BAD_CAST("categories"), name)) {
        assert(peek_state(ctx) == ps_start);
        push_state(ctx, ps_categories);
    } else if(!xmlStrcmp(BAD_CAST("category"), name)) {
        const enum parse_state current_state = peek_state(ctx);
        assert(ps_categories == current_state
                || ps_category == current_state);
        struct iv_category **new;
        new = ctx->current->mgmt->sub_tail ?
            &ctx->current->mgmt->sub_tail->mgmt->next
                : &ctx->current->mgmt->sub_head;
        // We rely on calloc() below to nullify pointers in the structure
        // (assuming (NULL == 0) - there are bigger problems if this doesn't
        // hold); if this changes please use memset() to zero out the
        // structure.
        *new = calloc(1, sizeof(struct iv_category));
        if(!*new) {
            ctx->return_value = errno;
            return;
        }
        (*new)->mgmt = calloc(1, sizeof(struct iv_category_mgmt));
        if(!(*new)->mgmt) {
            ctx->return_value = errno;
            return;
        }
        if(!ctx->current->mgmt->sub_tail) {
            ctx->current->mgmt->sub_tail = ctx->current->mgmt->sub_head;
        }
        // Pivot to the new node
        (*new)->mgmt->parent = ctx->current;
        ctx->current = (*new);
        // Parse out the varying number of attributes
        attrs_handler(ctx, attrs);
        push_state(ctx, ps_category);
    } else if(!xmlStrcmp(BAD_CAST("name"), name)) {
        assert(peek_state(ctx) == ps_category);
        push_state(ctx, ps_name);
    } else {
        IV_DEBUG("Unknown element: %s\n", (const char *)name);
    }
}

static void end_element(void *_ctx, const xmlChar *name) {
    struct category_parse_ctx *ctx = (struct category_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) { return; }
    if(!xmlStrcmp(BAD_CAST("categories"), name)) {
        assert(peek_state(ctx) == ps_categories);
        pop_state(ctx);
        assert(peek_state(ctx) == ps_start);
        ctx->state[ctx->state_idx] = ps_end;
    } else if(!xmlStrcmp(BAD_CAST("category"), name)) {
        assert(peek_state(ctx) == ps_category);
        // Pivot back to the parent
        ctx->current->mgmt->parent->mgmt->sub_tail = ctx->current;
        ctx->current = ctx->current->mgmt->parent;
        pop_state(ctx);
    } else if(!xmlStrcmp(BAD_CAST("name"), name)) {
        assert(peek_state(ctx) == ps_name);
        pop_state(ctx);
    } else {
        IV_DEBUG("Closing unknown element: %s", (const char *)name);
    }
}

static void content_handler(void *_ctx, const xmlChar *data, int len) {
    struct category_parse_ctx *ctx = (struct category_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) { return; }
    // The only content we care about is in the <name /> tag
    if(ps_name == peek_state(ctx)) {
        ctx->current->name = (char *)xmlStrndup(data, len);
        if(!ctx->current->name) { ctx->return_value = errno; }
    }
}

int iv_parse_categories(const char *buf, size_t len,
        struct iv_category **categories_ptr) {
    struct iv_category *head = calloc(1, sizeof(struct iv_category));
    if(!head) { return -errno; }
    head->mgmt = calloc(1, sizeof(struct iv_category_mgmt));
    if(!head->mgmt) { return -errno; }
    // Initialise XML parser context
    xmlSAXHandlerPtr handler = calloc(1, sizeof(xmlSAXHandler));
    if(!handler) {
        int _errno = errno;
        free(head);
        return -_errno;
    }
    handler->initialized = XML_SAX2_MAGIC;
    handler->startElement = start_element;
    handler->endElement = end_element;
    handler->characters = content_handler;
    // Allocate space for the state stack
    enum parse_state *_state = calloc(1, sizeof(int));
    if(!_state) {
        int _errno = errno;
        free(handler);
        free(head);
        return -_errno;
    };
    *_state = ps_start;
    // Initialise user state
    struct category_parse_ctx ctx = {
        .state = _state,
        .state_idx = 0,
        .state_idx_max = 0,
        .return_value = IV_OK,
        .current = head,
    };
    // Parse document
    if(0 > xmlSAXUserParseMemory(handler, &ctx, buf, len)) {
        iv_destroy_categories(head);
        head = NULL;
    }
    *categories_ptr = head;
    free(ctx.state);
    free(handler);
    return -(ctx.return_value);
}

#ifdef LIBIVIEW_TEST
#include "test/CuTest.h"

// Chopped down to demonstrate that all features work, nothing more. Complete
// document is left in the comments so that if this subset is not enough we can
// easily expand it with the required data.
static const char *category_buf =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\
<categories>\
        <category id=\"arts\" genre=\"true\">\
                <name><![CDATA[Arts & Culture]]></name>\
                <category id=\"music\">\
                        <name>Music</name>\
                </category>\
\
                <category id=\"people\">\
                        <name>People</name>\
                </category>\
        </category>\
        <category id=\"docs\" index=\"true\">\
                <name>Documentary</name>\
                <category id=\"bio\">\
                        <name>Biography</name>\
                </category>\
                <category id=\"history\">\
\
                        <name>History</name>\
                </category>\
                <category id=\"nature\">\
                        <name>Nature</name>\
                </category>\
        </category>\
</categories>";

void test_iv_parse_categories(CuTest *tc) {
    struct iv_category *categories, *current;
    const int result =
        iv_parse_categories(category_buf, strlen(category_buf), &categories);
    CuAssertIntEquals(tc, IV_OK, result);
    CuAssertPtrNotNull(tc, categories);
    CuAssertPtrNotNull(tc, categories->mgmt->sub_head);

    // <category id="arts" genre="true"> ...
    current = categories->mgmt->sub_head;

    CuAssertIntEquals(tc, 1, current->mgmt->genre);
    CuAssertIntEquals(tc, 0, current->mgmt->index);
    CuAssertTrue(tc, categories == current->mgmt->parent);
    CuAssertPtrNotNull(tc, current->mgmt->sub_head);
    CuAssertPtrNotNull(tc, current->mgmt->sub_tail);
    CuAssertPtrNotNull(tc, current->mgmt->next);

    CuAssertPtrNotNull(tc, current->id);
    CuAssertStrEquals(tc, "arts", current->id);

    CuAssertPtrNotNull(tc, current->name);
    CuAssertStrEquals(tc, "Arts & Culture", current->name);

    // Depth-first testing
    CuAssertPtrNotNull(tc, current->mgmt->sub_head);

    // <category id="music"> ...
    current = current->mgmt->sub_head;

    CuAssertIntEquals(tc, 0, current->mgmt->genre);
    CuAssertIntEquals(tc, 0, current->mgmt->index);
    CuAssertPtrNotNull(tc, current->mgmt->parent);
    CuAssertTrue(tc, categories->mgmt->sub_head == current->mgmt->parent);
    CuAssertTrue(tc, NULL == current->mgmt->sub_head);
    CuAssertTrue(tc, NULL == current->mgmt->sub_tail);
    CuAssertPtrNotNull(tc, current->mgmt->next);

    CuAssertPtrNotNull(tc, current->id);
    CuAssertStrEquals(tc, "music", current->id);

    CuAssertPtrNotNull(tc, current->name);
    CuAssertStrEquals(tc, "Music", current->name);


    // <category id="people"> ...
    current = current->mgmt->next;

    CuAssertIntEquals(tc, 0, current->mgmt->genre);
    CuAssertIntEquals(tc, 0, current->mgmt->index);
    CuAssertPtrNotNull(tc, current->mgmt->parent);
    CuAssertTrue(tc, categories->mgmt->sub_head == current->mgmt->parent);
    CuAssertTrue(tc, NULL == current->mgmt->sub_head);
    CuAssertTrue(tc, NULL == current->mgmt->sub_tail);
    CuAssertTrue(tc, NULL == current->mgmt->next);

    CuAssertPtrNotNull(tc, current->id);
    CuAssertStrEquals(tc, "people", current->id);

    CuAssertPtrNotNull(tc, current->name);
    CuAssertStrEquals(tc, "People", current->name);

    CuAssertPtrNotNull(tc, categories->mgmt->sub_head->mgmt->next);

    // <category id="docs" index="true"> ...
    current = categories->mgmt->sub_head->mgmt->next;

    CuAssertIntEquals(tc, 0, current->mgmt->genre);
    CuAssertIntEquals(tc, 1, current->mgmt->index);
    CuAssertTrue(tc, categories == current->mgmt->parent);
    CuAssertPtrNotNull(tc, current->mgmt->sub_head);
    CuAssertPtrNotNull(tc, current->mgmt->sub_tail);
    CuAssertTrue(tc, NULL == current->mgmt->next);

    CuAssertPtrNotNull(tc, current->id);
    CuAssertStrEquals(tc, "docs", current->id);

    CuAssertPtrNotNull(tc, current->name);
    CuAssertStrEquals(tc, "Documentary", current->name);

    // <category id="bio"> ...
    current = current->mgmt->sub_head;

    CuAssertIntEquals(tc, 0, current->mgmt->genre);
    CuAssertIntEquals(tc, 0, current->mgmt->index);
    CuAssertPtrNotNull(tc, current->mgmt->parent);
    CuAssertTrue(tc, categories->mgmt->sub_head->mgmt->next
            == current->mgmt->parent);
    CuAssertTrue(tc, NULL == current->mgmt->sub_head);
    CuAssertTrue(tc, NULL == current->mgmt->sub_tail);

    CuAssertPtrNotNull(tc, current->id);
    CuAssertStrEquals(tc, "bio", current->id);

    CuAssertPtrNotNull(tc, current->name);
    CuAssertStrEquals(tc, "Biography", current->name);

    CuAssertPtrNotNull(tc, current->mgmt->next);

    // <category id="history"> ...
    current = current->mgmt->next;

    CuAssertIntEquals(tc, 0, current->mgmt->genre);
    CuAssertIntEquals(tc, 0, current->mgmt->index);
    CuAssertPtrNotNull(tc, current->mgmt->parent);
    CuAssertTrue(tc, categories->mgmt->sub_head->mgmt->next
            == current->mgmt->parent);
    CuAssertTrue(tc, NULL == current->mgmt->sub_head);
    CuAssertTrue(tc, NULL == current->mgmt->sub_tail);

    CuAssertPtrNotNull(tc, current->id);
    CuAssertStrEquals(tc, "history", current->id);

    CuAssertPtrNotNull(tc, current->name);
    CuAssertStrEquals(tc, "History", current->name);

    CuAssertPtrNotNull(tc, current->mgmt->next);

    // <category id="nature"> ...
    current = current->mgmt->next;

    CuAssertIntEquals(tc, 0, current->mgmt->genre);
    CuAssertIntEquals(tc, 0, current->mgmt->index);
    CuAssertPtrNotNull(tc, current->mgmt->parent);
    CuAssertTrue(tc, categories->mgmt->sub_head->mgmt->next
            == current->mgmt->parent);
    CuAssertTrue(tc, NULL == current->mgmt->sub_head);
    CuAssertTrue(tc, NULL == current->mgmt->sub_tail);

    CuAssertPtrNotNull(tc, current->id);
    CuAssertStrEquals(tc, "nature", current->id);

    CuAssertPtrNotNull(tc, current->name);
    CuAssertStrEquals(tc, "Nature", current->name);

    CuAssertTrue(tc, NULL == current->mgmt->next);
    iv_destroy_categories(categories);
}

CuSuite *iv_parse_categories_get_cusuite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_iv_parse_categories);
    return suite;
}
#endif /* LIBIVIEW_TEST */
