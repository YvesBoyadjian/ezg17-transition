// transition.cpp : Definiert den Einstiegspunkt f�r die Konsolenanwendung.
//

#include "stdafx.h"
#include <iostream>
#include "conio.h"
#include "RenderingEngine.h"

int main()
{
	RenderingEngine *engine = new RenderingEngine();


	std::cout << "Hello World" << std::endl;
	_getch();
    return 0;
}

