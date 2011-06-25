#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <assert.h>
#include <string.h>
#include <json/json.h>
#include "iview.h"
#include "internal.h"

char *extract_json_string(json_object *obj) {
    char *trimmed, *sanitised;
    if(!strtrim(&trimmed, json_object_to_json_string(obj), "\"")) {
        return NULL;
    }
    strrpl(&sanitised, trimmed, "\\/", "/");
    free(trimmed);
    return sanitised;
}

/* Sample JSON structure
 * [{"a":"2986845",
     "b":"dirtgirlworld",
     "c":"Dirt Girl World is a celebration of life outside. It's a place of bizarre insects, underground tunnels, vaudevillian trained chickens, and performing stunt bugs. Funky pop songs, guitars with attitude, beats, grooves and loops, all blended with a tractor, make up the infectiously cool music in Dirt Girl World.",
     "d":"http:\/\/www.abc.net.au\/reslib\/201008\/r623390_4199898.jpg",
     "e":"dirtgirlworld pre-school",
     "f":[
           {
                "a":"635502",
                "b":"Episode 15 Walkabout",
                "d":"Today dirtgirl discovers, scrapboy swaps, and feathers fly when ken becomes a birdman in grubby's latest stunt sensation.",
                "e":"pre-school",
                "f":"2011-05-05 07:00:00",
                "g":"2011-06-04 07:00:00",
                "i":"55",
                "j":"660",
                "m":"G",
                "n":"kids\/dirtgirl_10_01_15.mp4",
                "s":"http:\/\/www.abc.net.au\/reslib\/201008\/r623390_4199898.jpg",
                "u":"1",
                "v":"15"
           },{
                "a":"635499",
                "b":"Episode 14 Home",
                "d":"dirtgirl finds out where peas call home and an old cardboard box is transformed into a bedroom fit for a super stunt star weevil.",
                "e":"pre-school",
                "f":"2011-04-28 07:00:00",
                "g":"2011-05-28 07:00:00",
                "i":"55",
                "j":"660",
                "m":"G",
                "n":"kids\/dirtgirl_10_01_14.mp4",
                "s":"http:\/\/www.abc.net.au\/reslib\/201008\/r623390_4199898.jpg",
                "u":"1",
                "v":"14"
           },{
                "a":"633855",
                "b":"Episode 13 Jam",
                "d":"Strawberry jam, a tricky chicken and a disappearing stunt bug...now there's a dirtgirl recipe for fun.",
                "e":"pre-school",
                "f":"2011-04-21 07:10:00",
                "g":"2011-05-21 07:10:00",
                "i":"55",
                "j":"660",
                "m":"G",
                "n":"kids\/dirtgirl_10_01_13.mp4",
                "s":"http:\/\/www.abc.net.au\/reslib\/201008\/r623390_4199898.jpg",
                "u":"1",
                "v":"13"
           }
        ]
    }]
 */
int iv_parse_series(const char *buf, struct iv_episode **items) {
    IV_DEBUG("Series list:\n%s\n", buf);
    json_object *json_series = json_tokener_parse(buf);
    const int series_len = json_object_array_length(json_series);
    if(0 >= series_len) { return 0; }
    if(1 < series_len) { IV_DEBUG("%d series found!\n", series_len); }
    json_object *json_entry = json_object_array_get_idx(json_series, 0);
    json_object *json_episodes =
        json_object_object_get(json_entry, JSON_SERIES_EPISODES);
    const int num_episodes = json_object_array_length(json_episodes);
    *items =
        (struct iv_episode *)calloc(num_episodes, sizeof(struct iv_episode));
    int i;
    for(i=0; i<num_episodes; i++) {
        // Extract the element
        json_object *json_element = json_object_array_get_idx(json_episodes, i);
        json_object *obj = NULL;
        // Populate id
        obj = json_object_object_get(json_element, JSON_EPISODE_ID);
        (*items)[i].id = json_object_get_int(obj);
        // Populate title
        (*items)[i].title = extract_json_string(
                json_object_object_get(json_element, JSON_EPISODE_NAME));
        // Populate url, trimming stupid escapes
        (*items)[i].url = extract_json_string(
                json_object_object_get(json_element, JSON_EPISODE_FILENAME));
        // Populate description
        (*items)[i].description = extract_json_string(
                json_object_object_get(json_element, JSON_EPISODE_DESCRIPTION));
        // Populate thumbnail, trimming stupid escapes
        (*items)[i].thumbnail = extract_json_string(
                json_object_object_get(json_element, JSON_EPISODE_IMAGE));
        // Populate date
        (*items)[i].date =
            extract_json_string(json_object_object_get(json_element,
                        JSON_EPISODE_TRANSMISSION));
        // Populate rating
        (*items)[i].rating =
                extract_json_string(json_object_object_get(json_element,
                        JSON_EPISODE_CLASSIFICATION));
        // Populate size
        obj = json_object_object_get(json_element, JSON_EPISODE_SIZE_MB);
        (*items)[i].size_mb = json_object_get_int(obj);
        // Populate length
        obj = json_object_object_get(json_element, JSON_EPISODE_LENGTH_SEC);
        (*items)[i].length_sec = json_object_get_int(obj);
    }
    array_list_free(json_object_get_array(json_series));
    free(json_series);
    return num_episodes;
}

#ifdef LIBIVIEW_TEST
#include "test/CuTest.h"

static const char *series_buf =
"[{\"a\":\"2986845\",\
     \"b\":\"dirtgirlworld\",\
     \"c\":\"Dirt Girl World is a celebration of life outside. It's a place of bizarre insects, underground tunnels, vaudevillian trained chickens, and performing stunt bugs. Funky pop songs, guitars with attitude, beats, grooves and loops, all blended with a tractor, make up the infectiously cool music in Dirt Girl World.\",\
     \"d\":\"http:\\/\\/www.abc.net.au\\/reslib\\/201008\\/r623390_4199898.jpg\",\
     \"e\":\"dirtgirlworld pre-school\",\
     \"f\":[\
           {\
                \"a\":\"635502\",\
                \"b\":\"Episode 15 Walkabout\",\
                \"d\":\"Today dirtgirl discovers, scrapboy swaps, and feathers fly when ken becomes a birdman in grubby's latest stunt sensation.\",\
                \"e\":\"pre-school\",\
                \"f\":\"2011-05-05 07:00:00\",\
                \"g\":\"2011-06-04 07:00:00\",\
                \"i\":\"55\",\
                \"j\":\"660\",\
                \"m\":\"G\",\
                \"n\":\"kids\\/dirtgirl_10_01_15.mp4\",\
                \"s\":\"http:\\/\\/www.abc.net.au\\/reslib\\/201008\\/r623390_4199898.jpg\",\
                \"u\":\"1\",\
                \"v\":\"15\"\
           },{\
                \"a\":\"635499\",\
                \"b\":\"Episode 14 Home\",\
                \"d\":\"dirtgirl finds out where peas call home and an old cardboard box is transformed into a bedroom fit for a super stunt star weevil.\",\
                \"e\":\"pre-school\",\
                \"f\":\"2011-04-28 07:00:00\",\
                \"g\":\"2011-05-28 07:00:00\",\
                \"i\":\"55\",\
                \"j\":\"660\",\
                \"m\":\"G\",\
                \"n\":\"kids\\/dirtgirl_10_01_14.mp4\",\
                \"s\":\"http:\\/\\/www.abc.net.au\\/reslib\\/201008\\/r623390_4199898.jpg\",\
                \"u\":\"1\",\
                \"v\":\"14\"\
           },{\
                \"a\":\"633855\",\
                \"b\":\"Episode 13 Jam\",\
                \"d\":\"Strawberry jam, a tricky chicken and a disappearing stunt bug...now there's a dirtgirl recipe for fun.\",\
                \"e\":\"pre-school\",\
                \"f\":\"2011-04-21 07:10:00\",\
                \"g\":\"2011-05-21 07:10:00\",\
                \"i\":\"55\",\
                \"j\":\"660\",\
                \"m\":\"G\",\
                \"n\":\"kids\\/dirtgirl_10_01_13.mp4\",\
                \"s\":\"http:\\/\\/www.abc.net.au\\/reslib\\/201008\\/r623390_4199898.jpg\",\
                \"u\":\"1\",\
                \"v\":\"13\"\
           }\
        ]\
    }]";

void test_iv_parse_series(CuTest *tc) {
    struct iv_episode *episodes;
    const int result = iv_parse_series(series_buf, &episodes);
    CuAssertIntEquals(tc, 3, result);
    CuAssertPtrNotNull(tc, episodes);

    CuAssertIntEquals(tc, 635502, episodes[0].id);
    CuAssertPtrNotNull(tc, episodes[0].title);
    CuAssertStrEquals(tc, "Episode 15 Walkabout", episodes[0].title);
    CuAssertPtrNotNull(tc, episodes[0].url);
    CuAssertStrEquals(tc, "kids/dirtgirl_10_01_15.mp4", episodes[0].url);
    CuAssertPtrNotNull(tc, episodes[0].description);
    CuAssertStrEquals(tc, "Today dirtgirl discovers, scrapboy swaps, and feathers fly when ken becomes a birdman in grubby's latest stunt sensation.", episodes[0].description);
    CuAssertPtrNotNull(tc, episodes[0].thumbnail);
    CuAssertStrEquals(tc, "http://www.abc.net.au/reslib/201008/r623390_4199898.jpg", episodes[0].thumbnail);
    CuAssertPtrNotNull(tc, episodes[0].date);
    CuAssertStrEquals(tc, "2011-05-05 07:00:00", episodes[0].date);
    CuAssertPtrNotNull(tc, episodes[0].rating);
    CuAssertStrEquals(tc, "G", episodes[0].rating);
    CuAssertIntEquals(tc, 55, episodes[0].size_mb);
    CuAssertIntEquals(tc, 660, episodes[0].length_sec);

    CuAssertIntEquals(tc, 635499, episodes[1].id);
    CuAssertPtrNotNull(tc, episodes[1].title);
    CuAssertStrEquals(tc, "Episode 14 Home", episodes[1].title);
    CuAssertPtrNotNull(tc, episodes[1].url);
    CuAssertStrEquals(tc, "kids/dirtgirl_10_01_14.mp4", episodes[1].url);
    CuAssertPtrNotNull(tc, episodes[1].description);
    CuAssertStrEquals(tc, "dirtgirl finds out where peas call home and an old cardboard box is transformed into a bedroom fit for a super stunt star weevil.", episodes[1].description);
    CuAssertPtrNotNull(tc, episodes[1].thumbnail);
    CuAssertStrEquals(tc, "http://www.abc.net.au/reslib/201008/r623390_4199898.jpg", episodes[1].thumbnail);
    CuAssertPtrNotNull(tc, episodes[1].date);
    CuAssertStrEquals(tc, "2011-04-28 07:00:00", episodes[1].date);
    CuAssertPtrNotNull(tc, episodes[1].rating);
    CuAssertStrEquals(tc, "G", episodes[1].rating);
    CuAssertIntEquals(tc, 55, episodes[1].size_mb);
    CuAssertIntEquals(tc, 660, episodes[1].length_sec);

    CuAssertIntEquals(tc, 633855, episodes[2].id);
    CuAssertPtrNotNull(tc, episodes[2].title);
    CuAssertStrEquals(tc, "Episode 13 Jam", episodes[2].title);
    CuAssertPtrNotNull(tc, episodes[2].url);
    CuAssertStrEquals(tc, "kids/dirtgirl_10_01_13.mp4", episodes[2].url);
    CuAssertPtrNotNull(tc, episodes[2].description);
    CuAssertStrEquals(tc, "Strawberry jam, a tricky chicken and a disappearing stunt bug...now there's a dirtgirl recipe for fun.", episodes[2].description);
    CuAssertPtrNotNull(tc, episodes[2].thumbnail);
    CuAssertStrEquals(tc, "http://www.abc.net.au/reslib/201008/r623390_4199898.jpg", episodes[2].thumbnail);
    CuAssertPtrNotNull(tc, episodes[2].date);
    CuAssertStrEquals(tc, "2011-04-21 07:10:00", episodes[2].date);
    CuAssertPtrNotNull(tc, episodes[2].rating);
    CuAssertStrEquals(tc, "G", episodes[2].rating);
    CuAssertIntEquals(tc, 55, episodes[2].size_mb);
    CuAssertIntEquals(tc, 660, episodes[2].length_sec);
    iv_destroy_series(episodes, result);
}

CuSuite *iv_parse_series_get_cusuite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_iv_parse_series);
    return suite;
}
#endif /* LIBIVIEW_TEST */
