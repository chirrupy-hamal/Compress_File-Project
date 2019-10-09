#pragma once
#include<string>
//#include<vector>"HuffmanTree.hpp"头文件中已经包含<vector>
#include"HuffmanTree.hpp"

struct CharInfo//每一个字符的信息
{
	CharInfo(size_t charCount = 0)
		: _charCount(charCount)
	{}

	CharInfo operator+(const CharInfo& info)//构建HuffMan树的时候用到+
	{
		return CharInfo(_charCount + info._charCount);//无名对象
	}

	bool operator>(const CharInfo& info)//仿函数用到>
	{
		return _charCount > info._charCount;
	}

	bool operator!=(const CharInfo& info)const//构建HuffMan树的时候用到!=
	{
		return _charCount != info._charCount;
	}

	char _ch;
	size_t _charCount;
	std::string _strCode;//每个字符的Huffman编码
};

class FileCompressHuffMan
{
public:
	FileCompressHuffMan();
	void CompressFile(const std::string &strFilePath);
	void UNCompressFile(const std::string &strFilePath);
	void WriteHead(FILE *fOut, const std::string &strFilePath);//解压缩需要用到的头部信息
	void GetLine(FILE *fIn, std::string &strContent);

private:
	void GetHuffmanCode(HuffmanTreeNode<CharInfo> *pRoot);
private:
	std::vector<CharInfo> _charInfo;//便于保存每个字符的HuffMan编码
};