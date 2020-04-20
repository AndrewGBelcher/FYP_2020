/**
 * @file main.c
 * @author Andrew Belcher
 * @date 17 April 2020
 * @brief Source file for testing the shadow stack implementation on riscv-isa-sim
 */
#include <stdio.h>
#include <stdint.h>

// test return address corruption
void test_call(void)
{
	char* string = "\nin testcall\n\n";

	_write(1, string, strlen(string));

	// load some unused register with 33, an incorrect return address
	asm volatile("li a5, 33");

	// write to the currently stored return address on the stack to simulate a rop attack
	asm volatile("sd a5, 24(sp)");
}

// dummy process 1
void proc1(void)
{
	char* string = "\nprocess 1!\n\n";

	_write(1, string, strlen(string));
}

// dummy process 2
void proc2(void)
{
	char* string = "\nprocess 2!\n\n";

	_write(1, string, strlen(string));
}


int main(int argc, char *argv[])
{  

	// only need 1 extra argument to run
	if(argc == 2)
	{
		// Test bounds of thread selector
		if(!strcmp(argv[1], "th_bounds"))
		{
			char* string = "\nadjust value!\n\n";

			asm volatile("ssth 4");
			_write(1, string, strlen(string));
		}

		// Test bounds of pushing to shadow stack
		else if(!strcmp(argv[1], "st_bounds"))
		{
			char value[20];

			for(int i = 0; i < 257; i++)
			{
				asm volatile("ssst");
				sprintf(value, "on st:%d\n", i);
				_write(1, value, strlen(value));
			}
		}

		// Test bounds of popping from shadow stack
		else if(!strcmp(argv[1], "ld_bounds"))
		{
			char value[20];

			for(int i = 0; i < 257; i++)
			{
				asm volatile("ssld");
				sprintf(value, "on ld:%d\n", i);
				_write(1, value, strlen(value));
			}
		}

		// Test return address corruption recovery
		else if(!strcmp(argv[1], "call"))
		{
			char* string1 = "\nbefore corruption\n\n";
			char* string2 = "\nrecovery worked!\n\n";

			_write(1, string1, strlen(string1));
		
			test_call();

			_write(1, string2, strlen(string2));

		}

		// Test multithread functionality of shadow stack system
		else if(!strcmp(argv[1], "multithread"))
		{

			char* str = "\nshed process 0!\n\n";

			// simulate a scheduler for the purposes of testing multiple shadow stacks 
			// each process will have a prologue/epilogue in its function containing store/load
			// shadow stack opcodes
			while(1)
			{
				// switch back to 1st shadow stack and call its associated process
				asm volatile("ssth 0");	
				_write(1, str, strlen(str)); // current workaround for newlib CFI fault with printf

				asm volatile("ssth 1");	
				proc1();

				asm volatile("ssth 2");	
				proc2();
			}
		}
	}

	return 0;

} 

