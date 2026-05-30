#include "../include/cf_analyzer.h"
#include <curl/curl.h>

int main(int argc, char *argv[]) {
    char user_id[64];
    
    if(argc < 2) {
        printf("请输入Codeforces账号: ");
        scanf("%63s", user_id);
    } else {
        strcpy(user_id, argv[1]);
    }
    
    printf("\n========================================\n");
    printf("  Codeforces 账号分析工具\n");
    printf("========================================\n");
    printf("正在查询: %s\n\n", user_id);
    
    printf("[1/5] 获取账号信息...\n");
    UserData* my_data = get_user_data(user_id);
    if(my_data == NULL) {
        printf("错误: 查不到这个账号，请检查输入\n");
        return 1;
    }
    printf("      账号: %s\n", my_data->name);
    printf("      当前分: %d\n", my_data->now_score);
    printf("      头衔: %s\n", my_data->now_title);
    printf("      历史最高: %d\n\n", my_data->high_score);
    
    printf("[2/5] 获取比赛记录...\n");
    int game_cnt = 0;
    GameInfo* my_games = get_user_games(user_id, &game_cnt);
    if(my_games && game_cnt > 0) {
        printf("      共参加 %d 场比赛\n\n", game_cnt);
    } else {
        printf("      没有比赛记录\n\n");
    }
    
    printf("[3/5] 获取提交记录（可能要等一会儿）...\n");
    int submit_cnt = 0;
    SubmitRecord* my_subs = get_user_submits(user_id, &submit_cnt);
    if(my_subs && submit_cnt > 0) {
        printf("      获取到 %d 条通过记录\n", submit_cnt);
        
        printf("[4/5] 分析补题情况...\n");
        check_fix(my_subs, submit_cnt, my_games, game_cnt);
        printf("      分析完毕\n");
    } else {
        printf("      没有提交记录\n");
    }
    
    printf("[5/5] 生成网页报告...\n");
    char* html_code = make_html(user_id, my_data, my_games, game_cnt);
    
    char out_file[256];
    sprintf(out_file, "output/%s_report.html", user_id);
    FILE* fp = fopen(out_file, "w");
    if(fp) {
        fprintf(fp, "%s", html_code);
        fclose(fp);
        printf("      报告已保存: %s\n", out_file);
        
        char open_cmd[512];
        sprintf(open_cmd, "start %s", out_file);
        system(open_cmd);
        printf("      已在浏览器中打开\n");
    } else {
        printf("      保存失败\n");
    }
    
    free(my_data);
    free(my_games);
    free(my_subs);
    free(html_code);
    
    printf("\n========================================\n");
    printf("  查询完成！\n");
    printf("========================================\n");
    
    return 0;
}