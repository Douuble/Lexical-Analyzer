// The last GA.cpp: 定义控制台应用程序的入口点。
//

#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<iostream>
#include<map>
using namespace std;

//词法分析程序
/****************************************************************************************/
//全局变量，保留字表，保留字表全为小写
static char ReserveWord[32][20] = {
	"auto", "break", "case", "char", "const", "continue",
	"default", "do", "double", "else", "enum", "then",
	"float", "for", "goto", "if", "int", "long",
	"register", "return", "short", "signed", "sizeof", "static",
	"struct", "switch", "typedef", "union", "unsigned", "void",
	"volatile", "while"
};
//界符运算符表
static char OperatorOrDelimiter[36][10] = {
	"+", "-", "*", "/", "<", "<=", ">", ">=", "=", "==",
	"!=", ";", "(", ")", "^", ",", "\"", "\'", "#", "&",
	"&&", "|", "||", "%", "~", "<<", ">>", "[", "]", "{",
	"}", "\\", ".", "\?", ":", "!"
};
//标识符表，每次识别之后增加
static  char IDNTable[1000][50] = { "" };
/****************************************************************************************/

// 小数
string toReal(char buf[], int n, int b)
{
	double s = 0;
	double base = b;
	int final;
	string st;
	char str[20];
	for (int i = n; buf[i] != '\0'; i++) { //获得小数部分的数值
		if (buf[i] >= '0' && buf[i] <= '9') {
			s = s + (double)((double)(buf[i] - '0') / (double)base);
			base *= b;
		}
	}
	final = (int)(s * 1000000000); //由于返回一个字符串变量，因此将小数部分扩大后再进行除法
	while (final % 10 == 0)
		final /= 10;
	sprintf(str, "%d", final);
	st = str;
	return st;
}


// 八进制
string Num8(char s[]) { //识别 8 进制数
	int i;
	int num = 0;
	char str[20];
	string re;
	for (i = 1; s[i] != '\0'; i++) {
		if (s[i] >= '0' && s[i] <= '7')
			num = (s[i] - '0') + num * 8;
		else if (s[i] == '.') { //发现小数点
			sprintf(str, "%d.", num);
			re = str;
			re += toReal(s, i + 1, 8);
			return re;
		}
	}
	sprintf(str, "%d", num);
	re = str;
	return re;
}

// 十六进制
string Num16(char s[]) { //识别 16 进制数
	int i;
	int num = 0;
	char str[20];
	string re;
	for (i = 2; s[i] != '\0'; i++) {
		if ((s[i] >= '0' && s[i] <= '9') ||
			(s[i] >= 'a' && s[i] <= 'f') ||
			(s[i] >= 'A' && s[i] <= 'F')) {
			if (s[i] >= '0' && s[i] <= '9') {
				num = (s[i] - '0') + num * 16;
			}
			else if (s[i] >= 'a' && s[i] <= 'f') {
				num = (s[i] - 'a' + 10) + num * 16;
			}
			else if (s[i] >= 'A' && s[i] <= 'F') {
				num = (s[i] - 'A' + 10) + num * 16;
			}
		}
		else if (s[i] == '.') { //发现小数点
			sprintf(str, "%d.", num);
			re = str;
			re += toReal(s, i + 1, 16);
			return re;
		}
	}
	sprintf(str, "%d", num);
	re = str;
	return re;
}


// 十进制数字
bool IsDigit(char digit)
{
	if (digit >= '0' && digit <= '9')
	{
		return true;
	}
	else
	{
		return false;
	}
}


// 保留字
int SearchReserveWord(char ReserveWord[][20], char s[])
{
	for (int i = 0; i < 32; i++)
	{
		if (strcmp(ReserveWord[i], s) == 0)
		{//若成功查找，则返回种别码
			return i + 1;//返回种别码
		}
	}
	return -1;//否则返回-1，代表查找不成功，即为标识符
}

// 字母
bool IsLetter(char letter)
{
	if (letter >= 'a' && letter <= 'z' || letter >= 'A' && letter <= 'Z' || letter == '_')
	{
		return true;
	}
	else
	{
		return false;
	}
}




/********************编译预处理，取出无用的字符和注释**********************/
void FilterCode(char r[], int curStr)
{
	char tempString[10000];
	int count = 0;
	for (int i = 0; i <= curStr; i++)
	{
		//if(r[i] == ';') r[i] = ' ';
		if (r[i] == '/' && r[i + 1] == '/')
		{//若为单行注释“//”,则去除注释后面的东西，直至遇到回车换行
			while (r[i] != '\n')
			{
				i++;//向后扫描
			}
		}
		if (r[i] == '/' && r[i + 1] == '*')
		{//若为多行注释“/* 。。。*/”则去除该内容
			i += 2;
			while (r[i] != '*' || r[i + 1] != '/')
			{
				i++;//继续扫描
				if (r[i] == '$')
				{
					printf("注释出错，没有找到 */，程序结束！！！\n");
					exit(0);
				}
			}
			i += 2;//跨过“*/”
		}
		if (r[i] != '\n' && r[i] != '\t' && r[i] != '\v' && r[i] != '\r')
		{//若出现无用字符，则过滤；否则加载
			tempString[count++] = r[i];
		}
	}
	tempString[count] = '\0';
	strcpy(r, tempString);//产生净化之后的源程序
}
/********************编译预处理，取出无用的字符和注释**********************/


/****************************分析子程序，算法核心***********************/
void Scan(int& syn, char OriginalCode[], char PendingStr[], int& curStr)
{//根据DFA的状态转换图设计 
 //syn是分析结果,OriginalCode是整个需要分析的代码,PendingStr每次进行分析的字符串,curStr第二个参数
	int i, count = 0;//count用来做PendingStr[]的下标，收集有用字符
	char ch;//作为判断使用
	ch = OriginalCode[curStr];
	while (ch == ' ')
	{//过滤空格，防止程序因识别不了空格而结束
		curStr++;
		ch = OriginalCode[curStr];
	}
	for (i = 0; i < 20; i++)
	{//每次收集前先清零
		PendingStr[i] = '\0';
	}
	if (IsLetter(OriginalCode[curStr]))
	{//开头为字母
		PendingStr[count++] = OriginalCode[curStr];//收集
		curStr++;//下移
		while (IsLetter(OriginalCode[curStr]) || IsDigit(OriginalCode[curStr]))
		{//后跟字母或数字
			PendingStr[count++] = OriginalCode[curStr];//收集
			curStr++;//下移
		}//多读了一个字符既是下次将要开始的指针位置
		PendingStr[count] = '\0';
		syn = SearchReserveWord(ReserveWord, PendingStr);//查表找到种别码
		if (syn == -1)
		{//若不是保留字则是标识符
			syn = 100;//标识符种别码
		}
		return;
	}
	else if (IsDigit(OriginalCode[curStr]))
	{//首字符为数字
		char first = OriginalCode[curStr];
		char second;
		bool get = false;
		bool sign = false;
		while (IsDigit(OriginalCode[curStr]) ||
			OriginalCode[curStr] == 'x' ||
			OriginalCode[curStr] == 'X' ||
			OriginalCode[curStr] == '.' ||
			(OriginalCode[curStr] >= 'A' && OriginalCode[curStr] <= 'F') ||
			(OriginalCode[curStr] >= 'a' && OriginalCode[curStr] <= 'f')) {//后跟数字或者X与x,以及小数点
			if (OriginalCode[curStr] == '.') sign = true;
			PendingStr[count++] = OriginalCode[curStr];//收集
			curStr++;
			if (!get) {
				second = OriginalCode[curStr];
				get = true;
			}
		}//多读了一个字符既是下次将要开始的指针位置
		PendingStr[count] = '\0';
		if (sign) {
			if (first >= '1' && first <= '9') {
				syn = 97;
			}
			else if (first == '0') {
				if (second >= '0' && second <= '7')
					syn = 98;
				else if (second == 'x' || second == 'X')
					syn = 99;
				else
					syn = 97;
			}
		}
		else {
			if (first >= '1' && first <= '9') {
				syn = 94;
			}
			else if (first == '0') {
				if (second >= '0' && second <= '7')
					syn = 95;
				else if (second == 'x' || second == 'X')
					syn = 96;
				else
					syn = 94;
			}
		}

	}
	else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == ';' || ch == '(' || ch == ')' || ch == '^'
		|| ch == ',' || ch == '\"' || ch == '\'' || ch == '~' || ch == '#' || ch == '%' || ch == '['
		|| ch == ']' || ch == '{' || ch == '}' || ch == '\\' || ch == '.' || ch == '\?' || ch == ':')
	{//若为运算符或者界符，查表得到结果
		PendingStr[0] = OriginalCode[curStr];
		PendingStr[1] = '\0';//形成单字符串
		for (i = 0; i < 36; i++)
		{//查运算符界符表
			if (strcmp(PendingStr, OperatorOrDelimiter[i]) == 0)
			{
				syn = 33 + i;//获得种别码，使用了一点技巧，使之呈线性映射
				break;//查到即退出
			}
		}
		curStr++;//指针下移，为下一扫描做准备
		return;
	}
	else  if (OriginalCode[curStr] == '<')
	{//<,<=,<<
		curStr++;//后移，超前搜索
		if (OriginalCode[curStr] == '=')
		{
			syn = 38;
		}
		else if (OriginalCode[curStr] == '<')
		{//左移
			curStr--;
			syn = 58;
		}
		else
		{
			curStr--;
			syn = 37;
		}
		curStr++;//指针下移
		return;
	}
	else  if (OriginalCode[curStr] == '>')
	{//>,>=,>>
		curStr++;
		if (OriginalCode[curStr] == '=')
		{
			syn = 40;
		}
		else if (OriginalCode[curStr] == '>')
		{
			syn = 59;
		}
		else
		{
			curStr--;
			syn = 39;
		}
		curStr++;
		return;
	}
	else  if (OriginalCode[curStr] == '=')
	{//=.==
		curStr++;
		if (OriginalCode[curStr] == '=')
		{
			syn = 42;
		}
		else
		{
			curStr--;
			syn = 41;
		}
		curStr++;
		return;
	}
	else  if (OriginalCode[curStr] == '!')
	{//!,!=
		curStr++;
		if (OriginalCode[curStr] == '=')
		{
			syn = 43;
		}
		else
		{
			syn = 68;
			curStr--;
		}
		curStr++;
		return;
	}
	else  if (OriginalCode[curStr] == '&')
	{//&,&&
		curStr++;
		if (OriginalCode[curStr] == '&')
		{
			syn = 53;
		}
		else
		{
			curStr--;
			syn = 52;
		}
		curStr++;
		return;
	}
	else  if (OriginalCode[curStr] == '|')
	{//|,||
		curStr++;
		if (OriginalCode[curStr] == '|')
		{
			syn = 55;
		}
		else
		{
			curStr--;
			syn = 54;
		}
		curStr++;
		return;
	}
	else  if (OriginalCode[curStr] == '$')
	{//结束符
		syn = 0;//种别码为0
	}
	else
	{//不能被以上词法分析识别，则出错。
		printf("error：there is no exist %c \n", ch);
		exit(0);
	}
}

// 语法分析
char ech[100][10];
char echattr[100][10];
char Pplace[10], PPplace[10], Fplace[10], Eplace[10], Tplace[10], TTplace[10], EEplace[10],
E1place[10], E2place[10], idplace[10], Splace[10], Snext[10], Ctrue[10], Cfalse[10], Sbegin[10];
char S1next[10], S2next[10];
char Pcode[50], PPcode[50], Fcode[50], Ecode[50], Tcode[50], TTcode[50], EEcode[50], E1code[50], E2code[50], idcode[50], Scode[50], Ccode[50];
int label = 0;
int flag = 0;
int number = 0;
int now;
char attr[10];
int temp = 1;
bool oneflag = true;
char output[100000] = { 0 };   //存储三地址代码结果
void Sclear();
void Allclear();
void init(char* ch);
//分类
int P();
int PP();
int S();
int SS();
int E();
int C();
int CC();
int T();
int TT();
int EE();
int F();
void newtemp(char* ch);
void newlabel(char* ch);
int main()
{
	int counter = 0;//循环次数
	char OriginalCode[10000];
	char PendingStr[20] = { 0 };
	FILE* fp, * fp1;
	// fp打开源文件，fp1存储新生成的文件
	if ((fp = fopen("Prepared_Code.txt", "r")) == NULL)
	{
		cout << "can not open this file";
		exit(0);
	}
	if ((fp1 = fopen("Compiled_Code.txt", "w+")) == NULL)
	{
		cout << "can not open this file";
		exit(0);
	}//提交，test  在测试

	fscanf(fp, "%d", &counter);
	for (int j = 0; j < counter; j++) {
		int syn = -1, i;//初始化
		int curStr = 0;//源程序指针
		Allclear();
		OriginalCode[curStr] = fgetc(fp);
		while (!feof(fp))
		{
			curStr++;
			OriginalCode[curStr] = fgetc(fp);
			if (OriginalCode[curStr] == '$')break;
		}

		OriginalCode[++curStr] = '\0';

		printf("\n==========第%d次分析==========\n", j + 1);
		fprintf(fp1, "\n==========第%d次分析==========\n", j + 1);


		cout << endl << "源程序为:" << endl;
		cout << OriginalCode << endl;
		fprintf(fp1, "\n源程序为\n%s", OriginalCode);
		//对源程序进行过滤
		FilterCode(OriginalCode, curStr);
		cout << endl << "过滤之后的程序:" << endl;
		cout << OriginalCode << endl;
		fprintf(fp1, "\n过滤之后的程序:\n%s", OriginalCode);


		cout << endl << "==========词法分析器==========" << endl;
		fprintf(fp1, "\n==========词法分析器==========\n");
		cout << endl << "===========识别结果===========" << endl;
		fprintf(fp1, "===========识别结果===========\n");

		curStr = 0;//从头开始读
				   //首先，对源程序进行原有的词法分析操作，使之成为待分析字符串
		int ANAcounter = 0;
		while (syn != 0)
		{
			//启动扫描
			Scan(syn, OriginalCode, PendingStr, curStr);
			if (syn == 100)
			{//标识符
				for (i = 0; i < 1000; i++)
				{//插入标识符表中
					if (strcmp(IDNTable[i], PendingStr) == 0)
					{//已在表中
						break;
					}
					if (strcmp(IDNTable[i], "") == 0)
					{//查找空间
						strcpy(IDNTable[i], PendingStr);
						break;
					}
				}
				strcpy(ech[ANAcounter], "i\0");
				strcpy(echattr[ANAcounter], PendingStr);
				printf("IDN\t % s\n", echattr[ANAcounter]);
				fprintf(fp1, "IDN\t%s\n", echattr[ANAcounter]);
				ANAcounter++;
			}
			else if (syn >= 1 && syn <= 32)
			{//保留字
				strcpy(ech[ANAcounter], PendingStr);
				strcpy(echattr[ANAcounter], PendingStr);
				printf("_\t % s\n", echattr[ANAcounter]);
				fprintf(fp1, "_\t%s\n", echattr[ANAcounter]);
				ANAcounter++;
			}
			else if (syn == 94)
			{//10进制整数
				strcpy(ech[ANAcounter], "i\0");
				strcpy(echattr[ANAcounter], PendingStr);
				printf("INT10\t % s\n", echattr[ANAcounter]);
				fprintf(fp1, "INT10\t%s\n", echattr[ANAcounter]);
				ANAcounter++;
			}
			else if (syn == 95)
			{//8进制整数
				string temp = Num8(PendingStr);
				strcpy(PendingStr, temp.c_str());
				strcpy(ech[ANAcounter], "i\0");
				strcpy(echattr[ANAcounter], PendingStr);
				printf("INT8\t % s\n", echattr[ANAcounter]);
				fprintf(fp1, "INT8\t%s\n", echattr[ANAcounter]);
				ANAcounter++;
			}
			else if (syn == 96)
			{//16进制整数
				string temp = Num16(PendingStr);
				strcpy(PendingStr, temp.c_str());
				strcpy(ech[ANAcounter], "i\0");
				strcpy(echattr[ANAcounter], PendingStr);
				printf("INT16\t % s\n", echattr[ANAcounter]);
				fprintf(fp1, "INT16\t%s\n", echattr[ANAcounter]);
				ANAcounter++;
			}
			else if (syn == 97)
			{//10进制浮点
				strcpy(ech[ANAcounter], "i\0");
				strcpy(echattr[ANAcounter], PendingStr);		
				printf("FLOAT10\t%s\n", echattr[ANAcounter]);
				fprintf(fp1, "FLOAT10\t%s\n", echattr[ANAcounter]);
				ANAcounter++;
			}
			else if (syn == 98)
			{//8进制浮点
				string temp = Num8(PendingStr);
				strcpy(PendingStr, temp.c_str());
				strcpy(ech[ANAcounter], "i\0");
				strcpy(echattr[ANAcounter], PendingStr);
				printf("FLOAT8\t%s\n", echattr[ANAcounter]);
				fprintf(fp1, "FLOAT8\t%s\n", echattr[ANAcounter]);
				ANAcounter++;
			}
			else if (syn == 99)
			{//16进制浮点
				string temp = Num16(PendingStr);
				strcpy(PendingStr, temp.c_str());
				strcpy(ech[ANAcounter], "i\0");
				strcpy(echattr[ANAcounter], PendingStr);
				printf("FLOAT16\t%s\n", echattr[ANAcounter]);
				fprintf(fp1, "FLOAT16\t%s\n", echattr[ANAcounter]);
				ANAcounter++;
			}
			else if (syn >= 33 && syn <= 68)
			{
				strcpy(ech[ANAcounter], OperatorOrDelimiter[syn - 33]);
				strcpy(echattr[ANAcounter], OperatorOrDelimiter[syn - 33]);
				printf("_\t%s\n", echattr[ANAcounter]);
				fprintf(fp1, "_\t%s\n", echattr[ANAcounter]);
				ANAcounter++;
			}
		}
		strcpy(ech[ANAcounter], "$\0");
		cout << endl << "===========识别结束===========" << endl;
		fprintf(fp1, "===========识别结束===========\n");

		cout << endl << "==========语法分析器==========" << endl;
		fprintf(fp1, "\n==========语法分析器==========\n");
		
		
		int flag = 0;
		flag = P();

		fprintf(fp1, "%s", output);



		number = ANAcounter + 1;
		now = 0;

		cout << endl << "语法分析结果：" << endl;
		fprintf(fp1, "语法分析结果：\n");
		if (flag == 1)
		{
			printf("\nThe result is true!!!\n");
			fprintf(fp1, "\nThe result is true!!!\n");
		}
		else
		{
			printf("\nThe result is false!!\n");
			fprintf(fp1, "\nThe result is false!!\n");
		}
		flag = 0;


		printf("==========第%d次分析结束==========\n", j + 1);
		fprintf(fp1, "==========第%d次分析结束==========\n", j + 1);
	}
	fclose(fp);
	fclose(fp1);
	return 0;
}
int P()
{
	int s, pp;
	Sclear();
	s = S();
	strcpy(Pplace, Splace);
	strcpy(Pplace, Scode);
	if (s == 0) {
		return 0;
	}
	if (oneflag) {
		printf("\nL0:\t\n");
		strcat(output, "\nL0:\t\n");
	}
	pp = PP();
	if (pp == 0) {
		return 0;
	}
	else {
		return 1;
	}
}

int PP() {
	oneflag = false;
	if (strcmp(ech[now], "$") == 0)
	{
		return 1;
	}
	else
	{
		int p;
		p = P();
		strcpy(PPplace, Pplace);
		strcpy(PPcode, Pcode);
		if (p == 0) {
			return 0;
		}
		else {
			return 1;
		}
	}

}

int S()
{
	int c, s1, ss, e;
	if (strcmp(ech[now], "if") == 0)
	{
		if (label == 0)
		{
			newlabel(Snext);
			newlabel(Sbegin);
			printf("\n%s:\n", Sbegin);
			string ss = "\n" + (string)Sbegin + ":\n";
			char outtemp[100];
			strcpy(outtemp, ss.c_str());
			strcat(output, outtemp);
		}
		else
		{
			strcpy(Sbegin, Ctrue);
		}

		newlabel(Ctrue);
		strcpy(Cfalse, Snext);



		now++;
		c = C();
		if (c == 0)
		{
			return 0;
		}
		if (strcmp(ech[now], "then") == 0)
		{
			now++;
			if (strcmp(ech[now], "i") == 0 || strcmp(ech[now], "if") == 0)
			{
				printf("\n%s:\n", Ctrue);
				string s = "\n" + (string)Ctrue + ":\n";
				char outtemp[100];
				strcpy(outtemp, s.c_str());
				strcat(output, outtemp);
			}
			s1 = S();
			if (s1 == 0)
			{
				return 0;
			}
			ss = SS();
			if (ss == 0)
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
	}
	if (strcmp(ech[now], "i") == 0)
	{
		strcpy(idplace, echattr[now]);
		now++;
		if (strcmp(ech[now], "=") == 0)
		{
			now++;
			e = E();
			strcpy(Scode, Ecode);                                 /*aaaaaaaaaaaaaaa*/
			printf("   %s=%s\n", idplace, Eplace);
			string s = "   " + (string)idplace + "=" + (string)Eplace + "\n";
			char outtemp[100];
			strcpy(outtemp, s.c_str());
			strcat(output, outtemp);
			if (strcmp(Sbegin, "") != 0) {
				printf("   goto %s\n", Sbegin);
				string s = "   goto " + (string)Sbegin + "\n";
				char outtemp[100];
				strcpy(outtemp, s.c_str());
				strcat(output, outtemp);
			}

			if (e == 0)
			{
				return 0;
			}
			else
			{
				//return 1;
				if (strcmp(ech[now], ";") == 0) {
					now++;
					return 1;
				}
				else {
					now++;
					return 0;
				}
			}
		}
		now++;

		return 0;
	}
	if (strcmp(ech[now], "while") == 0)
	{
		now++;

		if (label == 0)
		{
			newlabel(Snext);
			newlabel(Sbegin);
		}
		else
		{
			strcpy(Sbegin, Ctrue);
		}
		newlabel(Ctrue);
		strcpy(Cfalse, Snext);
		strcpy(S1next, Sbegin);

		printf("\n%s:\n", Sbegin);
		string s = "\n" + (string)Sbegin + ":\n";
		char outtemp[100];
		strcpy(outtemp, s.c_str());
		strcat(output, outtemp);
		c = C();



		if (c == 0)
		{
			return 0;
		}
		if (strcmp(ech[now], "do") == 0)
		{
			now++;

			if (strcmp(ech[now], "while") != 0)
			{
				printf("\n%s:\n", Ctrue);
				string s = "\n" + (string)Ctrue + ":\n";
				char outtemp[100];
				strcpy(outtemp, s.c_str());
				strcat(output, outtemp);
			}
			strcpy(Snext, S1next);
			s1 = S();



			if (s1 == 0)
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
	}
	return 0;
}

int SS()
{
	int s;
	if (strcmp(ech[now], "else") == 0)
	{
		now++;
		printf("\n%s:\n", Snext);
		string ss = "\n" + (string)Snext + ":\n";
		char outtemp[100];
		strcpy(outtemp, ss.c_str());
		strcat(output, outtemp);
		s = S();
		if (s == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	if (strcmp(ech[now], "$") == 0 || strcmp(ech[now], "i") == 0 || strcmp(ech[now], "if") == 0 || strcmp(ech[now], "while") == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int C()
{
	int cc, e;
	e = E();
	strcpy(E1place, Eplace);           /*aaaaaaaaaaaaaa*/
	strcpy(E1code, Ecode);
	if (e == 0)
	{
		return 0;
	}
	cc = CC();
	if (cc == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int CC()
{
	int e;
	if (strcmp(ech[now], ">") == 0)
	{
		now++;
		e = E();
		strcpy(E2place, Eplace);           /*aaaaaaaaaaaaaa*/
		strcpy(E2code, Ecode);
		printf("   if %s>%s goto %s \n   goto %s", E1place, E2place, Ctrue, Cfalse);
		string ss = "   if " + (string)E1place + ">" + (string)E2place + " goto " + (string)Ctrue + " \n   goto " + (string)Cfalse;
		char outtemp[100];
		strcpy(outtemp, ss.c_str());
		strcat(output, outtemp);
		if (e == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	if (strcmp(ech[now], "<") == 0)
	{
		now++;
		e = E();
		strcpy(E2place, Eplace);           /*aaaaaaaaaaaaaa*/
		strcpy(E2code, Ecode);
		printf("   if %s<%s goto %s \n   goto %s", E1place, E2place, Ctrue, Cfalse);
		string ss = "   if " + (string)E1place + "<" + (string)E2place + " goto " + (string)Ctrue + " \n   goto " + (string)Cfalse;
		char outtemp[100];
		strcpy(outtemp, ss.c_str());
		strcat(output, outtemp);
		if (e == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	if (strcmp(ech[now], "=") == 0)
	{
		now++;
		e = E();
		strcpy(E2place, Eplace);           /*aaaaaaaaaaaaaa*/
		strcpy(E2code, Ecode);
		printf("   if %s=%s goto %s \n   goto %s", E1place, E2place, Ctrue, Cfalse);
		string ss = "   if " + (string)E1place + "=" + (string)E2place + " goto " + (string)Ctrue + " \n   goto " + (string)Cfalse;
		char outtemp[100];
		strcpy(outtemp, ss.c_str());
		strcat(output, outtemp);
		if (e == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 0;
	}
}

int E()
{
	int t, ee;
	t = T();
	strcpy(Eplace, Tplace);
	strcpy(Ecode, Tcode);
	if (t == 0)
	{
		return 0;
	}
	ee = EE();
	if (ee == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int T()
{
	int f, tt;
	f = F();
	strcpy(Tplace, Fplace);
	strcpy(Tcode, Fcode);
	if (f == 0)
	{
		return 0;
	}
	tt = TT();
	if (tt == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}

}

int TT()
{
	int f, tt;
	if (strcmp(ech[now], "*") == 0)
	{
		strcpy(TTplace, Tplace);
		strcpy(TTcode, Tcode);
		newtemp(Tplace);
		now++;
		f = F();
		strcpy(Tcode, TTcode);
		strcat(Tcode, Fcode);
		printf("   %s=%s*%s\n", Tplace, TTplace, Fplace);
		string ss = "   " + (string)Tplace + "=" + (string)TTplace + "*" + (string)Fplace + "\n";
		char outtemp[100];
		strcpy(outtemp, ss.c_str());
		strcat(output, outtemp);
		if (f == 0)
		{
			return 0;
		}
		tt = TT();
		if (tt == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}

	}
	if (strcmp(ech[now], "/") == 0)
	{
		strcpy(TTplace, Tplace);
		strcpy(TTcode, Tcode);
		newtemp(Tplace);
		now++;
		f = F();
		strcpy(Tcode, TTcode);
		strcat(Tcode, Fcode);
		printf("   %s=%s/%s\n", Tplace, TTplace, Fplace);
		string ss = "   " + (string)Tplace + "=" + (string)TTplace + "/" + (string)Fplace + "\n";
		char outtemp[100];
		strcpy(outtemp, ss.c_str());
		strcat(output, outtemp);
		if (f == 0)
		{
			return 0;
		}
		tt = TT();
		if (tt == 0)
		{
			return 0;
		}
		else
		{

			return 1;
		}
	}
	else
	{
		return 1;
	}
}

int EE()
{
	int t, ee;
	if (strcmp(ech[now], "+") == 0)
	{
		strcpy(EEplace, Eplace);
		strcpy(EEcode, Ecode);
		newtemp(Eplace);
		now++;
		t = T();
		strcpy(Ecode, EEcode);
		strcat(Ecode, Tcode);
		printf("   %s=%s+%s\n", Eplace, EEplace, Tplace);
		string ss = "   " + (string)Eplace + "=" + (string)EEplace + "+" + (string)Tplace + "\n";
		char outtemp[100];
		strcpy(outtemp, ss.c_str());
		strcat(output, outtemp);
		if (t == 0)
		{
			return 0;
		}
		ee = EE();
		if (ee == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	if (strcmp(ech[now], "-") == 0)
	{
		strcpy(EEplace, Eplace);
		strcpy(EEcode, Ecode);
		newtemp(Eplace);
		now++;
		t = T();
		strcpy(Ecode, EEcode);
		strcat(Ecode, Tcode);
		printf("   %s=%s-%s\n", Eplace, EEplace, Tplace);
		string ss = "   " + (string)Eplace + "=" + (string)EEplace + "-" + (string)Tplace + "\n";
		char outtemp[100];
		strcpy(outtemp, ss.c_str());
		strcat(output, outtemp);
		if (t == 0)
		{
			return 0;
		}
		ee = EE();
		if (ee == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
}

int F()
{
	int e = 0;
	if (strcmp(ech[now], "(") == 0)
	{
		now++;
		e = E();
		strcpy(Fplace, Eplace);
		strcpy(Fcode, Ecode);
		if (e == 0)
		{
			return 0;
		}
		if (strcmp(ech[now], ")") == 0)
		{
			now++;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	if (strcmp(ech[now], "i") == 0)
	{
		strcpy(Fplace, echattr[now]);
		strcpy(Fcode, "");
		now++;
		return 1;
	}
	if (strcmp(ech[now], "int8") == 0)
	{
		strcpy(Fplace, echattr[now]);
		strcpy(Fcode, "");
		now++;
		return 1;
	}
	if (strcmp(ech[now], "int10") == 0)
	{
		strcpy(Fplace, echattr[now]);
		strcpy(Fcode, "");
		now++;
		return 1;
	}
	if (strcmp(ech[now], "int16") == 0)
	{
		strcpy(Fplace, echattr[now]);
		strcpy(Fcode, "");
		now++;
		return 1;
	}

	else
	{
		return 0;
	}
}

void Sclear() {
	memset(Pplace, 0, sizeof(Pplace) / sizeof(char)); memset(PPplace, 0, sizeof(PPplace) / sizeof(char));
	memset(Fplace, 0, sizeof(Fplace) / sizeof(char)); memset(Eplace, 0, sizeof(Eplace) / sizeof(char));
	memset(Tplace, 0, sizeof(Tplace) / sizeof(char)); memset(TTplace, 0, sizeof(TTplace) / sizeof(char));
	memset(EEplace, 0, sizeof(EEplace) / sizeof(char)); memset(E1place, 0, sizeof(E1place) / sizeof(char));
	memset(E2place, 0, sizeof(E2place) / sizeof(char)); memset(idplace, 0, sizeof(idplace) / sizeof(char));
	memset(Splace, 0, sizeof(Splace) / sizeof(char)); memset(Snext, 0, sizeof(Snext) / sizeof(char));
	memset(Ctrue, 0, sizeof(Ctrue) / sizeof(char)); memset(Cfalse, 0, sizeof(Cfalse) / sizeof(char));
	memset(Sbegin, 0, sizeof(Sbegin) / sizeof(char)); memset(S1next, 0, sizeof(S1next) / sizeof(char));
	memset(Pcode, 0, sizeof(Pcode) / sizeof(char)); memset(PPcode, 0, sizeof(PPcode) / sizeof(char));
	memset(Fcode, 0, sizeof(Fcode) / sizeof(char)); memset(Ecode, 0, sizeof(Ecode) / sizeof(char));
	memset(Tcode, 0, sizeof(Tcode) / sizeof(char)); memset(TTcode, 0, sizeof(TTcode) / sizeof(char));
	memset(EEcode, 0, sizeof(EEcode) / sizeof(char)); memset(E1code, 0, sizeof(E1code) / sizeof(char));
	memset(E2code, 0, sizeof(E2code) / sizeof(char)); memset(idcode, 0, sizeof(idcode) / sizeof(char));
	memset(Scode, 0, sizeof(Scode) / sizeof(char)); memset(Ccode, 0, sizeof(Ccode) / sizeof(char));
}

void Allclear() {
	memset(Pplace, 0, sizeof(Pplace) / sizeof(char)); memset(PPplace, 0, sizeof(PPplace) / sizeof(char));
	memset(Fplace, 0, sizeof(Fplace) / sizeof(char)); memset(Eplace, 0, sizeof(Eplace) / sizeof(char));
	memset(Tplace, 0, sizeof(Tplace) / sizeof(char)); memset(TTplace, 0, sizeof(TTplace) / sizeof(char));
	memset(EEplace, 0, sizeof(EEplace) / sizeof(char)); memset(E1place, 0, sizeof(E1place) / sizeof(char));
	memset(E2place, 0, sizeof(E2place) / sizeof(char)); memset(idplace, 0, sizeof(idplace) / sizeof(char));
	memset(Splace, 0, sizeof(Splace) / sizeof(char)); memset(Snext, 0, sizeof(Snext) / sizeof(char));
	memset(Ctrue, 0, sizeof(Ctrue) / sizeof(char)); memset(Cfalse, 0, sizeof(Cfalse) / sizeof(char));
	memset(Sbegin, 0, sizeof(Sbegin) / sizeof(char)); memset(S1next, 0, sizeof(S1next) / sizeof(char));
	memset(Pcode, 0, sizeof(Pcode) / sizeof(char)); memset(PPcode, 0, sizeof(PPcode) / sizeof(char));
	memset(Fcode, 0, sizeof(Fcode) / sizeof(char)); memset(Ecode, 0, sizeof(Ecode) / sizeof(char));
	memset(Tcode, 0, sizeof(Tcode) / sizeof(char)); memset(TTcode, 0, sizeof(TTcode) / sizeof(char));
	memset(EEcode, 0, sizeof(EEcode) / sizeof(char)); memset(E1code, 0, sizeof(E1code) / sizeof(char));
	memset(E2code, 0, sizeof(E2code) / sizeof(char)); memset(idcode, 0, sizeof(idcode) / sizeof(char));
	memset(Scode, 0, sizeof(Scode) / sizeof(char)); memset(Ccode, 0, sizeof(Ccode) / sizeof(char));
	memset(ech, 0, sizeof(ech) / sizeof(char)); memset(echattr, 0, sizeof(echattr) / sizeof(char));
	memset(attr, 0, sizeof(attr) / sizeof(char)); memset(output, 0, sizeof(output) / sizeof(char));
	label = flag = number = now;
	temp = 1;
	oneflag = true;
}

void init(char* ch)
{
	char newc[10];
	newc[0] = NULL;
	strcpy(ch, newc);
}

void newtemp(char* ch)
{
	sprintf(attr, "%d", temp);
	strcpy(ch, "t");
	strcat(ch, attr);
	temp++;
}

void newlabel(char* ch)
{
	sprintf(attr, "%d", label);
	strcpy(ch, "L");
	strcat(ch, attr);
	label++;
}