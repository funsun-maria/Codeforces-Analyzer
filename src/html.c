#include "../include/cf_analyzer.h"
#include <time.h>

void get_time_str(time_t t, char* out, int size) {
    struct tm* tm_info = localtime(&t);
    strftime(out, size, "%Y-%m-%d", tm_info);
}

const char* get_color_by_score(int score) {
    if(score >= 3000) return "#FF0000";
    if(score >= 2600) return "#FF0000";
    if(score >= 2400) return "#FF8C00";
    if(score >= 2100) return "#AA00AA";
    if(score >= 1900) return "#0000FF";
    if(score >= 1600) return "#03A89E";
    if(score >= 1400) return "#008000";
    if(score >= 1200) return "#808080";
    return "#CCCCCC";
}

int check_recent(time_t old_time, int days) {
    time_t now = time(NULL);
    time_t before = now - days * 24 * 3600;
    return old_time >= before;
}

void get_recent_stats(GameInfo* games, int game_num, int* recent_num, int* recent_best) {
    *recent_num = 0;
    *recent_best = 0;
    
    for(int i = 0; i < game_num; i++) {
        if(check_recent(games[i].game_time, 180)) {
            (*recent_num)++;
            if(games[i].new_score > *recent_best) {
                *recent_best = games[i].new_score;
            }
        }
    }
}

char* make_html(const char* user_name, UserData* user, GameInfo* games, int game_num) {
    int recent_cnt = 0, recent_max = 0;
    get_recent_stats(games, game_num, &recent_cnt, &recent_max);
    
    char* html_buf = (char*)malloc(500 * 1024);
    char* pos = html_buf;
    int left = 500 * 1024;
    
    pos += snprintf(pos, left,
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <title>CF分析报告 - %s</title>\n"
        "    <script src=\"https://cdn.jsdelivr.net/npm/echarts@5.4.3/dist/echarts.min.js\"></script>\n"
        "    <style>\n"
        "        body { font-family: 'Segoe UI', Arial; background: linear-gradient(135deg, #667eea, #764ba2); min-height: 100vh; padding: 20px; }\n"
        "        .box { max-width: 1400px; margin: 0 auto; background: white; border-radius: 20px; overflow: hidden; }\n"
        "        .head { background: linear-gradient(135deg, #667eea, #764ba2); color: white; padding: 40px; text-align: center; }\n"
        "        .name { font-size: 48px; font-weight: bold; }\n"
        "        .cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; padding: 30px; background: #f8f9fa; }\n"
        "        .card { background: white; padding: 20px; border-radius: 15px; text-align: center; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n"
        "        .card-value { font-size: 32px; font-weight: bold; }\n"
        "        .chart-box { height: 450px; margin: 30px; padding: 20px; background: white; border-radius: 15px; }\n"
        "        .table-box { margin: 30px; overflow-x: auto; }\n"
        "        table { width: 100%%; border-collapse: collapse; }\n"
        "        th { background: #667eea; color: white; padding: 12px; }\n"
        "        td { padding: 10px; border-bottom: 1px solid #ddd; }\n"
        "        .up { color: green; font-weight: bold; }\n"
        "        .down { color: red; font-weight: bold; }\n"
        "        .foot { text-align: center; padding: 20px; background: #f8f9fa; color: #666; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "<div class=\"box\">\n"
        "    <div class=\"head\">\n"
        "        <div class=\"name\" style=\"color: %s\">%s</div>\n"
        "        <div>%s</div>\n"
        "    </div>\n",
        user_name, get_color_by_score(user->now_score), user_name, user->now_title);
    
    pos += snprintf(pos, left,
        "    <div class=\"cards\">\n"
        "        <div class=\"card\"><div>当前分</div><div class=\"card-value\" style=\"color: %s\">%d</div></div>\n"
        "        <div class=\"card\"><div>历史最高</div><div class=\"card-value\" style=\"color: %s\">%d</div></div>\n"
        "        <div class=\"card\"><div>参赛次数</div><div class=\"card-value\">%d</div></div>\n"
        "        <div class=\"card\"><div>近180天</div><div class=\"card-value\">%d</div></div>\n"
        "        <div class=\"card\"><div>近180天最高</div><div class=\"card-value\" style=\"color: %s\">%d</div></div>\n"
        "    </div>\n",
        get_color_by_score(user->now_score), user->now_score,
        get_color_by_score(user->high_score), user->high_score,
        game_num,
        recent_cnt,
        get_color_by_score(recent_max), recent_max);
    
    pos += snprintf(pos, left,
        "    <div class=\"chart-box\">\n"
        "        <div id=\"scoreChart\" style=\"height: 100%%;\"></div>\n"
        "    </div>\n"
        "    <script>\n"
        "        var myChart = echarts.init(document.getElementById('scoreChart'));\n"
        "        myChart.setOption({\n"
        "            title: { text: '分数变化趋势', left: 'center' },\n"
        "            tooltip: { trigger: 'axis' },\n"
        "            xAxis: { type: 'category', data: [");
    
    int start = game_num > 50 ? game_num - 50 : 0;
    for(int i = start; i < game_num; i++) {
        pos += snprintf(pos, left, "'%s'%s", 
                       games[i].game_name,
                       i < game_num - 1 ? ", " : "");
    }
    
    pos += snprintf(pos, left,
        "], axisLabel: { rotate: 45 } },\n"
        "            yAxis: { type: 'value', name: '分数' },\n"
        "            series: [{ name: '分数', type: 'line', data: [");
    
    for(int i = start; i < game_num; i++) {
        pos += snprintf(pos, left, "%d%s", 
                       games[i].new_score,
                       i < game_num - 1 ? ", " : "");
    }
    
    pos += snprintf(pos, left,
        "], smooth: true, lineStyle: { width: 3, color: '#667eea' }, areaStyle: { opacity: 0.3 } }]\n"
        "        });\n"
        "    </script>\n");
    
    pos += snprintf(pos, left,
        "    <h2 style=\"margin: 0 30px;\">比赛记录</h2>\n"
        "    <div class=\"table-box\">\n"
        "        <table>\n"
        "            <tr><th>比赛名称</th><th>日期</th><th>赛前分</th><th>赛后分</th><th>变化</th><th>排名</th><th>赛中</th><th>补题</th></tr>\n");
    
    for(int i = game_num - 1; i >= 0; i--) {
        char date_str[32];
        get_time_str(games[i].game_time, date_str, 32);
        
        char sign = games[i].score_change >= 0 ? '+' : '-';
        int abs_change = abs(games[i].score_change);
        const char* change_class = games[i].score_change >= 0 ? "up" : "down";
        
        pos += snprintf(pos, left,
            "            <tr>\n"
            "                <td>%s</td>\n"
            "                <td>%s</td>\n"
            "                <td style=\"color: %s\">%d</td>\n"
            "                <td style=\"color: %s\">%d</td>\n"
            "                <td class=\"%s\">%c%d</td>\n"
            "                <td>%d</td>\n"
            "                <td>%d</td>\n"
            "                <td>%d</td>\n"
            "            </tr>\n",
            games[i].game_name,
            date_str,
            get_color_by_score(games[i].old_score), games[i].old_score,
            get_color_by_score(games[i].new_score), games[i].new_score,
            change_class, sign, abs_change,
            games[i].my_rank,
            games[i].pass_during,
            games[i].pass_after);
    }
    
    pos += snprintf(pos, left,
        "        </table>\n"
        "    </div>\n"
        "    <div class=\"foot\">\n"
        "        <p>数据来自 Codeforces API | 生成时间 %s</p>\n"
        "    </div>\n"
        "</div>\n"
        "</body>\n"
        "</html>\n",
        __DATE__);
    
    return html_buf;
}