/*======================================
�����ͱ�ŵĻ���ת��
======================================*/
#include <iostream>
#include <string>
using namespace std;

extern int turn(const char& c)//�����������ת��Ϊ��ʾ��С�ı�ţ�0~14��
{
	switch (c)
	{
	case 'x'://С��
		return 13;
	case 'd'://����
		return 14;
	case 'j':
		return 8;
	case 'q':
		return 9;
	case 'k':
		return 10;
	case 'a':
		return 11;
	case '2':
		return 12;
	case 't':
		return 7;
	default:
		return c - '3';
	}
}

extern string turn_back(int ord)//���Ʊ��ת��������
{
	string s;
	switch (ord)
	{
	case 14:
		s = "JOKER";
		break;
	case 13:
		s = "joker";
		break;
	case 12:
		s = "2";
		break;
	case 11:
		s = "A";
		break;
	case 10:
		s = "K";
		break;
	case 9:
		s = "Q";
		break;
	case 8:
		s = "J";
		break;
	default:
		s = to_string(ord + 3);
	}
	return s;
}