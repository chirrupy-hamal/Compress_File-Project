#include"FileCompressHuffman.h"     
#include<iostream>
using namespace std;

#include<assert.h>

FileCompressHuffMan::FileCompressHuffMan()
{
	_charInfo.resize(256);//预约空间
	for (size_t i = 0; i < 256; ++i)
	{
		_charInfo[i]._ch = (char)i;
	}
}

void FileCompressHuffMan::CompressFile(const string &strFilePath)
{
	//1、获取原文件中每个字符出现的次数
	FILE *fIn = fopen(strFilePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "文件打开失败" << endl;
		return;
	}

	UCH *pReadBuff = new UCH[1024];//每次读1K的数据
	/*
	vector<CharInfo> charInfo(256);//文件在磁盘中以字节流的方式存在，1字节8比特位，共256种状态。
	for (size_t i = 0; i < 256; ++i)
	{
		charInfo[i]._ch = i;
	}
	*/
	//把vector<CharInfo> charInfo直接加到FileCompressHuffMan类中，方便管理每个字符的HuffMan编码。

	while (1)//不知道文件的大小，也就不知道读多少个1K，所以给成死循环。
	{
		size_t rdSize = fread(pReadBuff, 1, 1024, fIn);//如果读到文件末尾，fread返回0。
		if (0 == rdSize)
		{
			break;
		}
		for (size_t i = 0; i < rdSize; ++i)
		{
			_charInfo[pReadBuff[i]]._charCount++;
		}
	}

	//2、以每个字符出现的次数为权值创建Huffman树
	HuffmanTree<CharInfo> ht;
	ht.CreateHuffmanTree(_charInfo, CharInfo(0));//把出现0次的字符过滤掉，在最终的HuffMan树中没有0次字符的存在。

	//3、根据Huaffman树获取每个字符的编码(左0右1)
	GetHuffmanCode(ht.GetRoot());

    //4、根据每个字符的编码重新改写原文件
	FILE *fOut = fopen("1.rar", "wb");//命名实现？？？
	assert(fOut);

	WriteHead(fOut, strFilePath);//需要把头部信息写进压缩文件中，因为解压缩需要用到这些信息。

	char ch = 0;
	char bitCount = 0;

	fseek(fIn, 0, SEEK_SET);//改变指针位置，将文件指针恢复到文件的起始位置。
	                        //第二个参数是基于第三个参数的偏移量。
	while(true)
	{
		size_t rdSize = fread(pReadBuff, 1, 1024, fIn);//先前fIn已经读过一次了，fIn的文件指针已经在结尾了。
		if (0 == rdSize)
		{
			break;
		}
		for (size_t i = 0; i < rdSize; ++i)
		{
			string &strCode = _charInfo[pReadBuff[i]]._strCode;
			
			for (size_t j = 0; j < strCode.size(); ++j)
			{
				ch <<= 1;
				if ('1' == strCode[j])
				{
					ch |= 1;
				}
				bitCount++;
				if (8 == bitCount)
				{
					fputc(ch, fOut);//一个个字节写太慢，可以再建一个1K的buff，从buff写进文件，这样压缩能快一点
					bitCount = 0;
				}
			}
		}
	}
	if (bitCount < 8 && bitCount > 0)//压缩结果的最后一个字节8个比特位可能不够
	{
		ch <<= (8 - bitCount);
		fputc(ch, fOut);
	}
	
	delete[] pReadBuff;
	fclose(fIn);
	fclose(fOut);
}

void FileCompressHuffMan::GetHuffmanCode(HuffmanTreeNode<CharInfo> *pRoot)
{ 
	if(nullptr == pRoot)
		return;

	//找叶子结点
	GetHuffmanCode(pRoot->_pLeft);
	GetHuffmanCode(pRoot->_pRight);

	if (nullptr == pRoot->_pLeft && nullptr == pRoot->_pRight)
	{
		HuffmanTreeNode<CharInfo> *pCur = pRoot;
		HuffmanTreeNode<CharInfo> *pParent = pCur->_pParent;

		string &strCode = _charInfo[pCur->_weight._ch]._strCode;
		while (nullptr != pParent)
		{
			if (pCur == pParent->_pLeft)
			{
				strCode += '0';
			}
			else
				strCode += '1';

			pCur = pParent;
			pParent = pCur->_pParent;
		}

		reverse(strCode.begin(), strCode.end());//逆置
		//把每个字符的编码保存到HuffMan树的对应结点不是很好的办法，因为你用的时候还得去树中找。不方便。
	}
}

//最终的压缩文件由两部分组成：解压缩需要用到的信息（原文件后缀+字符种类个数+字符及每种字符出现的次数）+压缩后的数据
void FileCompressHuffMan::WriteHead(FILE *fOut, const std::string &strFilePath)
{
	string strHeadInfo;
	strHeadInfo = strFilePath.substr(strFilePath.rfind('.'));
	strHeadInfo += '\n';

	string strCharInfo;
	char szCount[32];
	size_t lineCount = 0;
	for (size_t i = 0; i < 256; ++i)
	{
		if (_charInfo[i]._charCount)
		{
			strCharInfo += _charInfo[i]._ch;
			strCharInfo += ',';
			//strCharInfo += _charInfo[i]._charCount;//_itoa，把整型数据转换成字符串
			_itoa(_charInfo[i]._charCount, szCount, 10);
			strCharInfo += szCount;
			strCharInfo += '\n';
			lineCount++;
		}
	}

	_itoa(lineCount, szCount, 10);
	strHeadInfo += szCount;//先放原文件后缀名，再放字符种类个数，最后放字符及每种字符出现的次数。
	strHeadInfo += '\n';

	strHeadInfo += strCharInfo;
	fwrite(strHeadInfo.c_str(), 1, strHeadInfo.size(), fOut);
}

void FileCompressHuffMan::UNCompressFile(const std::string &strFilePath)
{
	//检测压缩文件的后缀格式
	string strPostFix = strFilePath.substr(strFilePath.rfind('.'));
	if (".rar" != strPostFix)//编译器会把此处的C格式的字符串转为string类对象
	{
		cout << "压缩文件的格式有问题" << endl;
		return;
	} 

	//获取解压缩的信息
	FILE *fIn = fopen(strFilePath.c_str(), "rb");
	//文本文件的格式就是，在读的话如果检测到-1就认为读到结尾，r就是按照文本的方式读的。
	//二进制格式的文件很有可能出现-1的情况
	if (nullptr == fIn)
	{
		cout << "压缩文件打开失败" << endl;
		return;
	}

	//获取原文件的后缀
	strPostFix = "";
	GetLine(fIn, strPostFix);

	//获取
	string strContent;
	GetLine(fIn, strContent);
	size_t lineCount = atoi(strContent.c_str());

	//获取字符信息
	for (size_t i = 0; i < lineCount; ++i)
	{
		strContent = "";
		GetLine(fIn, strContent);

		//解决换行问题
		if (strContent.empty())
		{
			strContent += '\n';
			GetLine(fIn, strContent);
		}

		//string类里面存的是字符
		_charInfo[(UCH)strContent[0]]._charCount = atoi(strContent.c_str() + 2);//+2是因为前面有字符和逗号
	}

	//创建Huffman树
	HuffmanTree<CharInfo> ht;
	ht.CreateHuffmanTree(_charInfo, CharInfo(0));

	//解压缩
	string  strUNComFile = "2";//细化一下
	strUNComFile += strPostFix;
	FILE *fOut = fopen(strUNComFile.c_str(), "wb");
	assert(fOut);

	char *pReadBuff = new char[1024];
	HuffmanTreeNode<CharInfo> *pCur = ht.GetRoot();
	char pos = 7;
	size_t fileSize = pCur->_weight._charCount;//未压缩的文件大小，
	          //用来解决压缩文件的最后一个字符可能只有某几个比特位有效，其余比特位是为了“凑够”1字节而添加进去的。
	while (true)
	{
		size_t rdSize = fread(pReadBuff, 1, 1024, fIn);
		if (0 == rdSize)
		{
			break;
		}
		for (size_t i = 0; i < rdSize; ++i)
		{
			pos = 7;

			for (size_t j = 0; j < 8; ++j)//每字节有8比特位
			{
				//
				if (pReadBuff[i] & (1 << pos))//拿出最高位
				{
					pCur = pCur->_pRight;
				}
				else
				{
					pCur = pCur->_pLeft;
				}
				if (nullptr == pCur->_pLeft && nullptr == pCur->_pRight)
				{
					fputc(pCur->_weight._ch, fOut);//也可以进行优化
					pCur = ht.GetRoot();
					fileSize--;
					if (0 == fileSize)
					{
						break;
					}
				}
				pos--;
			}
		}
	}

	delete[] pReadBuff;
	fclose(fIn);
	fclose(fOut);
}

void FileCompressHuffMan::GetLine(FILE *fIn, std::string &strContent)
{
	while (!feof(fIn))//EOF -1
	{
		char ch = fgetc(fIn);
		if ('\n' == ch)
		{
			return;
		}
		strContent += ch;
	}
}