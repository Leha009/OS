#include <iostream>
#include <windows.h>
#include <mutex>
#include <iomanip>
#include <omp.h>

using namespace std;

long long N = 100000000;
long long BLOCK = 10 * 930835;

long double tPi = 0;
long long done = -1;

//mutex mu;

CRITICAL_SECTION CriticalSection;


long double piCalculation(long long currentN)
{
	long double xn = 0;
	long double pi = 0;
	for (long long i = currentN; (i < currentN + BLOCK) && (i < N); i++)
	{
		xn = (i + 0.5) * (1.0/N);
		pi = pi + (4.0 * (1.0 / (1 + (xn * xn)))) * (1.0 / N);
	}
	return pi;
}

DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter)
{
	long long currentN;
	long double localPi = 0;
	while (done * BLOCK < N)
	{
		EnterCriticalSection(&CriticalSection);
		done++;
		currentN = done * BLOCK;
		LeaveCriticalSection(&CriticalSection);
		//localPi = 0;
		localPi += piCalculation(currentN);

	} 

	tPi += localPi;

	return 0;
}

long double piMultiThread(int n)
{
	tPi = 0;
	done = -1;
	InitializeCriticalSection(&CriticalSection);
	HANDLE *threads = new HANDLE[n];
	for (int i = 0; i < n; i++)
		threads[i] = CreateThread(NULL, 0, ThreadProc, nullptr, CREATE_SUSPENDED, NULL);

	DWORD start = GetTickCount();
	for (int i = 0; i < n; ++i)
		ResumeThread(threads[i]);

	WaitForMultipleObjects(n, threads, true, INFINITE);
	cout << "time: " << GetTickCount() - start << " for " << n << " threads" << endl;

	DeleteCriticalSection(&CriticalSection);

	for (int i = 0; i < n; ++i)
		CloseHandle(threads[i]);
	delete threads;


	return tPi;
}

long double openmp(int n)
{
		long double tmPi = 0;
		long double xn = 0;
		omp_set_num_threads(n);
		DWORD start = GetTickCount();
//#pragma omp [parallel] for schedule(dynamic, BLOCK) reduction(+:dPi)
#pragma omp parallel for schedule(dynamic, BLOCK) reduction(+:tmPi)

		for (int i = 0; i < N; ++i)
		{
			xn = (i + 0.5) * (1.0 / N);
			tmPi = tmPi + (4.0 * (1.0 / (1 + (xn * xn)))) * (1.0 / N);
		}
		cout << "time: " << GetTickCount() - start << " for " << n << " threads" << endl;

	return tmPi;
}

int main()
{
	cout.precision(50);
	cout << piMultiThread(1) << endl;
	cout << piMultiThread(2) << endl;
	cout << piMultiThread(4) << endl;
	cout << piMultiThread(8) << endl;
	cout << piMultiThread(12) << endl;
	cout << piMultiThread(16) << endl;
	cout << openmp(1) << endl;
	cout << openmp(2) << endl;
	cout << openmp(4) << endl;
	cout << openmp(8) << endl;
	cout << openmp(12) << endl;
	cout << openmp(16) << endl;
	system("pause");
}