#include"FileCompressHuffman.h"

int main()
{
	const char *ptr = "���";
	FileCompressHuffMan fc;
	fc.CompressFile("1.txt");
	fc.UNCompressFile("1.rar");

	system("pause");
	return 0;
}
//��չ
//1��ѹ��������
//2��ѹ������
//3��ѹ���ļ���
//4�����߳�ѹ��