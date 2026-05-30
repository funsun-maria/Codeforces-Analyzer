#ifndef CF_ANALYZER_H
#define CF_ANALYZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    char name[64];
    int now_score;
    char now_title[32];
    int high_score;
    char high_title[32];
    int total_games;
} UserData;

typedef struct {
    int game_id;
    char game_name[128];
    int old_score;
    int new_score;
    int my_rank;
    int score_change;
    time_t game_time;
    int pass_during;
    int pass_after;
} GameInfo;

typedef struct {
    int game_id;
    char problem_id[8];
    char problem_name[64];
    int problem_score;
    int is_ac;
    time_t submit_time;
} SubmitRecord;

UserData* get_user_data(const char* user_name);
GameInfo* get_user_games(const char* user_name, int* game_num);
SubmitRecord* get_user_submits(const char* user_name, int* submit_num);
void check_fix(SubmitRecord* subs, int sub_count, GameInfo* games, int game_count);
char* make_html(const char* user_name, UserData* user, GameInfo* games, int game_count);

const char* get_color_by_score(int score);
int check_recent(time_t old_time, int days);
void get_recent_stats(GameInfo* games, int game_num, int* recent_num, int* recent_best);

#endif