// transition.cpp : Definiert den Einstiegspunkt f�r die Konsolenanwendung.
//

#include <iostream>
#include "conio.h"
#include "RenderingEngine.h"

int main()
{
	auto engine = RenderingEngine(800, 600);
	engine.run();

    return 0;
}

