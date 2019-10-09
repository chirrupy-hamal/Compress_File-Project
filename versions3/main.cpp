#include"FileCompressHuffman.h"

int main()
{
	const char *ptr = "你好";
	FileCompressHuffMan fc;
	fc.CompressFile("1.txt");
	fc.UNCompressFile("1.rar");

	system("pause");
	return 0;
}
//扩展
//1、压缩进度条
//2、压缩比率
//3、压缩文件夹
//4、多线程压缩