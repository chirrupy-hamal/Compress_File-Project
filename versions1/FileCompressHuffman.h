#pragma once
#include<string>
//#include<vector>"HuffmanTree.hpp"ͷ�ļ����Ѿ�����<vector>
#include"HuffmanTree.hpp"

struct CharInfo//ÿһ���ַ�����Ϣ
{
	CharInfo(size_t charCount = 0)
		: _charCount(charCount)
	{}

	CharInfo operator+(const CharInfo& info)//����HuffMan����ʱ���õ�+
	{
		return CharInfo(_charCount + info._charCount);//��������
	}

	bool operator>(const CharInfo& info)//�º����õ�>
	{
		return _charCount > info._charCount;
	}

	bool operator!=(const CharInfo& info)const//����HuffMan����ʱ���õ�!=
	{
		return _charCount != info._charCount;
	}

	char _ch;
	size_t _charCount;
	std::string _strCode;//ÿ���ַ���Huffman����
};

class FileCompressHuffMan
{
public:
	FileCompressHuffMan();
	void CompressFile(const std::string &strFilePath);
	void UNCompressFile(const std::string &strFilePath);
	void WriteHead(FILE *fOut, const std::string &strFilePath);//��ѹ����Ҫ�õ���ͷ����Ϣ
	void GetLine(FILE *fIn, std::string &strContent);

private:
	void GetHuffmanCode(HuffmanTreeNode<CharInfo> *pRoot);
private:
	std::vector<CharInfo> _charInfo;//���ڱ���ÿ���ַ���HuffMan����
};