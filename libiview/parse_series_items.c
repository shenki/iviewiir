#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <assert.h>
#include <string.h>
#include <libxml/parser.h>
#include "iview.h"

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

struct iv_item_list {
    size_t len;
    struct iv_item *head;
};

struct item_parse_ctx {
    enum item_parse_state state;
    int return_value;
    struct iv_item_list *items;
};

static void start_item_child_handler(struct item_parse_ctx *ctx,
        const xmlChar *localname, const xmlChar *prefix,
        const xmlChar **attrs) {
    if(!(xmlStrcmp(BAD_CAST("abc"), prefix) ||
                xmlStrcmp(BAD_CAST("id"), localname))) {
        ctx->state = ps_id;
    } else if(!xmlStrcmp(BAD_CAST("title"), localname)) {
        ctx->state = ps_title;
    } else if(!(xmlStrcmp(BAD_CAST("media"), prefix) ||
                xmlStrcmp(BAD_CAST("thumbnail"), localname))) {
        ctx->state = ps_thumbnail;
        (ctx->items->head[ctx->items->len]).thumbnail =
            xmlStrdup(attrs[1]);
    } else if(!(xmlStrcmp(BAD_CAST("media"), prefix) ||
                xmlStrcmp(BAD_CAST("player"), localname))) {
        ctx->state = ps_link;
        (ctx->items->head[ctx->items->len]).link =
            xmlStrdup(attrs[1]);
    } else if(!xmlStrcmp(BAD_CAST("description"), localname)) {
        ctx->state = ps_description;
    } else if(!(xmlStrcmp(BAD_CAST("abc"), prefix) ||
                xmlStrcmp(BAD_CAST("linkURL"), localname))) {
        ctx->state = ps_home;
    } else if(!(xmlStrcmp(BAD_CAST("abc"), prefix) ||
                xmlStrcmp(BAD_CAST("videoAsset"), localname))) {
        ctx->state = ps_url;
    } else if(!(xmlStrcmp(BAD_CAST("abc"), prefix) ||
                xmlStrcmp(BAD_CAST("transmitDate"), localname))) {
        ctx->state = ps_date;
    } else if(!(xmlStrcmp(BAD_CAST("abc"), prefix) ||
                xmlStrcmp(BAD_CAST("rating"), localname))) {
        ctx->state = ps_rating;
    } else {
        IV_DEBUG("unhandled element: %s:%s", prefix, localname);
    }
}

static void start_element_ns(void *_ctx,
        const xmlChar *localname,
        const xmlChar *prefix,
        const xmlChar *uri IV_UNUSED,
        int nb_namespaces IV_UNUSED,
        const xmlChar **namespaces IV_UNUSED,
        int nb_attributes IV_UNUSED,
        int nb_defaulted IV_UNUSED,
        const xmlChar **attrs) {
    struct item_parse_ctx *ctx = (struct item_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    switch(ctx->state) {
        case ps_start:
            // Root element should be 'rss'
            if(xmlStrcmp(BAD_CAST("rss"), localname)) {
                ctx->return_value = IV_EXML;
                return;
            }
            ctx->state = ps_rss;
            return;
        case ps_rss:
            // Shouldn't have any elemente except 'channel'
            if(xmlStrcmp(BAD_CAST("channel"), localname)) {
                ctx->return_value = IV_EXML;
                return;
            }
            ctx->state = ps_channel;
            break;
        case ps_channel:
            // We only care about the 'item' element
            if(!xmlStrcmp(BAD_CAST("item"), localname)) {
                ctx->state = ps_item;
                // Found new item, add another element to the list
                if(!(ctx->items->head = realloc(ctx->items->head,
                                ++(ctx->items->len)*sizeof(struct iv_item)))) {
                    ctx->return_value = -IV_ENOMEM;
                    return;
                }
                memset(&(ctx->items->head[ctx->items->len-1]), 0,
                        sizeof(struct iv_item));
            }
            break;
        case ps_item:
            start_item_child_handler(ctx, localname, prefix, attrs);
            break;
        default:
            break;
    }
}

static void end_element_ns(void *_ctx,
        const xmlChar *localname IV_UNUSED,
        const xmlChar *prefix IV_UNUSED,
        const xmlChar *uri IV_UNUSED) {
    struct item_parse_ctx *ctx = (struct item_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    switch(ctx->state) {
        case ps_rss:
             ctx->state = ps_end;
             break;
        case ps_channel:
             ctx->state = ps_rss;
             break;
        case ps_item:
             ctx->state = ps_channel;
             break;
        default:
             break;
    }
}

static void cdata_block(void *_ctx, const xmlChar *data, int len) {
    struct item_parse_ctx *ctx = (struct item_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    xmlChar *_data = xmlStrndup(data, len);
    struct iv_item *item = &ctx->items->head[ctx->items->len-1];
    switch(ctx->state) {
        case ps_id:
            item->id = atoi((char *)_data);
            free(_data);
            break;
        case ps_title:
            item->title = _data;
            break;
        case ps_url:
            item->url = _data;
            break;
        case ps_description:
            item->description = _data;
            break;
        case ps_date:
            item->date = _data;
            break;
        case ps_rating:
            item->rating = _data;
            break;
        case ps_home:
            item->home = _data;
            break;
        default:
            break;
    }
}

ssize_t iv_parse_series_items(char *buf, size_t len, struct iv_item **items) {
    // Instantiate SAX parser
    xmlSAXHandlerPtr handler = calloc(1, sizeof(xmlSAXHandler));
    handler->startElementNs = start_element_ns;
    handler->endElementNs = end_element_ns;
    handler->cdataBlock = cdata_block;
    // Initialise parser context
    struct iv_item_list item_list = { .len = 0, .head = NULL };
    if(!(item_list.head = malloc(++(item_list.len)*sizeof(struct iv_item)))) {
        perror("malloc");
        return IV_ENOMEM;
    }
    *items = item_list.head;
    struct item_parse_ctx ctx = {
        .state = ps_start,
        .return_value = IV_OK,
        .items = &item_list
    };
    // Parse document
    if(0 > xmlSAXUserParseMemory(handler, &ctx, buf, len)) {
        return -IV_ESAXPARSE;
    }
    return ctx.return_value;
}
