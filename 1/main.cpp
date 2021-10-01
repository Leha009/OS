#include <iostream>
#include "task1.h"
#include "task2.h"

using namespace std;

int main()
{
    system("chcp 1251");
    int iMenuItem;
    do
    {
        system("cls");
        cout << "1. Task one\n";
        cout << "2. Task two: copy file\n";
        cout << "0. Exit\n";
        cin >> iMenuItem;
        if(iMenuItem < 0 || iMenuItem > 2)
        {
            cout << "Wrong menu item selected!\n";
            system("pause");
        }
        else
        {
            if(iMenuItem == 1)
            {
                Task1Run();
            }   
            else if(iMenuItem == 2)
            {
                Task2Run();
            }
        }
    } while(iMenuItem != 0);
    return 0;
}