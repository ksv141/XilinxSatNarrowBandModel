#include <iostream>
#include <fstream>
#include "mls.h"
#include "constellation.h"

using namespace std;

int main()
{
	// отладочный файл
	ofstream dbg_out("dbg_out.txt");

	// псевдослучайный источник символов (4-х позиционный)
	mls symbol_source(32, 4);

	// количество генерируемых символов
	int symbol_count = 1000;

	// основной цикл обработки символов
	for (int i = 0; i < symbol_count; i++)
	{
		// генерация очередного символа
		int current_symbol = symbol_source.nextSymbol();
		dbg_out << current_symbol << endl;
	}

	dbg_out.close();
	return 0;
}