/*======================================
牌名和编号的互相转换
======================================*/
#include <iostream>
#include <string>
using namespace std;

extern int turn(const char& c)//把输入的牌名转化为表示大小的编号（0~14）
{
	switch (c)
	{
	case 'x'://小王
		return 13;
	case 'd'://大王
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

extern string turn_back(int ord)//把牌编号转换回牌名
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