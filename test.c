#include <stdio.h>
#include <curl/curl.h>

int main() {
    printf("Codeforces Analyzer - 环境测试\n");
    printf("libcurl版本: %s\n", curl_version());
    return 0;
}