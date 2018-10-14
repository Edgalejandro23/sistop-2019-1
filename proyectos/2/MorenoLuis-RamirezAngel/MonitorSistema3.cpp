// MonitorSistema3.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <mutex>


#pragma comment(lib, "iphlpapi.lib")
std::mutex candado;

char c = 'a';
int bandera = 0;
unsigned int usoCPU=0;
unsigned int porciento=0;
float usada = 0, total = 0, disponible = 0;
float dTot=0, dEot=0;

void usoDelCPU();
void usoDeRam();
void datagramas();
void imprime();
void barraCPU();
void barraRAM();
void barraRecibe();
void barraEnvia();
void pideImpresion();
void espera();


int main()
{

	//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 51);
	//printf("Hola %d\n",51);
	//int i;
	std::thread hilos[3];

	//for (i = 0; i < 4; i++)
	//{
	//printf("\n\nPasada %d: \n\n", i);
	hilos[0] = std::thread(usoDelCPU);
	hilos[1] = std::thread(usoDeRam);
	hilos[2] = std::thread(datagramas);

	while (c != '\n')
	{
		c = getchar();
	}

	hilos[0].join();
	hilos[1].join();
	hilos[2].join();

	getchar();
	getchar();
	return 0;
}

void usoDelCPU()
{
	FILETIME idle, kernel, user;
	ULARGE_INTEGER aIdle, nIdle, aKernel, nKernel, aUser, nUser, tIdle, tKernel, tUser;
	std::thread impresora;
	//int i = 0;

	GetSystemTimes(&idle, &kernel, &user); //Obtenemos datos para inicializar variables

	aIdle.LowPart = idle.dwLowDateTime; //Inicializamos las variables "antiguas"
	aIdle.HighPart = idle.dwHighDateTime;
	aKernel.LowPart = kernel.dwLowDateTime;
	aKernel.HighPart = kernel.dwHighDateTime;
	aUser.LowPart = user.dwLowDateTime;
	aUser.HighPart = user.dwHighDateTime;
	Sleep(1000); //Esperamos antes de medir los datos de nuevo

	while (c != '\n')
	{
		GetSystemTimes(&idle, &kernel, &user); //Medimos los datos "nuevos"

		nIdle.LowPart = idle.dwLowDateTime; //Guardamos los datos "nuevos"
		nIdle.HighPart = idle.dwHighDateTime;
		nKernel.LowPart = kernel.dwLowDateTime;
		nKernel.HighPart = kernel.dwHighDateTime;
		nUser.LowPart = user.dwLowDateTime;
		nUser.HighPart = user.dwHighDateTime;

		tIdle.QuadPart = nIdle.QuadPart - aIdle.QuadPart; //Calculamos los tiempos "totales" restandole los
		tUser.QuadPart = nUser.QuadPart - aUser.QuadPart; //tiempos antiguos a los tiempos nuevos
		tKernel.QuadPart = nKernel.QuadPart - aKernel.QuadPart;

		//calculamos el uso del CPU
		espera(); //Si se esta imprimiendo espera antes de modificar los valores
		usoCPU = ((tKernel.QuadPart + tUser.QuadPart - tIdle.QuadPart) * 100 / (tKernel.QuadPart + tUser.QuadPart));

		impresora = std::thread(pideImpresion);
		
		aIdle.QuadPart = nIdle.QuadPart;
		aKernel.QuadPart = nKernel.QuadPart;
		aUser.QuadPart = nUser.QuadPart;

		Sleep(1000);
		impresora.join();
	}
}

void usoDeRam()
{
	MEMORYSTATUSEX memoria;
	std::thread impresora;
	int i;
	
	memoria.dwLength = sizeof(memoria);


	while (c != '\n')
	{
		GlobalMemoryStatusEx(&memoria);

		espera(); //Si se esta imprimiendo espera antes de modificar los valores
		usada = (float)(memoria.ullTotalPhys - memoria.ullAvailPhys)/1000000000;

		espera(); //Si se esta imprimiendo espera antes de modificar los valores
		total = (float)memoria.ullTotalPhys /1000000000;

		espera(); //Si se esta imprimiendo espera antes de modificar los valores
		disponible = (float)memoria.ullAvailPhys /1000000000;

		espera(); //Si se esta imprimiendo espera antes de modificar los valores
		porciento = memoria.dwMemoryLoad;

		impresora = std::thread(pideImpresion);
		Sleep(1000);//Espera antes de volver a leer los datos
		impresora.join();//Espera a que hilo impresora haya terminado
	}
}

void datagramas()
{
	//int i;
	unsigned int rAnt, rNvo, eAnt, eNvo;
	std::thread impresora;
	MIB_IPSTATS *pStats;
	pStats = (MIB_IPSTATS *)malloc(sizeof(MIB_IPSTATS));

	//incializamos datos =D
	GetIpStatistics(pStats);
	rAnt = pStats->dwInReceives;
	eAnt = pStats->dwOutRequests;
	Sleep(1000); //esperamos para poder tomar datos de nuevo =D


	while (c != '\n')
	{
		GetIpStatistics(pStats);
		rNvo = pStats->dwInReceives;
		eNvo = pStats->dwOutRequests;

		espera(); //Si se esta imprimiendo espera antes de modificar los valores
		dTot = (float)(rNvo - rAnt)*(1.5f);
		espera(); //Si se esta imprimiendo espera antes de modificar los valores
		dEot = (float)(eNvo - eAnt)*(1.5f);
		impresora = std::thread(pideImpresion);

		rAnt = rNvo;
		eAnt = eNvo;
		Sleep(1000);
		impresora.join();//espera a que el hilo impresora haya terminado
	}
}

void imprime()
{

	int i;
	system("cls");
	printf("\n\n");
	barraCPU();
	printf("\tCPU:\n");
	barraCPU();
	printf("\tUso %3u%%\n\n", usoCPU);

	//------------------------
	barraRAM();
	printf("\tRAM: %.2g GB usados [%3u%%]\n", usada, porciento);
	barraRAM();
	printf("\t%.2g GB disponibles de %.2g GB\n\n\t", disponible, total);
	//88888888888888888888888888888


	if (dTot > 999)
	{
		barraRecibe();
		printf("\tInternet (datagramas):\n\t");
		barraRecibe();
		dTot = dTot / 1000;
		printf("\tRecibo %6g Mbps\n\n\t", dTot);
	}
	else
	{
		barraRecibe();
		printf("\tInternet (datagramas):\n\t");
		barraRecibe();
		printf("\tRecibo: %6g kbps\n\n\t", dTot);
	}
	if (dEot > 999)
	{
		barraEnvia();
		printf("\tInternet (datagramas):\n\t");
		barraEnvia();
		dEot = dEot / 1000;
		printf("\tEnvio %6g Mbps\n\n\t", dEot);
	}
	else
	{
		barraEnvia();
		printf("\tInternet (datagramas):\n\t");
		barraEnvia();
		printf("\tEnvio %6g kbps\n\n\t", dEot);
	}
	printf("Presiona ENTER para terminar el programa =D ");
}

void barraCPU()
{
	unsigned int i;
	if (usoCPU<33)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 34);//verde
	}
	else
	{
		if (usoCPU < 66)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 238);//amarillo
		}
		else
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 204);//rojo
		}
	}

	printf("\t");
	for (i = 0; i<(usoCPU/5); i++)
	{
		printf(" ");
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	for (i = 0; i<20 - (usoCPU / 5); i++)
	{
		printf("%c", 176);
	}
}

void barraRAM()
{
	unsigned int i;
	printf("\t");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 83);
	for (i = 0; i<porciento / 5; i++)
	{
		printf(" ");
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	for (i = 0; i<20 - (porciento / 5); i++)
	{
		printf("%c", 176);
	}
}

void barraRecibe()
{
	int i;
	int auxTot;
	if (dTot > 999)//si son Megabytes
	{
		auxTot = (int)(dTot / 1000);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 153);
		if (auxTot > 20)
		{
			for (i = 0; i < 20; i++)
			{
				printf(" ");
			}
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		}
		else
		{
			for (i = 0; i < auxTot; i++)
			{
				printf(" ");
			}
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			for (i = 0; i < 20-auxTot; i++)
			{
				printf("%c",176);
			}
		}
	}
	else //si son kbytes
	{
		auxTot = (int)(dTot / 50);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 51);
		
		for (i = 0; i < auxTot; i++)
		{
			printf(" ");
		}
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		for (i = 0; i < 20 - auxTot; i++)
		{
			printf("%c", 176);
		}

	}

	
}

void barraEnvia()
{
	int i;
	int auxTot;
	if (dEot > 999)//si son Megabytes
	{
		auxTot = (int)(dEot / 1000);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 153);
		if (auxTot > 20)
		{
			for (i = 0; i < 20; i++)
			{
				printf(" ");
			}
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		}
		else
		{
			for (i = 0; i < auxTot; i++)
			{
				printf(" ");
			}
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			for (i = 0; i < 20 - auxTot; i++)
			{
				printf("%c", 176);
			}
		}
	}
	else //si son kbytes
	{
		auxTot = (int)(dEot / 50);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 51);

		for (i = 0; i < auxTot; i++)
		{
			printf(" ");
		}
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		for (i = 0; i < 20 - auxTot; i++)
		{
			printf("%c", 176);
		}

	}


}

void pideImpresion()
{
	candado.lock();
	bandera = 1; //Estámos imprimiendo
	imprime();
	bandera = 0; //Terminamos de imprimir
	//Sleep(500);
	candado.unlock();
}

void espera()
{
	while (bandera == 1)//si se está imprimiendo
	{
		Sleep(10); //espera
	}
}