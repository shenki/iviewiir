#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <assert.h>
#include <string.h>
#include <json/json.h>
#include "iview.h"
#include "internal.h"

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
int iv_parse_series(char *buf, struct iv_episode **items) {
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
        obj = json_object_object_get(json_element, JSON_EPISODE_NAME);
        if(-1 == strtrim((char **)&((*items)[i].title),
                    json_object_to_json_string(obj), "\"")) {
            (*items)[i].title = NULL;
        }
        // Populate url
        obj = json_object_object_get(json_element, JSON_EPISODE_FILENAME);
        if(-1 == strtrim((char **)&((*items)[i].url),
                    json_object_to_json_string(obj), "\"")) {
            (*items)[i].url = NULL;
        }
        // Populate description
        obj = json_object_object_get(json_element, JSON_EPISODE_DESCRIPTION);
        if(-1 == strtrim((char **)&((*items)[i].description),
                json_object_to_json_string(obj), "\"")) {
            (*items)[i].description = NULL;
        }
        // Populate thumbnail
        obj = json_object_object_get(json_element, JSON_EPISODE_IMAGE);
        if(-1 == strtrim((char **)&((*items)[i].thumbnail),
                    json_object_to_json_string(obj), "\"")) {
            (*items)[i].thumbnail = NULL;
        }
        // Populate date
        obj = json_object_object_get(json_element, JSON_EPISODE_TRANSMISSION);
        if(-1 == strtrim((char **)&((*items)[i].date),
                    json_object_to_json_string(obj), "\"")) {
            (*items)[i].date = NULL;
        }
        // Populate rating
        obj = json_object_object_get(json_element, JSON_EPISODE_CLASSIFICATION);
        if(-1 == strtrim((char **)&((*items)[i].rating),
                    json_object_to_json_string(obj), "\"")) {
            (*items)[i].rating = NULL;
        }
    }
    array_list_free(json_object_get_array(json_series));
    free(json_series);
    return num_episodes;
}
