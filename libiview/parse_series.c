#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <assert.h>
#include <string.h>
#include <libxml/parser.h>
#include "iview.h"
#include "internal.h"

/* Sample series xml
<rss version="2.0" xmlns:media="http://search.yahoo.com/mrss/" xmlns:abc="http://www.abc.net.au/tv/mrss">
    <channel>
        <title><![CDATA[Scrapheap Challenge Series 8]]></title>
        <link>http://www.abc.net.au/iview/#/series/3094182</link>
        <description><![CDATA[The Emmy nominated hit entertainment show where teams have just one day to build incredible machines using only materials they find on the scrapheap.]]></description>
        <abc:keywords><![CDATA[Scrapheap Challenge lifestyle abc2 reality]]></abc:keywords>
        <abc:seriesDownloadURL><![CDATA[]]></abc:seriesDownloadURL>
        <copyright>2010 Australian Broadcasting Corporation</copyright>
        <language>en-au</language>
        <pubDate>Mon, 16 Aug 2010 12:58:54</pubDate>
        <image>
            <url>http://www.abc.net.au/reslib/201012/r690027_5187600.jpg</url>
            <title></title>
            <link></link>
        </image>
        <ttl>60</ttl>
        <item>
            <abc:seriesid>3094182</abc:seriesid>
            <abc:id>689983</abc:id>
            <guid isPermaLink="true">http://www.abc.net.au/iview/#/view/689983</guid>
            <title><![CDATA[Scrapheap Challenge Series 8 Episode 1]]></title>
            <category><![CDATA[Arts and Culture]]></category>

            <description><![CDATA[The Emmy nominated hit entertainment show where teams have just one day to build incredible machines using only materials they find on the scrapheap.]]></description>
            <pubDate>15/12/2010 19:30:00</pubDate>
            <media:content fileSize="235" type="video/x-flv" duration="46.38" bitrate="650"></media:content>
            <media:thumbnail url="http://www.abc.net.au/reslib/201012/r690027_5187600.jpg"></media:thumbnail>
            <media:player url="http://www.abc.net.au/iview/#/view/689983"></media:player>
            <media:restriction relationship="allow" type="country">au</media:restriction>
            <abc:shortTitle><![CDATA[Underwater Cars]]></abc:shortTitle>
            <abc:linkCopy><![CDATA[Go to website]]></abc:linkCopy>

            <abc:linkURL><![CDATA[]]></abc:linkURL>
            <abc:shopLinkCopy><![CDATA[]]></abc:shopLinkCopy>
            <abc:shopLinkURL><![CDATA[]]></abc:shopLinkURL>
            <abc:videoDownloadURL><![CDATA[]]></abc:videoDownloadURL>
            <abc:videoAsset>scrapheap_10_08_01.mp4</abc:videoAsset>
            <abc:transmitDate>15/12/2010 00:00:00</abc:transmitDate>
            <abc:expireDate>29/12/2010 19:30:00</abc:expireDate>

            <abc:rating>G</abc:rating>
            <abc:warning><![CDATA[]]></abc:warning>
        </item>
        <item>
            <abc:seriesid>3094182</abc:seriesid>
            <abc:id>689987</abc:id>
            <guid isPermaLink="true">http://www.abc.net.au/iview/#/view/689987</guid>
            <title><![CDATA[Scrapheap Challenge Series 8 Episode 2]]></title>

            <category><![CDATA[Arts and Culture]]></category>
            <description><![CDATA[The Emmy nominated hit entertainment show where teams have just one day to build incredible machines using only materials they find on the scrapheap.]]></description>
            <pubDate>16/12/2010 19:30:00</pubDate>
            <media:content fileSize="240" type="video/x-flv" duration="47.37" bitrate="650"></media:content>
            <media:thumbnail url="http://www.abc.net.au/reslib/201012/r690027_5187600.jpg"></media:thumbnail>
            <media:player url="http://www.abc.net.au/iview/#/view/689987"></media:player>
            <media:restriction relationship="allow" type="country">au</media:restriction>
            <abc:shortTitle><![CDATA[Kung Fu Cars]]></abc:shortTitle>

            <abc:linkCopy><![CDATA[Go to website]]></abc:linkCopy>
            <abc:linkURL><![CDATA[]]></abc:linkURL>
            <abc:shopLinkCopy><![CDATA[]]></abc:shopLinkCopy>
            <abc:shopLinkURL><![CDATA[]]></abc:shopLinkURL>
            <abc:videoDownloadURL><![CDATA[]]></abc:videoDownloadURL>
            <abc:videoAsset>scrapheap_10_08_02.mp4</abc:videoAsset>
            <abc:transmitDate>16/12/2010 00:00:00</abc:transmitDate>
            <abc:expireDate>30/12/2010 19:30:00</abc:expireDate>

            <abc:rating>G</abc:rating>
            <abc:warning><![CDATA[]]></abc:warning>
        </item>
    </channel>
</rss>
 */

enum item_parse_state {
    ps_start,
    ps_rss,
    ps_channel,
    ps_item,
    ps_id,
    ps_title,
    ps_url,
    ps_description,
    ps_thumbnail,
    ps_date,
    ps_rating,
    ps_link,
    ps_home,
    ps_end
};

struct iv_episode_list {
    unsigned int len;
    struct iv_episode *head;
};

struct item_parse_ctx {
    enum item_parse_state state;
    int return_value;
    struct iv_episode_list *items;
};

static void start_item_child_handler(struct item_parse_ctx *ctx,
        const xmlChar *name, const xmlChar **attrs) {
    if(!xmlStrcmp(BAD_CAST("abc:id"), name)) {
        ctx->state = ps_id;
    } else if(!xmlStrcmp(BAD_CAST("title"), name)) {
        ctx->state = ps_title;
    } else if(!xmlStrcmp(BAD_CAST("media:thumbnail"), name)) {
        ctx->state = ps_thumbnail;
        (ctx->items->head[ctx->items->len-1]).thumbnail =
            xmlStrdup(attrs[1]);
    } else if(!xmlStrcmp(BAD_CAST("media:player"), name)) {
        ctx->state = ps_link;
        (ctx->items->head[ctx->items->len-1]).link =
            xmlStrdup(attrs[1]);
    } else if(!xmlStrcmp(BAD_CAST("description"), name)) {
        ctx->state = ps_description;
    } else if(!xmlStrcmp(BAD_CAST("abc:linkURL"), name)) {
        ctx->state = ps_home;
    } else if(!xmlStrcmp(BAD_CAST("abc:videoAsset"), name)) {
        ctx->state = ps_url;
    } else if(!xmlStrcmp(BAD_CAST("abc:transmitDate"), name)) {
        ctx->state = ps_date;
    } else if(!xmlStrcmp(BAD_CAST("abc:rating"), name)) {
        ctx->state = ps_rating;
    } else {
        IV_DEBUG("unhandled element: %s\n", name);
    }
}

static void start_element(void *_ctx, const xmlChar *name,
        const xmlChar **attrs) {
    struct item_parse_ctx *ctx = (struct item_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    switch(ctx->state) {
        case ps_start:
            // Root element should be 'rss'
            if(xmlStrcmp(BAD_CAST("rss"), name)) {
                ctx->return_value = IV_EXML;
                IV_DEBUG("Found unexpected element %s\n", name);
                return;
            }
            ctx->state = ps_rss;
            return;
        case ps_rss:
            // Shouldn't have any elemente except 'channel'
            if(xmlStrcmp(BAD_CAST("channel"), name)) {
                ctx->return_value = IV_EXML;
                IV_DEBUG("Found unexpected element %s\n", name);
                return;
            }
            ctx->state = ps_channel;
            break;
        case ps_channel:
            // We only care about the 'item' element
            if(!xmlStrcmp(BAD_CAST("item"), name)) {
                ctx->state = ps_item;
                // Found new item, add another element to the list
                if(!(ctx->items->head = realloc(ctx->items->head,
                                ++(ctx->items->len)*sizeof(struct iv_episode)))) {
                    ctx->return_value = -IV_ENOMEM;
                    return;
                }
                memset(&(ctx->items->head[ctx->items->len-1]), 0,
                        sizeof(struct iv_episode));
            } else {
                IV_DEBUG("Skipping non-item element %s\n", name);
            }
            break;
        case ps_item:
            start_item_child_handler(ctx, name, attrs);
            break;
        default:
            break;
    }
}

static void end_element(void *_ctx, const xmlChar *name) {
    struct item_parse_ctx *ctx = (struct item_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    if(!xmlStrcmp(BAD_CAST("abc:id"), name) ||
            !xmlStrcmp(BAD_CAST("title"), name) ||
            !xmlStrcmp(BAD_CAST("media:thumbnail"), name) ||
            !xmlStrcmp(BAD_CAST("media:player"), name) ||
            !xmlStrcmp(BAD_CAST("description"), name) ||
            !xmlStrcmp(BAD_CAST("abc:linkURL"), name) ||
            !xmlStrcmp(BAD_CAST("abc:videoAsset"), name) ||
            !xmlStrcmp(BAD_CAST("abc:transmitDate"), name) ||
            !xmlStrcmp(BAD_CAST("abc:rating"), name)) {
        ctx->state = ps_item;
    } else if(!xmlStrcmp(BAD_CAST("item"), name)) {
        ctx->state = ps_channel;
    } else if(!xmlStrcmp(BAD_CAST("channel"), name)) {
        ctx->state = ps_rss;
    } else if(!xmlStrcmp(BAD_CAST("rss"), name)) {
        ctx->state = ps_end;
    }
}

static void content_handler(void *_ctx, const xmlChar *_ch, int len) {
    struct item_parse_ctx *ctx = (struct item_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    xmlChar *ch = xmlStrndup(_ch, len);
    if(!ch) {
        ctx->return_value = -IV_ENOMEM;
        return;
    }
    struct iv_episode *item = &ctx->items->head[ctx->items->len-1];
    switch(ctx->state) {
        case ps_id:
            item->id = atoi((char *)ch);
            free(ch);
            break;
        case ps_url:
            item->url = ch;
            break;
        case ps_date:
            item->date = ch;
            break;
        case ps_rating:
            item->rating = ch;
            break;
        default:
            break;
    }
}

static void cdata_handler(void *_ctx, const xmlChar *_data, int len) {
    struct item_parse_ctx *ctx = (struct item_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    char *data;
    if(0 > strntrim(&data, (char *)_data, len, "\"")) {
        ctx->return_value = -IV_ENOMEM;
        return;
    }
    struct iv_episode *item = &ctx->items->head[ctx->items->len-1];
    switch(ctx->state) {
        case ps_title:
            item->title = BAD_CAST(data);
            break;
        case ps_description:
            item->description = BAD_CAST(data);
            break;
        case ps_home:
            item->home = BAD_CAST(data);
            break;
        default:
            free(data);
            break;
    }
}

int iv_parse_series(char *buf, size_t len, struct iv_episode **items) {
    // Instantiate SAX parser
    xmlSAXHandlerPtr handler = calloc(1, sizeof(xmlSAXHandler));
    handler->initialized = XML_SAX2_MAGIC;
    handler->startElement = start_element;
    handler->characters = content_handler;
    handler->endElement = end_element;
    handler->cdataBlock = cdata_handler;
    // Initialise parser context
    struct iv_episode_list item_list = { .len = 0, .head = NULL };
    if(!(item_list.head = calloc(++(item_list.len), sizeof(struct iv_episode)))) {
        return -IV_ENOMEM;
    }
    struct item_parse_ctx ctx = {
        .state = ps_start,
        .return_value = IV_OK,
        .items = &item_list
    };
    // Parse document
    xmlParserCtxtPtr pctx =
        xmlCreatePushParserCtxt(handler, &ctx, NULL, 0, NULL);
    xmlCtxtUseOptions(pctx, XML_PARSE_NOENT);
    const int result = xmlParseChunk(pctx, buf, len, 0);
    xmlParseChunk(pctx, NULL, 0, 1);
    xmlFreeParserCtxt(pctx);
    if(0 != result) {
        return -IV_ESAXPARSE;
    }
    *items = ctx.items->head;
    return ctx.items->len;
}
