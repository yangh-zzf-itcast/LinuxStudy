/*************************************************************************
    > File Name: 08_parse_express.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Tue 04 Dec 2018 09:19:49 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"cJSON.h"

int main(void)
{
	//08_express_information.json
	//打开文件，先将json文件 读入缓存区中　变成buf 再进行操作
	FILE *fp = fopen("08_express_information.json", "rb");
	fseek(fp, 0, SEEK_END);     //fseek移动文件指针，ftell读取偏移量来读取文件的大小
	int size = ftell(fp) + 1;
	char *buf = (char *)malloc(sizeof(char) * size);
	memset(buf, 0, sizeof(char) * size);

	//将json文件　中的内容全部读取到 buf中
	rewind(fp);    //因为前面将文件指针偏移到文件末尾，这里将文件指针重新指向开头位置
	fread(buf, sizeof(char), size - 1, fp);

	//对内容进行解析
	cJSON *json = cJSON_Parse(buf);

	//获取快递编号
	cJSON *node = NULL;
	if(cJSON_HasObjectItem(json, "nu") == 0){
		printf("not found nu data\n");
		goto done;
	}

	node = cJSON_GetObjectItem(json, "nu");
	if(node->type != cJSON_String){
		printf("nu is not string\n");
		goto done;
	}
	printf("nu: %s\n", node->valuestring);

	//打印快递信息，２种循环方式
	if(cJSON_HasObjectItem(json, "data") == 0){
		printf("data not found\n");
		goto done;
	}

	node = cJSON_GetObjectItem(json, "data");
	if(node->type != cJSON_Array){
		printf("data's type is not array\n");
		goto done;
	}
	int i;
	cJSON *tnode = NULL;

	int length = cJSON_GetArraySize(node);
	for(i = 0;i < length;++i){
		tnode = cJSON_GetArrayItem(node, i);
		if(tnode->type != cJSON_Object){
			printf("tnode's type is not object\n");
			continue;
		}
		printf("-------------------------------\ntime:%s\n", cJSON_GetObjectItem(tnode, "time")->valuestring);
		printf("context:%s\n", cJSON_GetObjectItem(tnode, "context")->valuestring);
		printf("ftime:%s\n", cJSON_GetObjectItem(tnode, "ftime")->valuestring);
	}

done:
	//释放申请空间
	fclose(fp);
	free(buf);
	cJSON_Delete(json);
	return 0;
}

