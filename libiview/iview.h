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
#include <unistd.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* IV_CONFIG_URI
 *
 * The current location for iView's initial XML configuration file. It's
 * intended consumer is iv_parse_config().
 */
#define IV_CONFIG_URI "http://www.abc.net.au/iview/xml/config.xml?r=374"

/* Return values */
#define IV_OK 0
#define IV_EURIPARSE 1001
#define IV_EREQUEST 1002
#define IV_ESAXPARSE 1003
#define IV_EXML 1004
#define IV_EEMPTY 1005

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
    const char *title;
    const char *keywords;
};

/* struct iv_episode
 *
 * Provides information on a particular episode, including its download URL.
 * Consumers of this struct include functions for displaying episode data and
 * fetching the associated video. Currently this struct only gives access to a
 * small-ish number of the elements available in the episode's XML description,
 * however these have been more than enough for current users.
 */
struct iv_episode {
    unsigned int id;
    const char *title;
    const char *url;
    const char *description;
    const char *thumbnail;
    const char *date;
    const char *rating;
    int size_mb;
    int length_sec;
};

/* struct iv_auth
 *
 * Consumed by iv_get_auth() and iv_(easy_)?fetch_episode(), the auth struct
 * provides a session token for fetching the video from the iView server, plus
 * some other data. As the internals shouldn't be of concern to libiview
 * consumers it is a partial declaration.
 */
struct iv_auth;

/* struct iv_progress
 *
 * Consumed by implementations of iv_download_progess_cb, struct iv_progress
 * outlines the current state of the download including total bytes downloaded,
 * duration of the episode in MS, the percentage of the duration downloaded
 * (i.e. the current position of 'playback') and whether or not the
 * duration/percentage is valid.
 *
 * The valid field is required as the duration is not known until at least one
 * read of the data stream has been performed. Prior to obtaining the actual
 * duration, the duration is temporarily set to two hours which may produce
 * incorrect percentage calculations, hence the valid field.
 *
 * Once the download is complete the done field is set to 1.
 */
struct iv_progress {
    size_t count; // downloaded byte count
    double duration; // duration in ms
    double percentage; // percentage of downloaded duration
    short valid; // 1 if the percentage/duration is valid, 0 otherwise
    int done; // 1 if the download the download is complete
};

/* iv_download_progress_cb
 *
 * A callback prototype for reporting download progress back to the
 * application. Implementations of this prototype can be passed to the
 * iv_(easy_)?fetch_episode_async() functions.
 */
typedef int iv_download_progress_cb(const struct iv_progress *progress,
        void *user_data);

/* iv_get_http_buffer
 *
 * Given a URL, will populate buf_ptr with the data retrieved over HTTP (uses
 * the nanoHTTP implementation from libxml2). The primary purpose of
 * iv_get_http_buffer in libiview is for fetching the various XML files required
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
ssize_t iv_get_http_buffer(const char *uri, char **buf_ptr);

/* iv_destroy_http_buffer
 *
 * For freeing buffers allocated by calls to iv_get_http_buffer()
 */
void iv_destroy_http_buffer(char *buf);

/* iv_parse_config
 *
 * Taking in an XML buffer, provides and populates a pointer to an instance of
 * struct iv_config. Typically the provided buffer should be fetched using
 * iv_get_http_buffer() with the value of the uri parameter being IV_CONFIG_URI.
 *
 * Freeing of the struct iv_config instance is the responsibility of the
 * caller.
 *
 * @buf: The configuration XML buffer
 * @len: The length of the XML configuration buffer
 * @config: A container for the configuration struct.
 *
 * @return: IV_OK on success, negated error code on failure.
 */
int iv_parse_config(const char *buf, size_t len, struct iv_config **config);

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
 * For freeing the struct iv_config instance returned by iv_parse_config()
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
 * @config: The configuration context returned by iv_parse_config()
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
 * @config: The configuration context returned by iv_parse_config()
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

/* iv_get_series
 *
 * Fetches a JSON buffer containing information on episodes for the given
 * series (typically gathered through iv_get_index()/iv_parse_index()). Freeing
 * the item JSON buffer is the responsibility of the caller.
 *
 * @config: The configuration context returned by iv_parse_config()
 * @series: A pointer to a particular struct iv_series instance.
 * @buf_ptr: A container for the series items XML data
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the call is successful the return value represents the size of
 * the buffer held in buf_ptr. If unsuccessful, the value is the negated error
 * code.
 */
ssize_t iv_get_series(struct iv_config *config,
        struct iv_series *series, char **buf_ptr);

/* iv_parse_series
 *
 * Parses an item list JSON buffer into constituant struct iv_episode instances.
 * Freeing the struct iv_episode array is the responsibility of the caller.
 * Individual elements of the resulting list can be passed to
 * iv_(easy_)?fetch_episode() for downloading.
 *
 * @buf: The buffer to parse
 * @items: A container for the items list
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the result is greater than or equal to zero it represents the
 * length of the items list. If it is less than zero the value is the negated
 * error code.
 */
int iv_parse_series(const char *buf, struct iv_episode **items);

/* iv_easy_series
 *
 * Get a populated items list without the bother of fetching XML buffers.
 *
 * @config: The configuration context returned by iv_parse_config()
 * @items_ptr: A container for the item struct array.
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the result is greater than or equal to zero it represents the
 * length of the items list. If it is less than zero the value is the negated
 * error code.
 */
int iv_easy_series(struct iv_config *config, struct iv_series *series,
        struct iv_episode **items_ptr);

/* iv_find_episode
 *
 * Searches through the items list for the provided item ID, populating the
 * item_ptr parameter if found.
 *
 * @item_id: The SID for the series required
 * @items_list: The list provided by iv_parse_series()
 * @items_len: The length of the items list returned by iv_parse_series()
 * @items_ptr: A container for the series struct if the SID is matched. Pass
 * NULL if not required.
 *
 * @return: Greater than or equal to zero on success, -1 on failure. If the
 * call is successful series_ptr contains the struct associated with the series
 * ID and the return value is the index into the series list. If the call is
 * not successful the value contained in series_ptr is invalid.
 */
int iv_find_episode(const unsigned int item_id, const struct iv_episode *items_list,
        const unsigned int items_len, const struct iv_episode **item_ptr);

/* iv_destroy_series
 *
 * Destroys the items list created by iv_parse_series()
 *
 * @items: The item list as created by iv_parse_series()
 * @items_len: The return value of the call to iv_parse_series
 */
void iv_destroy_series(struct iv_episode *items, int items_len);

/* iv_get_auth
 *
 * Fetches the iView authentication XML document to be passed to iv_parse_auth.
 *
 * @config: The configuration context as provided by iv_parse_config()
 * @buf: A container for the XML document
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the value is greater than or equal to zero it represents the
 * size of the buffer now pointed at by the dereferenced buf_ptr, otherwise
 * it's value is the negated error code.
 */
ssize_t iv_get_auth(const struct iv_config *config, char **buf_ptr);

/* iv_parse_auth
 *
 * Parses the iView authentication XML buffer returned by iv_get_auth,
 * populating an instance of struct iv_auth to be passed to
 * iv_(easy_)?fetch_episode(). It is the responsibility of the caller to free
 * the struct iv_auth instance.
 *
 * @buf: The authentication XML data as retrieved by iv_get_auth()
 * @len: The length of |buf|
 * @auth: A container for the populated struct iv_auth instance.
 *
 * @return: IV_OK on success, negated error code on failure. If the call fails
 * auth is invalid.
 */
int iv_parse_auth(const char *buf, size_t len, struct iv_auth **auth);

/* iv_easy_auth
 *
 * Get a populated instance of the authentication struct without the bother of
 * fetching / parsing.
 *
 * @config: The configuration context as provided by iv_parse_config()
 * @auth: A container for the authentication struct
 *
 * @return: IV_OK on success, negated error code on failure.
 */
int iv_easy_auth(const struct iv_config *config, struct iv_auth **auth);

/* iv_destroy_auth
 *
 * Destroys an instance of struct iv_auth as provided by iv_get_auth()
 *
 * @auth: The instance of struct iv_auth to free
 */
void iv_destroy_auth(struct iv_auth *auth);

/* iv_fetch_episode
 *
 * Downloads the episode represented by item, writing it to the provided file
 * descriptor.
 *
 * @auth: The struct iv_auth instance returned by iv_get_auth()
 * @item: A element of the item list returned by iv_get_series(), the
 * item that wants
 * @fd: The file descriptor to write the downloaded data to.
 * @offset: The number of milliseconds into the stream to start downloading.
 *
 * @return: IV_OK on success, negated error code on failure.
 */
int iv_fetch_episode(const struct iv_auth *auth,
        const struct iv_episode *item,
        const int fd,
        const uint32_t offset);

/* iv_fetch_episode_async
 *
 * Downloads the episode represented by item, writing it to the provided file
 * descriptor.
 *
 * @auth: The struct iv_auth instance returned by iv_get_auth()
 * @item: A element of the item list returned by iv_get_series(), the
 * item that wants
 * @fd: The file descriptor to write the downloaded data to.
 * @offset: The number of milliseconds into the stream to start downloading.
 * @progress_cb: The progress callback function to trigger throughout the
 * download. Can be NULL if not required.
 * @user_data: The user data to provide to progress_cb. Can be NULL if not
 * required.
 *
 * @return: IV_OK on success, negated error code on failure.
 */
int iv_fetch_episode_async(const struct iv_auth *auth,
        const struct iv_episode *item,
        const int fd,
        const uint32_t offset,
        iv_download_progress_cb *progress_cb,
        void *user_data);

/* iv_easy_fetch_episode
 *
 * Downloads an episode without the bother of populating an authentication
 * struct.
 *
 * @config: The configuration context as provided by iv_parse_config().
 * @item: The episode to download.
 * @fd: The file descriptor to write the downloaded data to.
 * @offset: The number of milliseconds into the stream to start downloading.
 *
 * @return: IV_OK on success, negated error code on failure.
 */
int iv_easy_fetch_episode(const struct iv_config *config,
        const struct iv_episode *item,
        const int fd,
        const uint32_t offset);

/* iv_easy_fetch_episode_async
 *
 * Downloads an episode without the bother of populating an authentication
 * struct. As data is downloaded, progress_cb() is called with the current
 * download state and user data passed through.
 *
 * @config: The configuration context as provided by iv_parse_config().
 * @item: The episode to download.
 * @fd: The file descriptor to write the downloaded data to.
 * @offset: The number of milliseconds into the stream to start downloading.
 * @progress_cb: The progress callback function to trigger throughout the
 * download. Can be NULL if not required.
 * @user_data: The user data to provide to progress_cb. Can be NULL if not
 * required.
 *
 * @return: IV_OK on success, negated error code on failure.
 */
int iv_easy_fetch_episode_async(
        const struct iv_config *config,
        const struct iv_episode *item,
        const int fd,
        const uint32_t offset,
        iv_download_progress_cb *progress_cb,
        void *user_data);

/* iv_get_categories
 *
 * Creates a buffer containing the categories XML data from iView. The buffer
 * should be passed to iv_parse_categories().
 *
 * It is the responsibility of the caller to free the populated XML buffer.
 *
 * @config: The configuration context returned by iv_parse_config()
 * @buf_ptr: A container for the categories XML data
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the result is greater than or equal to zero it represents the
 * size of the buffer held in buf_ptr. Otherwise the value represents the
 * negated error code and buf_ptr is invalid.
 */
ssize_t iv_get_categories(const struct iv_config *config, char **buf_ptr);

/* struct iv_category
 *
 * Represents an element of the category tree provided by iView (see
 * iv_parse_categories() implementation or the categories.xml file from iView
 * for more information). Allows the user to sort series and episodes by
 * category/genre. A partial definition of iv_category_mgmt is provided for
 * type safety and general avoidance of void* declarations. Operations on
 * category structures should be carried out with the associated helpers.
 */
struct iv_category_mgmt;
struct iv_category {
    char *id;
    char *name;
    struct iv_category_mgmt *mgmt;
};

/* struct iv_category_list
 *
 * Used to create a list of genres, indexes and categories from the category tree.
 */
struct iv_category_list {
    struct iv_category *category;
    struct iv_category_list *next;
};

/* iv_parse_categories
 *
 * Parses a categories XML buffer into an array of categories. Freeing the
 * array is the responsibility of the caller.
 *
 * @buf: The categories XML buffer to parse. Must be null terminated.
 * @categories_ptr: A container for the categories list.
 *
 * @return: Greater than or equal to zero 0 on success, less than zero on
 * failure. If the call is successful categories_ptr contains a list of
 * categories and the return value represents the number of elements in the
 * list. If the call is not successful the value contained in categories_ptr is
 * invalid.
 */
int iv_parse_categories(const char *buf, size_t len,
        struct iv_category **categories_ptr);

/* iv_destroy_categories
 *
 * Destroys the category hierarchy provided by iv_parse_categories().
 *
 * @categories: The category hierarchy to free.
 */
void iv_destroy_categories(struct iv_category *categories);

/* iv_easy_categories
 *
 * Get a populated categories list without the bother of fetching XML buffers.
 *
 * @config: The configuration context as returned by iv_parse_config()
 * @categories: A container for the tree of categories
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the call is successful categories_ptr contains a list of valid
 * categories and the return value represents the number of elements in the
 * list. If the call is not successful the value contained in categories_ptr is
 * invalid.
 */
int iv_easy_categories(const struct iv_config *config,
        struct iv_category **categories_ptr);

/* iv_destroy_category_list
 *
 * Deallocates a linked list created by one of the iv_list_*() functions.
 *
 * @list: The list to free
 */
void iv_destroy_category_list(struct iv_category_list *list);

/* iv_list_categories
 *
 * Provides a linked list of all catories associated categories.
 *
 * @categories: The root or a subcategory as provided by iv_(parse|easy)_categories()
 * @list: A container for the list of categories
 *
 * @return: IV_OK on success, less than 0 on failure.
 */
int iv_list_categories(const struct iv_category *categories,
        struct iv_category_list **list);

/* iv_list_genres
 *
 * Provides a linked list containing all genres described in the category tree
 * provided by iv_(parse|easy)_categories() (must be the root node).
 *
 * @categories: The root or a subcategory as provided by iv_(parse|easy)_categories()
 * @list: A container for the list of genres.
 *
 * @return: IV_OK on success, less than 0 on failure.
 */
int iv_list_genres(const struct iv_category *categories,
        struct iv_category_list **list);

/* iv_list_indicies
 *
 * Provides a linked list containing all indices described in the category tree
 * provided by iv_(parse|easy)_categories() (must be the root node).
 *
 * @categories: The root or a subcategory as provided by iv_(parse|easy)_categories()
 * @list: A container for the list of indices.
 *
 * @return: IV_OK on success, less than 0 on failure.
 */
int iv_list_indices(const struct iv_category *categories,
        struct iv_category_list **list);

/* iv_series_list
 *
 * Used to create a list of series filtered on some criteria, for example genre.
 */
struct iv_series_list {
    struct iv_series *series;
    struct iv_series_list *next;
};

/* iv_destroy_series_list
 *
 * Dispose of a series list as returned by iv_list_series_by or similar.
 *
 * @list: The list to deallocate.
 */
void iv_destroy_series_list(struct iv_series_list *list);

/* iv_list_series_by
 *
 * Filter the whole series list by a category.
 */
int iv_list_series_by(const char *id, struct iv_series *series,
        const int num_series, struct iv_series_list **list);

#ifdef __cplusplus
};
#endif

#endif /* LIBIVIEW_H */
