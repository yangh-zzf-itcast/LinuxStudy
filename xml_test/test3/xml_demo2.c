/*************************************************************************
    > File Name: xml_demo2.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Sun 02 Dec 2018 05:58:06 PM CST
 ************************************************************************/

#include<stdio.h>
#include"mxml.h"

int main(void)
{
	FILE *fp = fopen("02_index.html","r");
	//将磁盘上xml格式的文件，加载到内存中，保存在mxml_node_t类型的　结构体指针中
	mxml_node_t *xml = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);

	//寻找节点
    mxml_node_t *title = mxmlFindElement(xml, xml, "title", NULL, NULL, MXML_DESCEND);
	
	printf(" %s\n", mxmlElementGetAttr(xml, "version"));

	//获得节点文本
	printf("title text: %s\n", mxmlGetText(title, NULL));

	mxml_node_t *p = mxmlFindElement(xml, xml, "p", "style", NULL, MXML_DESCEND);
	if(p == NULL){
		printf("p not find\n");
	}else{
		printf("p style: %s\n", mxmlElementGetAttr(p, "style"));
	}

	printf("p text: %s\n", mxmlGetText(p, NULL));

	//释放内存空间
	mxmlDelete(xml);

	fclose(fp);

	return 0;
}
