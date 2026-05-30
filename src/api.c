#include "../include/cf_analyzer.h"
#include <curl/curl.h>
#include "../cJSON.h"

extern char* get_html(const char* url);

static int curl_ready = 0;

typedef struct {
    char *data_ptr;
    size_t data_len;
} MemBlock;

static size_t save_data(void *content, size_t unit, size_t count, void *user_ptr) {
    size_t real_len = unit * count;
    MemBlock *store = (MemBlock *)user_ptr;
    
    char *new_ptr = realloc(store->data_ptr, store->data_len + real_len + 1);
    if(!new_ptr) return 0;
    
    store->data_ptr = new_ptr;
    memcpy(&(store->data_ptr[store->data_len]), content, real_len);
    store->data_len += real_len;
    store->data_ptr[store->data_len] = 0;
    
    return real_len;
}

char* get_html(const char* url) {
    CURL *curl;
    CURLcode result;
    MemBlock buf;
    
    buf.data_ptr = malloc(1);
    buf.data_len = 0;
    
    if(!curl_ready) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_ready = 1;
    }
    
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buf);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        
        result = curl_easy_perform(curl);
        
        if(result != CURLE_OK) {
            fprintf(stderr, "网络错误: %s\n", curl_easy_strerror(result));
            free(buf.data_ptr);
            buf.data_ptr = NULL;
        }
        
        curl_easy_cleanup(curl);
    }
    
    return buf.data_ptr;
}

UserData* get_user_data(const char* user_name) {
    char api_url[256];
    sprintf(api_url, "https://codeforces.com/api/user.info?handles=%s", user_name);
    
    char* json_text = get_html(api_url);
    if(!json_text) return NULL;
    
    cJSON *root = cJSON_Parse(json_text);
    free(json_text);
    
    if(!root) return NULL;
    
    cJSON *state = cJSON_GetObjectItem(root, "status");
    if(!state || strcmp(state->valuestring, "OK") != 0) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON *data_list = cJSON_GetObjectItem(root, "result");
    cJSON *user_json = cJSON_GetArrayItem(data_list, 0);
    
    UserData *my_info = (UserData*)malloc(sizeof(UserData));
    memset(my_info, 0, sizeof(UserData));
    
    cJSON *name_json = cJSON_GetObjectItem(user_json, "handle");
    if(name_json) strcpy(my_info->name, name_json->valuestring);
    
    cJSON *score_json = cJSON_GetObjectItem(user_json, "rating");
    if(score_json) my_info->now_score = score_json->valueint;
    else my_info->now_score = 0;
    
    cJSON *title_json = cJSON_GetObjectItem(user_json, "rank");
    if(title_json) strcpy(my_info->now_title, title_json->valuestring);
    else strcpy(my_info->now_title, "未定级");
    
    cJSON *best_json = cJSON_GetObjectItem(user_json, "maxRating");
    if(best_json) my_info->high_score = best_json->valueint;
    else my_info->high_score = my_info->now_score;
    
    cJSON *best_title_json = cJSON_GetObjectItem(user_json, "maxRank");
    if(best_title_json) strcpy(my_info->high_title, best_title_json->valuestring);
    else strcpy(my_info->high_title, my_info->now_title);
    
    cJSON_Delete(root);
    return my_info;
}

GameInfo* get_user_games(const char* user_name, int* game_num) {
    char api_url[256];
    sprintf(api_url, "https://codeforces.com/api/user.rating?handle=%s", user_name);
    
    char* json_text = get_html(api_url);
    if(!json_text) return NULL;
    
    cJSON *root = cJSON_Parse(json_text);
    free(json_text);
    
    if(!root) return NULL;
    
    cJSON *state = cJSON_GetObjectItem(root, "status");
    if(!state || strcmp(state->valuestring, "OK") != 0) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON *data_list = cJSON_GetObjectItem(root, "result");
    *game_num = cJSON_GetArraySize(data_list);
    
    if(*game_num == 0) {
        cJSON_Delete(root);
        return NULL;
    }
    
    GameInfo* game_list = (GameInfo*)malloc(sizeof(GameInfo) * (*game_num));
    memset(game_list, 0, sizeof(GameInfo) * (*game_num));
    
    for(int i = 0; i < *game_num; i++) {
        cJSON *one_game = cJSON_GetArrayItem(data_list, i);
        
        cJSON *gid = cJSON_GetObjectItem(one_game, "contestId");
        if(gid) game_list[i].game_id = gid->valueint;
        
        cJSON *gname = cJSON_GetObjectItem(one_game, "contestName");
        if(gname) strcpy(game_list[i].game_name, gname->valuestring);
        
        cJSON *old_rat = cJSON_GetObjectItem(one_game, "oldRating");
        if(old_rat) game_list[i].old_score = old_rat->valueint;
        
        cJSON *new_rat = cJSON_GetObjectItem(one_game, "newRating");
        if(new_rat) game_list[i].new_score = new_rat->valueint;
        
        cJSON *my_rank = cJSON_GetObjectItem(one_game, "rank");
        if(my_rank) game_list[i].my_rank = my_rank->valueint;
        
        cJSON *up_time = cJSON_GetObjectItem(one_game, "ratingUpdateTimeSeconds");
        if(up_time) game_list[i].game_time = up_time->valueint;
        
        game_list[i].score_change = game_list[i].new_score - game_list[i].old_score;
    }
    
    cJSON_Delete(root);
    return game_list;
}