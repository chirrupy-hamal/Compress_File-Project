#include"FileCompressHuffman.h"     
#include<iostream>
using namespace std;

#include<assert.h>

FileCompressHuffMan::FileCompressHuffMan()
{
	_charInfo.resize(256);//ԤԼ�ռ�
	for (size_t i = 0; i < 256; ++i)
	{
		_charInfo[i]._ch = (char)i;
	}
}

void FileCompressHuffMan::CompressFile(const string &strFilePath)
{
	//1����ȡԭ�ļ���ÿ���ַ����ֵĴ���
	FILE *fIn = fopen(strFilePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "�ļ���ʧ��" << endl;
		return;
	}

	UCH *pReadBuff = new UCH[1024];//ÿ�ζ�1K������
	/*
	vector<CharInfo> charInfo(256);//�ļ��ڴ��������ֽ����ķ�ʽ���ڣ�1�ֽ�8����λ����256��״̬��
	for (size_t i = 0; i < 256; ++i)
	{
		charInfo[i]._ch = i;
	}
	*/
	//��vector<CharInfo> charInfoֱ�Ӽӵ�FileCompressHuffMan���У��������ÿ���ַ���HuffMan���롣

	while (1)//��֪���ļ��Ĵ�С��Ҳ�Ͳ�֪�������ٸ�1K�����Ը�����ѭ����
	{
		size_t rdSize = fread(pReadBuff, 1, 1024, fIn);//��������ļ�ĩβ��fread����0��
		if (0 == rdSize)
		{
			break;
		}
		for (size_t i = 0; i < rdSize; ++i)
		{
			_charInfo[pReadBuff[i]]._charCount++;
		}
	}

	//2����ÿ���ַ����ֵĴ���ΪȨֵ����Huffman��
	HuffmanTree<CharInfo> ht;
	ht.CreateHuffmanTree(_charInfo, CharInfo(0));//�ѳ���0�ε��ַ����˵��������յ�HuffMan����û��0���ַ��Ĵ��ڡ�

	//3������Huaffman����ȡÿ���ַ��ı���(��0��1)
	GetHuffmanCode(ht.GetRoot());

    //4������ÿ���ַ��ı������¸�дԭ�ļ�
	FILE *fOut = fopen("1.rar", "wb");//����ʵ�֣�����
	assert(fOut);

	WriteHead(fOut, strFilePath);//��Ҫ��ͷ����Ϣд��ѹ���ļ��У���Ϊ��ѹ����Ҫ�õ���Щ��Ϣ��

	char ch = 0;
	char bitCount = 0;

	fseek(fIn, 0, SEEK_SET);//�ı�ָ��λ�ã����ļ�ָ��ָ����ļ�����ʼλ�á�
	                        //�ڶ��������ǻ��ڵ�����������ƫ������
	while(true)
	{
		size_t rdSize = fread(pReadBuff, 1, 1024, fIn);//��ǰfIn�Ѿ�����һ���ˣ�fIn���ļ�ָ���Ѿ��ڽ�β�ˡ�
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
					fputc(ch, fOut);//һ�����ֽ�д̫���������ٽ�һ��1K��buff����buffд���ļ�������ѹ���ܿ�һ��
					bitCount = 0;
				}
			}
		}
	}
	if (bitCount < 8 && bitCount > 0)//ѹ����������һ���ֽ�8������λ���ܲ���
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

	//��Ҷ�ӽ��
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

		reverse(strCode.begin(), strCode.end());//����
		//��ÿ���ַ��ı��뱣�浽HuffMan���Ķ�Ӧ��㲻�Ǻܺõİ취����Ϊ���õ�ʱ�򻹵�ȥ�����ҡ������㡣
	}
}

//���յ�ѹ���ļ�����������ɣ���ѹ����Ҫ�õ�����Ϣ��ԭ�ļ���׺+�ַ��������+�ַ���ÿ���ַ����ֵĴ�����+ѹ���������
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
			//strCharInfo += _charInfo[i]._charCount;//_itoa������������ת�����ַ���
			_itoa(_charInfo[i]._charCount, szCount, 10);
			strCharInfo += szCount;
			strCharInfo += '\n';
			lineCount++;
		}
	}

	_itoa(lineCount, szCount, 10);
	strHeadInfo += szCount;//�ȷ�ԭ�ļ���׺�����ٷ��ַ���������������ַ���ÿ���ַ����ֵĴ�����
	strHeadInfo += '\n';

	strHeadInfo += strCharInfo;
	fwrite(strHeadInfo.c_str(), 1, strHeadInfo.size(), fOut);
}

void FileCompressHuffMan::UNCompressFile(const std::string &strFilePath)
{
	//���ѹ���ļ��ĺ�׺��ʽ
	string strPostFix = strFilePath.substr(strFilePath.rfind('.'));
	if (".rar" != strPostFix)//��������Ѵ˴���C��ʽ���ַ���תΪstring�����
	{
		cout << "ѹ���ļ��ĸ�ʽ������" << endl;
		return;
	} 

	//��ȡ��ѹ������Ϣ
	FILE *fIn = fopen(strFilePath.c_str(), "rb");
	//�ı��ļ��ĸ�ʽ���ǣ��ڶ��Ļ������⵽-1����Ϊ������β��r���ǰ����ı��ķ�ʽ���ġ�
	//�����Ƹ�ʽ���ļ����п��ܳ���-1�����
	if (nullptr == fIn)
	{
		cout << "ѹ���ļ���ʧ��" << endl;
		return;
	}

	//��ȡԭ�ļ��ĺ�׺
	strPostFix = "";
	GetLine(fIn, strPostFix);

	//��ȡ
	string strContent;
	GetLine(fIn, strContent);
	size_t lineCount = atoi(strContent.c_str());

	//��ȡ�ַ���Ϣ
	for (size_t i = 0; i < lineCount; ++i)
	{
		strContent = "";
		GetLine(fIn, strContent);

		//�����������
		if (strContent.empty())
		{
			strContent += '\n';
			GetLine(fIn, strContent);
		}

		//string�����������ַ�
		_charInfo[(UCH)strContent[0]]._charCount = atoi(strContent.c_str() + 2);//+2����Ϊǰ�����ַ��Ͷ���
	}

	//����Huffman��
	HuffmanTree<CharInfo> ht;
	ht.CreateHuffmanTree(_charInfo, CharInfo(0));

	//��ѹ��
	string  strUNComFile = "2";//ϸ��һ��
	strUNComFile += strPostFix;
	FILE *fOut = fopen(strUNComFile.c_str(), "wb");
	assert(fOut);

	char *pReadBuff = new char[1024];
	HuffmanTreeNode<CharInfo> *pCur = ht.GetRoot();
	char pos = 7;
	size_t fileSize = pCur->_weight._charCount;//δѹ�����ļ���С��
	          //�������ѹ���ļ������һ���ַ�����ֻ��ĳ��������λ��Ч���������λ��Ϊ�ˡ��չ���1�ֽڶ���ӽ�ȥ�ġ�
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

			for (size_t j = 0; j < 8; ++j)//ÿ�ֽ���8����λ
			{
				//
				if (pReadBuff[i] & (1 << pos))//�ó����λ
				{
					pCur = pCur->_pRight;
				}
				else
				{
					pCur = pCur->_pLeft;
				}
				if (nullptr == pCur->_pLeft && nullptr == pCur->_pRight)
				{
					fputc(pCur->_weight._ch, fOut);//Ҳ���Խ����Ż�
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