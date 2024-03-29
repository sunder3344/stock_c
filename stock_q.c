#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "util.c"
#include <curl/curl.h>

char * res;

size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
	//printf("ptr = %s\n", ptr);
	//printf("userdata = %s\n", userdata);
	//strcpy(res, ptr);
	res = ptr;
	return size * nmemb;
}

int main(int argc, char *argv[]) {
	//read stock code from config
	FILE *fp = fopen("config.ini", "r");
	if (NULL == fp) {
		perror("Open config file failed!");
	}
	
	//输出标题
	printf("%-15s|%-15s|%-15s|%-15s|%-15s|%-20s|%-20s|%-20s|%-20s|\n", "名称", "当前", "涨幅(%)", "昨收", "今开", "当日最高", "当日最低", "成交数(手)", "成交金额(万元)");

	char line[10];
	char * stockcode;
	stockcode = (char *)malloc(sizeof(char) * 10);
	while (!feof(fp)) {
		memset(line, 0, sizeof(line));
		if (fgets(line, sizeof(line), fp) != NULL) {
			int len = strlen(line);
			//char * stockcode;
			stockcode = (char *)realloc(stockcode, sizeof(char) * (len - 1));
			strncpy(stockcode, line, len - 1);
			
			CURL * curl = curl_easy_init();
			CURLcode code;
			char url[100] = "http://qt.gtimg.cn/q=";
			strncat(url, stockcode, len - 1);

			char * flag = "\"";
			int pos;
			struct curl_slist * head = NULL;
			curl_slist_append(head, "Contenl-Type:text/html;charset=GBK");
            curl_slist_append(head, "Content-Encoding: gzip, deflate, br");
            curl_slist_append(head, "Connection: keep-alive");
            curl_slist_append(head, "Host:qt.gtimg.cn");
            curl_slist_append(head, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
            curl_slist_append(head, "User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/97.0.4692.99 Safari/537.36");
            

			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
			curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);
			code = curl_easy_perform(curl);
			curl_slist_free_all(head);
			curl_easy_cleanup(curl);
			/*int strLen = strlen(res) - 24;
			char str[strLen+1];
			strncpy(str, res + 21, strLen);*/

			pos = strcspn(res, flag);
			char * res2 = res + pos + 1;
			pos = strcspn(res2, flag);
			char str[pos];
			strncpy(str, res2, pos);
			//printf("%s\n", str);
			
			int len2 = strlen(str);
			char out[len2 + 1];
			int rc = g2u(str, strlen(str), out, len2);
			//printf("%s\n", out);

			char seperator = '~';
			char * data[49];
			explode(out, seperator, data);
			int i;
			/*for (i = 0; i < 49; i++) {
				printf("data[%d] = %s\n", i, data[i]);
			}*/
			double init_price = atof(data[5]);
			double yesterday_price = atof(data[4]);
			double current_price = atof(data[3]);
			double top_price = atof(data[33]);
			double end_price = atof(data[34]);
			long deal_num = atol(data[36]);
			double deal_amount = atof(data[37]);

			double surplus = current_price - yesterday_price;
			double rate = surplus * 100 / yesterday_price;
			deal_num = deal_num / 100;
			deal_amount = deal_amount / 10000;
			
			if (strlen(data[1]) == 12) {	
				printf("%-17s|%-13.2f|%-13.2f|%-13.2f|%-13.2f|%-16.2f|%-16.2f|%-16d|%-14.2f|\n", data[1], 
							current_price, rate, yesterday_price, init_price, top_price, end_price, deal_num, deal_amount);
			} else if (strlen(data[1]) == 9) {
				printf("%-16s|%-13.2f|%-13.2f|%-13.2f|%-13.2f|%-16.2f|%-16.2f|%-16d|%-14.2f|\n", data[1],
							current_price, rate, yesterday_price, init_price, top_price, end_price, deal_num, deal_amount);
			} else {
				printf("%-17s|%-13.2f|%-13.2f|%-13.2f|%-13.2f|%-16.2f|%-16.2f|%-16d|%-14.2f|\n", data[1],
							current_price, rate, yesterday_price, init_price, top_price, end_price, deal_num, deal_amount);
            }
			//free(stockcode);
            stockcode = NULL;
		}
	}
    free(stockcode);
	fclose(fp);
	return 0;
}
