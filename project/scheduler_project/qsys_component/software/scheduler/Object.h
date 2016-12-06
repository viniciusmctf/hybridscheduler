#include "HAL/inc/io.h"

class Object {
	int id;
public:
	int priority;


	Object(int id, int priority) : id(id), priority(priority) {
	}

	void execute() {
		alt_printf("Ola, sou a Thread '%x'\nPrioridade: '%x'\n\n", id, priority);
	}
};
