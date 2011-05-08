#ifndef LIBIVIEW_H
#define LIBIVIEW_H

/* iview.h
 *
 * libiview is a C library for scraping information and downloading episodes
 * from the Australian Broadcast Corporation's iView online TV viewing
 * platform[1][2]. libiview's primary goal is as a support library for
 * iviewiir, a homebrew iView client for the Nintendo Wii.
 *
 * libiview reserves the "iv_" prefix to identify structures and functions in
 * the global namespace.
 *
 * [1] http://www.abc.net.au/
 * [2] http://www.abc.net.au/iview/
 */

#include <stdlib.h>
#include <errno.h>
#include <libxml/xmlstring.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(DEBUG)
#define IV_DEBUG(format, ...) \
        fprintf(stderr, "DEBUG (%s:%d): " format, \
                __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define IV_DEBUG(...) do {} while (0)
#endif

/* Typesafe min/max macros from Linux: include/linux/kernel.h */
#define IV_MIN(x, y) ({        \
            typeof(x) _min1 = (x);      \
            typeof(y) _min2 = (y);      \
            (void) (&_min1 == &_min2);    \
            _min1 < _min2 ? _min1 : _min2; })

#define IV_MAX(x, y) ({        \
            typeof(x) _max1 = (x);      \
            typeof(y) _max2 = (y);      \
            (void) (&_max1 == &_max2);    \
            _max1 > _max2 ? _max1 : _max2; })

#define IV_XML_ATTR_NAME(attrs) (attrs[1])
#define IV_XML_ATTR_VALUE(attrs) (attrs[3])

/*
 * IV_UNUSED - a parameter is unused
 *
 * Some compilers (eg. gcc with -W or -Wunused) warn about unused
 * function parameters.  This suppresses such warnings and indicates
 * to the reader that it's deliberate.
 */
#define IV_UNUSED __attribute__((unused))

/* IV_CONFIG_URI
 *
 * The current location for iView's initial XML configuration file. It's
 * intended consumer is iv_get_config().
 */
#define IV_CONFIG_URI "http://www.abc.net.au/iview/xml/config.xml?r=374"

/* Return values */
#define IV_OK 0
#define IV_EURIPARSE 1
#define IV_EREQUEST 2
#define IV_ESAXPARSE 3
#define IV_EXML 4
#define IV_ENOMEM ENOMEM

/* struct iv_config:
 *
 * The main configuration structure, containing values parsed from the
 * configuration XML (see IV_CONFIG_URI, defined above). As these values are
 * intrinsic to the iView platform and only of internal use to the library,
 * iv_config is partially declared to avoid confusion and misuse of the
 * library.
 */
struct iv_config;

/* struct iv_series
 *
 * Represents a series as listed by iView. struct iv_series is used as input to
 * functions retrieving and manipulating series items.
 */
struct iv_series {
    unsigned int id;
    const xmlChar *title;
};

/* struct iv_item
 *
 * Provides information on a particular episode, including its download URL.
 * Consumers of this struct include functions for displaying episode data and
 * fetching the associated video. Currently this struct only gives access to a
 * small-ish number of the elements available in the episode's XML description,
 * however these have been more than enough for current users.
 */
struct iv_item {
    unsigned int id;
    xmlChar *title;
    xmlChar *url;
    xmlChar *description;
    xmlChar *thumbnail;
    xmlChar *date;
    xmlChar *rating;
    xmlChar *link; /* link to play on iView site */
    xmlChar *home; /* Program website */
};

/* struct iv_auth
 *
 * Consumed by iv_generate_video_uri, the auth struct provides a session token
 * for fetching the video from the iView server, plus some other data. As the
 * internals shouldn't be of concern to libiview consumers it is a partial
 * declaration.
 */
struct iv_auth;

/*
 * Currently unused inlining macros to work around the conflict between GNU89
 * inlines and C99 (which are virtually opposed in implementation).
 */
#if __STDC_VERSION__ == 199901L
#define INLINE inline
#elif __GNUC__
#define INLINE extern inline
#else
#define INLINE
#endif

/* iv_get_xml_buffer
 *
 * Given a URL, will populate buf_ptr with the data retrieved over HTTP (uses
 * the nanoHTTP implementation from libxml2). The primary purpose of
 * iv_get_xml_buffer in libiview is for fetching the various XML files required
 * before downloading an episode.
 *
 * Freeing of the buffer pointed at by buf_ptr is the responsibility of the
 * caller.
 *
 * @uri: The URI of the data to fetch over HTTP.
 * @buf_ptr: The dereferenced pointer will point at the buffered data.
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the value is greater than or equal to zero it represents the
 * size of the buffer now pointed at by the dereferenced buf_ptr, otherwise
 * it's value is the negated error code.
 */
ssize_t iv_get_xml_buffer(const char *uri, char **buf_ptr);

/* iv_destroy_xml_buffer
 *
 * For freeing buffers allocated by calls to iv_get_xml_buffer()
 */
#define iv_destroy_xml_buffer(buf) free(buf)

/* iv_get_config
 *
 * Taking in an XML buffer, provides and populates a pointer to an instance of
 * struct iv_config. Typically the provided buffer should be fetched using
 * iv_get_xml_buffer() with the value of the uri parameter being IV_CONFIG_URI.
 *
 * Freeing of the struct iv_config instance is the responsibility of the
 * caller.
 *
 * @buf: The configuration XML buffer
 * @len: The length of the XML configuration buffer
 * @config: A container for the configuration struct.
 *
 * @return: 0 on success, less than zero on failure.
 */
int iv_get_config(const char *buf, size_t len, struct iv_config **config);

/* iv_easy_config
 *
 * Get a populated iv_config struct without the bother of fetching XML buffers.
 *
 * @config: A container for the configuration struct
 *
 * @return 0 on success, less than zero on failure.
 */
int iv_easy_config(struct iv_config **config);

/* iv_destroy_config
 *
 * For freeing the struct iv_config instance returned by iv_get_config()
 *
 * @config: The struct iv_config instance to free
 */
void iv_destroy_config(struct iv_config *config);

/* iv_get_index
 *
 * Retrieves the buffer containing the series list from iView. The buffer
 * should be passed to iv_parse_index().
 *
 * It is the responsibility of the caller to free the populated XML buffer
 *
 * @config: The configuration context returned by iv_get_config()
 * @buf_ptr: A container for the series list JSON data.
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the result is greater than or equal to zero it represents the
 * size of the buffer held in buf_ptr. Otherwise the value represents the
 * negated error code and buf_ptr is invalid.
 */
ssize_t iv_get_index(struct iv_config *config, char **buf_ptr);

/* iv_parse_index
 *
 * Parses a series list JSON buffer into constituant struct iv_series instances.
 * Freeing the series array is the responsibility of the caller.
 *
 * @buf: The series list JSON buffer to parse. Must be null terminated.
 * @index_ptr: A container for the series list.
 *
 * @return: Greater than or equal to zero 0 on success, less than zero on
 * failure. If the call is successful index_ptr contains a list of series and
 * the return value represents the number of elements in the list. If the call
 * is not successful the value contained in index_ptr is invalid.
 */
int iv_parse_index(const char *buf, struct iv_series **index_ptr);

/* iv_easy_index
 *
 * Get a populated series index without the bother of fetching XML buffers.
 *
 * @config: The configuration context returned by iv_get_config()
 * @index_ptr: A container for the index struct array
 *
 * @return: Greater than or equal to zero 0 on success, less than zero on
 * failure. If the call is successful index_ptr contains a list of series and
 * the return value represents the number of elements in the list. If the call
 * is not successful the value contained in index_ptr is invalid.
 */
int iv_easy_index(struct iv_config *config, struct iv_series **index_ptr);

/* iv_find_series
 *
 * Searches through the series index for the provided series ID, populating the
 * series parameter if found.
 *
 * @series_id: The SID for the series required
 * @series_list: The list provided by iv_parse_index()
 * @series_len: The length of the series list as returned by iv_parse_index()
 * @series_ptr: A container for the series struct if the SID is matched. Pass
 * NULL if not required.
 *
 * @return: Greater than or equal to zero on success, -1 on failure. If the
 * call is successful series_ptr contains the struct associated with the series
 * ID and the return value is the index into the series list. If the call is
 * not successful the value contained in series_ptr is invalid.
 */
int iv_find_series(const unsigned int series_id, const struct iv_series *series_list,
        const unsigned int series_len, const struct iv_series **series_ptr);

/* iv_destroy_index
 *
 * Destroys the index list created by iv_parse_index()
 *
 * @index: A pointer to the first struct iv_series instance in the list
 * @len: The length of the list
 */
void iv_destroy_index(struct iv_series *index, int len);

/* iv_get_series_items
 *
 * Fetches an XML buffer containing information on episodes for the given
 * series (typically gathered through iv_get_index()/iv_parse_index()). Freeing
 * the items XML buffer is the responsibility of the caller.
 *
 * @config: The configuration context returned by iv_get_config()
 * @series: A pointer to a particular struct iv_series instance.
 * @buf_ptr: A container for the series items XML data
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the call is successful the return value represents the size of
 * the buffer held in buf_ptr. If unsuccessful, the value is the negated error
 * code.
 */
ssize_t iv_get_series_items(struct iv_config *config,
        struct iv_series *series, char **buf_ptr);

/* iv_parse_series_items
 *
 * Parses an item list XML buffer into constituant struct iv_item instances.
 * Freeing the struct iv_item array is the responsibility of the caller.
 * Individual elements of the resulting list can be passed to
 * iv_generate_video_uri in preparation for downloading the episode.
 *
 * @buf: The buffer to parse
 * @len: The length of the buffer
 * @items: A container for the items list
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the result is greater than or equal to zero it represents the
 * length of the items list. If it is less than zero the value is the negated
 * error code.
 */
int iv_parse_series_items(char *buf, size_t len, struct iv_item **items);

/* iv_easy_series_items
 *
 * Get a populated items list without the bother of fetching XML buffers.
 *
 * @config: The configuration context returned by iv_get_config()
 * @items_ptr: A container for the item struct array.
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the result is greater than or equal to zero it represents the
 * length of the items list. If it is less than zero the value is the negated
 * error code.
 */
int iv_easy_series_items(struct iv_config *config, struct iv_series *series,
        struct iv_item **items_ptr);

/* iv_find_item
 *
 * Searches through the items list for the provided item ID, populating the
 * item_ptr parameter if found.
 *
 * @item_id: The SID for the series required
 * @items_list: The list provided by iv_parse_series_items()
 * @items_len: The length of the items list returned by iv_parse_series_items()
 * @items_ptr: A container for the series struct if the SID is matched. Pass
 * NULL if not required.
 *
 * @return: Greater than or equal to zero on success, -1 on failure. If the
 * call is successful series_ptr contains the struct associated with the series
 * ID and the return value is the index into the series list. If the call is
 * not successful the value contained in series_ptr is invalid.
 */
int iv_find_item(const unsigned int item_id, const struct iv_item *items_list,
        const unsigned int items_len, const struct iv_item **item_ptr);

/* iv_destroy_series_items
 *
 * Destroys the items list created by iv_parse_series_items()
 *
 * @items: The item list as created by iv_parse_series_items()
 * @items_len: The return value of the call to iv_parse_series_items
 */
void iv_destroy_series_items(struct iv_item *items, int items_len);

/* iv_get_auth
 *
 * Fetches and parses the iView authentication XML document, populating an
 * instance of struct iv_auth to be passed to iv_generate_video_uri(). It is
 * the responsibility of the caller to free the struct iv_auth instance.
 *
 * @config: The configuration context as provided by iv_get_config()
 * @auth: A container for the populated struct iv_auth instance.
 *
 * @return: 0 on success, less than zero on failure. If the call fails auth
 * is invalid.
 */
int iv_get_auth(const struct iv_config *config, struct iv_auth **auth);

/* iv_destroy_auth
 *
 * Destroys an instance of struct iv_auth as provided by iv_get_auth()
 *
 * @auth: The instance of struct iv_auth to free
 */
void iv_destroy_auth(struct iv_auth *auth);

/* iv_destroy_vieo_uri
 *
 * Frees the RTMP URI generated by iv_generate_video_uri()
 *
 * @uri: The URI string to free
 */
#define iv_destroy_video_uri(uri) free(uri)

/* iv_fetch_video
 *
 * Downloads the episode represented by item to the file outpath.
 *
 * @auth: The struct iv_auth instance returned by iv_get_auth()
 * @item: A element of the item list returned by iv_get_series_items(), the
 * item that wants
 * @outpath: The filename to write the downloaded data to.
 *
 * @return: 0 on success, less than zero on failure. Values less than
 * zero represent an error code (IV_E*)
 */
int iv_fetch_video(const struct iv_auth *auth, const struct iv_item *item,
        const int fd);

/* iv_easy_fetch_video
 *
 * Downloads an episode without the bother of fetching an authentication
 * struct.
 *
 * @config: The configuration context as provided by iv_get_config().
 * @item: The item to download.
 * @outpath: The filename to write to.
 *
 * @return: 0 on success, less than zero on failure. Values less than
 * zero represent an error code (IV_E*)
 */
int iv_easy_fetch_video(const struct iv_config *config,
        const struct iv_item *item, const int fd);

#ifdef __cplusplus
};
#endif

#endif /* LIBIVIEW_H */
