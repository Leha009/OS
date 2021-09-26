#include <windows.h>
#include <iostream>

using namespace std;

#define BUFSIZE 512

DWORD _LogicalDrives;

void PrintDrives();
void PrintDriveInfo(LPCSTR);

int main()
{
    PrintDrives();
    string str = "C:\\";
    PrintDriveInfo(str.c_str());
    return 0;
}

void PrintDrives()
{
    //https://tinyurl.com/4rndwjw8
    _LogicalDrives = GetLogicalDrives();    //Получаем битовую маску дисков
    int driveBits = 0;          //Понадобится, чтобы выводить вместе с диском число, соответвующее его месту в битовой маске
    TCHAR drives[BUFSIZE];      //Сюда мы запишем все имеющиеся у нас диски
    TCHAR buff[2] = TEXT(":");  /*TEXT = https://tinyurl.com/2h44tvmy. Нужно, чтобы отображались только буквы, без :\ */
    TCHAR* p = drives;          //Благодаря этому указателю мы будем бегать по всем возможным дискам (буквам дисков)
    GetLogicalDriveStrings(BUFSIZE-1, drives);  //BUFSIZE-1, т.к. не учитывается нуль-терминатор
    cout << "Drives available:\n";
    do
    {
        for(; driveBits < 32 && (_LogicalDrives >> driveBits & 1) != 1; driveBits++);
            //Сравните эту строку с верхней. Знаете, в чем отличие? В том, что не работает нижнее как надо (:
        //for(; driveBits < 32 && _LogicalDrives >> driveBits & 1 != 1; driveBits++);   
        *buff = *p;
        cout << buff << '(' << (1 << driveBits++) << ") ";  //Выводим букву диска и число, соответвующее его месту в битовой маске
        while(*p++);    //Идем до следующей буквы
    } while(*p);
    cout << '\n';
}

void PrintDriveInfo(LPCSTR str)
{
    cout << "Drive " << str << " type is " << GetDriveTypeA(str) << '\n';
}