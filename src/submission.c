#include "../include/cf_analyzer.h"
#include <curl/curl.h>
#include "../cJSON.h"

extern char* get_html(const char* url);

SubmitRecord* get_user_submits(const char* user_name, int* submit_num) {
    int max_num = 5000;
    SubmitRecord* all_sub = (SubmitRecord*)malloc(sizeof(SubmitRecord) * max_num);
    memset(all_sub, 0, sizeof(SubmitRecord) * max_num);
    int total = 0;
    
    for(int start = 1; start <= 5000; start += 1000) {
        char api_url[512];
        sprintf(api_url, "https://codeforces.com/api/user.status?handle=%s&from=%d&count=1000", 
                user_name, start);
        
        char* json_text = get_html(api_url);
        if(!json_text) break;
        
        cJSON *root = cJSON_Parse(json_text);
        free(json_text);
        if(!root) break;
        
        cJSON *state = cJSON_GetObjectItem(root, "status");
        if(!state || strcmp(state->valuestring, "OK") != 0) {
            cJSON_Delete(root);
            break;
        }
        
        cJSON *data_list = cJSON_GetObjectItem(root, "result");
        int size = cJSON_GetArraySize(data_list);
        
        for(int i = 0; i < size && total < max_num; i++) {
            cJSON *one_sub = cJSON_GetArrayItem(data_list, i);
            
            cJSON *gid = cJSON_GetObjectItem(one_sub, "contestId");
            if(!gid) continue;
            all_sub[total].game_id = gid->valueint;
            
            cJSON *result = cJSON_GetObjectItem(one_sub, "verdict");
            if(result && strcmp(result->valuestring, "OK") == 0) {
                all_sub[total].is_ac = 1;
            } else {
                all_sub[total].is_ac = 0;
            }
            
            cJSON *problem = cJSON_GetObjectItem(one_sub, "problem");
            if(problem) {
                cJSON *pid = cJSON_GetObjectItem(problem, "index");
                if(pid) strcpy(all_sub[total].problem_id, pid->valuestring);
                
                cJSON *pname = cJSON_GetObjectItem(problem, "name");
                if(pname) strcpy(all_sub[total].problem_name, pname->valuestring);
                
                cJSON *pscore = cJSON_GetObjectItem(problem, "rating");
                if(pscore) all_sub[total].problem_score = pscore->valueint;
            }
            
            cJSON *sub_time = cJSON_GetObjectItem(one_sub, "creationTimeSeconds");
            if(sub_time) all_sub[total].submit_time = sub_time->valueint;
            
            total++;
        }
        
        cJSON_Delete(root);
        if(size < 1000) break;
    }
    
    *submit_num = total;
    return all_sub;
}

void check_fix(SubmitRecord* subs, int sub_count, GameInfo* games, int game_count) {
    for(int i = 0; i < game_count; i++) {
        games[i].pass_during = 0;
        games[i].pass_after = 0;
    }
    
    for(int i = 0; i < sub_count; i++) {
        if(!subs[i].is_ac) continue;
        
        for(int j = 0; j < game_count; j++) {
            if(subs[i].game_id == games[j].game_id) {
                time_t end_time = games[j].game_time;
                if(subs[i].submit_time <= end_time) {
                    games[j].pass_during++;
                } else {
                    games[j].pass_after++;
                }
                break;
            }
        }
    }
}