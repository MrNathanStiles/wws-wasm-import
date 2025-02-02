#ifndef ACTRGITHUB_H
#define ACTRGITHUB_H

extern int actr_fetch_text_length(char * url, int urlLength, int mapId, char * name, int nameLength);

extern int actr_fetch_json_length(char * url, int urlLength, int jsonId);

// fetch the url asynchronously, puts the result into the provided map
// calls actr_async_result with the returned handle and success indicator  
// returns the handle of the asynchronous operation
int actr_fetch_text(char * url, int mapId, char * name) {
    return actr_fetch_text_length(url, strlen(url), mapId, name, strlen(name));
}

// fetch the url asynchronously, puts the result into the provided json container
// calls actr_async_result with the returned handle and success indicator  
// returns the handle of the asynchronous operation
int actr_fetch_json(char * url, int jsonId) {
    return actr_fetch_json_length(url, strlen(url), jsonId);
}


#endif
