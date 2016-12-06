/* 
 * "Small Hello World" example. 
 * 
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example 
 * designs. It requires a STDOUT  device in your system's hardware. 
 *
 * The purpose of this example is to demonstrate the smallest possible Hello 
 * World application, using the Nios II HAL library.  The memory footprint
 * of this hosted application is ~332 bytes by default using the standard 
 * reference design.  For a more fully featured Hello World application
 * example, see the example titled "Hello World".
 *
 * The memory footprint of this example has been reduced by making the
 * following changes to the normal "Hello World" example.
 * Check in the Nios II Software Developers Manual for a more complete 
 * description.
 * 
 * In the SW Application project (small_hello_world):
 *
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 * In System Library project (small_hello_world_syslib):
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 *    - Define the preprocessor option ALT_NO_INSTRUCTION_EMULATION 
 *      This removes software exception handling, which means that you cannot 
 *      run code compiled for Nios II cpu with a hardware multiplier on a core 
 *      without a the multiply unit. Check the Nios II Software Developers 
 *      Manual for more details.
 *
 *  - In the System Library page:
 *    - Set Periodic system timer and Timestamp timer to none
 *      This prevents the automatic inclusion of the timer driver.
 *
 *    - Set Max file descriptors to 4
 *      This reduces the size of the file handle pool.
 *
 *    - Check Main function does not exit
 *    - Uncheck Clean exit (flush buffers)
 *      This removes the unneeded call to exit when main returns, since it
 *      won't.
 *
 *    - Check Don't use C++
 *      This builds without the C++ support code.
 *
 *    - Check Small C library
 *      This uses a reduced functionality C library, which lacks  
 *      support for buffering, file IO, floating point and getch(), etc. 
 *      Check the Nios II Software Developers Manual for a complete list.
 *
 *    - Check Reduced device drivers
 *      This uses reduced functionality drivers if they're available. For the
 *      standard design this means you get polled UART and JTAG UART drivers,
 *      no support for the LCD driver and you lose the ability to program 
 *      CFI compliant flash devices.
 *
 *    - Check Access device drivers directly
 *      This bypasses the device file system to access device drivers directly.
 *      This eliminates the space required for the device file system services.
 *      It also provides a HAL version of libc services that access the drivers
 *      directly, further reducing space. Only a limited number of libc
 *      functions are available in this configuration.
 *
 *    - Use ALT versions of stdio routines:
 *
 *           Function                  Description
 *        ===============  =====================================
 *        alt_printf       Only supports %s, %x, and %c ( < 1 Kbyte)
 *        alt_putstr       Smaller overhead than puts with direct drivers
 *                         Note this function doesn't add a newline.
 *        alt_putchar      Smaller overhead than putchar with direct drivers
 *        alt_getchar      Smaller overhead than getchar with direct drivers
 *
 */

#include "sys/alt_stdio.h"
#include "system.h"
#include "HAL/inc/io.h"
enum{
	CMD_CREATE = 0x00000001,
	CMD_DESTROY = 0x00000002,
	CMD_INSERT = 0x00000003,
	CMD_REMOVE = 0x00000004,
	CMD_REMOVE_HEAD = 0x00000005,
	CMD_UPDATE_RUNNING = 0x00000006,
	CMD_SET_QUANTUM = 0x00000007,
	CMD_ENABLE = 0x00000008,
	CMD_DISABLE = 0x00000009,
	CMD_INT_ACK = 0x0000000a,
	CMD_GETID = 0x0000000b,
	CMD_CHOSEN = 0x0000000c,
	CMD_SIZE = 0x0000000d,
	CMD_RSTICKS = 0x0000000e,
	CMD_RESET = 0x0000000f
};

enum {
	STAT_ENABLE = 0x00000020,
	STAT_RESHEDULE = 0x00000010,
	STAT_DONE = 0x00000008,
	STAT_FULL = 0x00000004,
	STAT_EMPTY = 0x00000002,
	STAT_ERROR = 0x00000001
};

enum {
	ADDR_CMD = 0x0000,
	ADDR_PRIORITY = 0x0004,
	ADDR_PARAM = 0x0008,
	ADDR_RETURN = 0x000c,
	ADDR_STATUS = 0x0010
};

int  execute_cmd(int cmd, int priority, int param){
	//send command
	IOWR_32DIRECT(ADDR_CMD, 0, cmd);
	IOWR_32DIRECT(ADDR_PRIORITY, 0, priority);
	IOWR_32DIRECT(ADDR_PARAM, 0, param);

	//wait done
	unsigned int status;
	do {
		status = IORD_32DIRECT(ADDR_STATUS, 0);
	} while( (status & STAT_DONE) == 0);

	if (status & STAT_ERROR) {
		return -1;
	}
	//se necessário, tratar status co contexto de quem invocou este método.
	return IORD_32DIRECT(ADDR_RETURN, 0);
}

void test_1() {
	//criacao de uma thread
	alt_putstr("\nTEST: 1\n");
	alt_putstr("Create single thread ");
	execute_cmd(CMD_RESET, 0, 0);

	int ret = execute_cmd(CMD_CREATE, 1, 100);
	if(ret == -1) {
		alt_putstr("[TEST FAIL]\n");
	} else {
		alt_putstr("[TEST PASS]\n");
		alt_printf("-Thread ID %x;\n", IORD_32DIRECT(ADDR_RETURN, 0) );
	}
}

void test_2() {
	//criacao ate estourar o espaco disponivel
	alt_putstr("\nTEST: 2\n");
	alt_putstr("8th thread creation ");
	execute_cmd(CMD_RESET, 0, 0);

	int i = 0;
	int ret = 0;
	for(i = 1; i<9; i++) {
		ret = execute_cmd(CMD_CREATE, 1, i*100);
		if(ret == -1) {
			alt_putstr("[TEST FAIL]\n");
			return;
		}
	}

	alt_putstr("[TEST PASS]\n");
	alt_printf("Thread ID %x;\n", IORD_32DIRECT(ADDR_RETURN, 0) );
}

void test_3() {
	//criacao ate estourar o espaco disponivel
	alt_putstr("\nTEST: 3\n");
	alt_putstr("Expected error on 9th thread creation ");
	execute_cmd(CMD_RESET, 0, 0);

	int i = 0;
	int ret = 0;
	for(i = 1; i<9; i++) {
		ret = execute_cmd(CMD_CREATE, 1, i*100);
		if(ret == -1) {
			alt_putstr("[TEST FAIL]\n");
			return;
		}
	}
	ret = execute_cmd(CMD_CREATE, 1, 900);
	if(ret == -1) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_4() {
	alt_putstr("\nTEST: 4\n");
	alt_putstr("Check empty status ");
	unsigned int status = IORD_32DIRECT(ADDR_STATUS, 0);
	int empty = status & STAT_EMPTY;
	if(empty == 2) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_5() {
	alt_putstr("\nTEST: 5\n");
	alt_putstr("Check empty status after create");
	execute_cmd(CMD_RESET, 0, 0);

	execute_cmd(CMD_CREATE, 1, 100);
	unsigned int status = IORD_32DIRECT(ADDR_STATUS, 0);
	int empty = status & STAT_EMPTY;
	if(empty == 2) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_6() {
	alt_putstr("\nTEST: 6\n");
	alt_putstr("Check empty status after insert ");
	execute_cmd(CMD_RESET, 0, 0);

	int tid = execute_cmd(CMD_CREATE, 1, 100);
	execute_cmd(CMD_INSERT, 1, tid);

	unsigned int status = IORD_32DIRECT(ADDR_STATUS, 0);
	int empty = status & STAT_EMPTY;
	if(empty == 0) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_7() {
	alt_putstr("\nTEST: 7\n");
	alt_putstr("Destroy a created thread");
	execute_cmd(CMD_RESET, 0, 0);

	int tid = execute_cmd(CMD_CREATE, 1, 100);
	execute_cmd(CMD_DESTROY, 0, tid);
	int ret = execute_cmd(CMD_GETID, 0, 1);
	if(ret < 0) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_8() {
	alt_putstr("\nTEST: 8\n");
	alt_putstr("Destroy an inserted thread");
	execute_cmd(CMD_RESET, 0, 0);

	int tid = execute_cmd(CMD_CREATE, 1, 100);
	execute_cmd(CMD_INSERT, 1, tid);
	execute_cmd(CMD_DESTROY, 0, tid);
	int ret = execute_cmd(CMD_GETID, 0, 100);
	int empty = IORD_32DIRECT(ADDR_STATUS, 0) & STAT_EMPTY;
	if(ret < 0 && empty == STAT_EMPTY) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_9() {
	alt_putstr("\nTEST: 9\n");
	alt_putstr("Create Thread after destroy 1 of 8 ");
	execute_cmd(CMD_RESET, 0, 0);

	int i = 0;
	int ret = 0;
	for(i = 1; i<9; i++) {
		ret = execute_cmd(CMD_CREATE, 1, i*100);
		if(ret == -1) {
			alt_putstr("[TEST FAIL]\n");
			return;
		}
	}
	int tid = 3;
	execute_cmd(CMD_DESTROY, 0, tid);
	ret = execute_cmd(CMD_CREATE, 1, 900);

	if(ret == -1 && ret != tid) {
		alt_putstr("[TEST FAIL]\n");
	} else {
		alt_putstr("[TEST PASS]\n");
		alt_printf(" Removed = %x, new Thread ID = %x;\n", ret, tid);
	}

}

void test_10() {
	alt_putstr("\nTEST: 10\n");
	alt_putstr("Get ID return error if not created ");
	execute_cmd(CMD_RESET, 0, 0);

	int ret = execute_cmd(CMD_GETID, 0, 100);
	if(ret == -1) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_11() {
	alt_putstr("\nTEST: 11\n");
	alt_putstr("Get ID return correct ID if created ");
	execute_cmd(CMD_RESET, 0, 0);

	int address = 100;
	int tid = execute_cmd(CMD_CREATE, 0, address);
	int ret = execute_cmd(CMD_GETID, 0, address);
	if(ret != -1 && ret == tid) {
		alt_putstr("[TEST PASS]\n");
		alt_printf("-Thread ID = %x\n", tid);
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_12() {
	alt_putstr("\nTEST: 12\n");
	alt_putstr("Check Size ");
	execute_cmd(CMD_RESET, 0, 0);

	int ret = execute_cmd(CMD_SIZE, 0, 0);
	if(ret == 1) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_13() {
	alt_putstr("\nTEST: 13\n");
	alt_putstr("Check Size after create ");
	execute_cmd(CMD_RESET, 0, 0);

	execute_cmd(CMD_CREATE, 0, 100);
	int ret = execute_cmd(CMD_SIZE, 0, 0);
	if(ret == 1) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_14() {
	alt_putstr("\nTEST: 14\n");
	alt_putstr("Check Size after insert ");
	execute_cmd(CMD_RESET, 0, 0);

	int tid = execute_cmd(CMD_CREATE, 1, 100);
	execute_cmd(CMD_INSERT, 1, tid);

	int ret = execute_cmd(CMD_SIZE, 0, 0);
	if(ret == 2) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_15() {
	alt_putstr("\nTEST: 15\n");
	alt_putstr("Check Size after insert and remove ");
	execute_cmd(CMD_RESET, 0, 0);

	int tid = execute_cmd(CMD_CREATE, 1, 100);
	execute_cmd(CMD_INSERT, 1, tid);
	execute_cmd(CMD_REMOVE, 0, tid);
	int ret = execute_cmd(CMD_SIZE, 0, 0);
	if(ret == 1) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_16() {
	alt_putstr("\nTEST: 16\n");
	alt_putstr("Check Size after insert and destroy ");
	execute_cmd(CMD_RESET, 0, 0);

	int tid = execute_cmd(CMD_CREATE, 1, 100);
	execute_cmd(CMD_INSERT, 1, tid);
	execute_cmd(CMD_DESTROY, 0, tid);
	int ret = execute_cmd(CMD_SIZE, 0, 0);
	if(ret == 1) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_17() {
	alt_putstr("\nTEST: 17\n");
	alt_putstr("Check Size after insert and remove head ");
	execute_cmd(CMD_RESET, 0, 0);

	int tid = execute_cmd(CMD_CREATE, 1, 100);
	execute_cmd(CMD_INSERT, 1, tid);
	tid = execute_cmd(CMD_CREATE, 1, 200);
	execute_cmd(CMD_INSERT, 1, tid);
	execute_cmd(CMD_REMOVE_HEAD, 0, 0);
	int ret = execute_cmd(CMD_SIZE, 0, 0);
	if(ret == 2) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_18() {
	alt_putstr("\nTEST: 18\n");
	alt_putstr("Interrupt enable ");
	execute_cmd(CMD_RESET, 0, 0);

	int ret = execute_cmd(CMD_ENABLE, 0, 0);
	if(ret == -1) {
		alt_putstr("[TEST FAIL - 1]\n");
		return;
	}

	int enable = IORD_32DIRECT(ADDR_STATUS, 0) & STAT_ENABLE;

	if(enable == STAT_ENABLE) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL - 2]\n");
		alt_printf("Status: %x\n", IORD_32DIRECT(ADDR_STATUS, 0));
	}
}

void test_19() {
	alt_putstr("\nTEST: 19\n");
	alt_putstr("Interrupt disable ");
	execute_cmd(CMD_RESET, 0, 0);

	int ret = execute_cmd(CMD_ENABLE, 0, 0);
	if(ret == -1) {
		alt_putstr("[TEST FAIL]\n");
		return;
	}

	ret = execute_cmd(CMD_DISABLE, 0, 0);
	if(ret == -1) {
		alt_putstr("[TEST FAIL]\n");
		return;
	}
	int enable = IORD_32DIRECT(ADDR_STATUS, 0) & STAT_ENABLE;
	if(enable == 0) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL]\n");
	}
}

void test_20() {
	alt_putstr("\nTEST: 20\n");
	alt_putstr("Interrupt occurs after enable and another is waiting ");
	execute_cmd(CMD_RESET, 0, 0);

	execute_cmd(CMD_ENABLE, 0, 0);
	int tid = execute_cmd(CMD_CREATE, 2, 100);
	execute_cmd(CMD_INSERT, 2, tid);
	execute_cmd(CMD_UPDATE_RUNNING, 0, tid);

	tid = execute_cmd(CMD_CREATE, 1, 200);
	execute_cmd(CMD_INSERT, 1, tid);
	alt_putstr("\n[DONE]\n");
	while(1);
}

void test_21() {
	alt_putstr("\nTEST: 21\n");
	alt_putstr("Set to lower quantum ");
	execute_cmd(CMD_RESET, 0, 0);

	execute_cmd(CMD_ENABLE, 0, 0);
	int tid = execute_cmd(CMD_CREATE, 2, 100);
	execute_cmd(CMD_INSERT, 2, tid);
	execute_cmd(CMD_UPDATE_RUNNING, 0, tid);
	tid = execute_cmd(CMD_CREATE, 1, 200);
	execute_cmd(CMD_INSERT, 1, tid);

	int ret = execute_cmd(CMD_SET_QUANTUM, 0, 50000000);
	if(ret == -1) {
		alt_putstr("[TEST FAIL]\n");
	}
	execute_cmd(CMD_RSTICKS, 0, 0);

	alt_putstr("\n[DONE]\n");
		while(1);
}

void test_22() {
	alt_putstr("\nTEST: 22\n");
	alt_putstr("Remove inserted thread ");
	execute_cmd(CMD_RESET, 0, 0);

	int address = 100;
	int ret = 0;
	int tid = execute_cmd(CMD_CREATE, 1, address);
	ret = execute_cmd(CMD_INSERT, 1, tid);
	if(ret == -1) {
		alt_putstr("[TEST FAIL - 1]\n");
		return;
	}

	ret = execute_cmd(CMD_REMOVE, 0, tid);
	if(ret == -1) {
		alt_putstr("[TEST FAIL - 2]\n");
		return;
	}

	ret = execute_cmd(CMD_GETID, 0, address);
	if(ret == tid) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL - 3]\n");
	}
}

void test_23() {
	alt_putstr("\nTEST: 23\n");
	alt_putstr("Remove not inserted thread don't change Size");
	execute_cmd(CMD_RESET, 0, 0);

	int address = 100;
	int ret = 0;
	int tid = execute_cmd(CMD_CREATE, 1, address);
	ret = execute_cmd(CMD_INSERT, 1, tid);
	if(ret == -1) {
		alt_putstr("[TEST FAIL - 1]\n");
		return;
	}
	int size1 = execute_cmd(CMD_SIZE, 0, 0);

	ret = execute_cmd(CMD_REMOVE, 0, tid-1);
	if(ret == -1) {
		alt_putstr("[TEST FAIL - 2]\n");
		return;
	}

	int size2 = execute_cmd(CMD_SIZE, 0, 0);

	if(size1 == size2) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL - 3]\n");
	}
}

void test_24() {
	alt_putstr("\nTEST: 24\n");
	alt_putstr("choose return 0 when no thread is running ");
	execute_cmd(CMD_RESET, 0, 0);

	int ret = execute_cmd(CMD_CHOSEN, 0, 0);
	if(ret == 0) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL - 1]\n");
	}
}

void test_25() {
	alt_putstr("\nTEST: 25\n");
	alt_putstr("choose return running thread address ");
	execute_cmd(CMD_RESET, 0, 0);

	int address = 100;
	int ret = 0;
	int tid = execute_cmd(CMD_CREATE, 1, address);
	ret = execute_cmd(CMD_INSERT, 1, tid);
	if(ret == -1) {
		alt_putstr("[TEST FAIL - 1]\n");
		return;
	}
	execute_cmd(CMD_UPDATE_RUNNING, 0, tid);
	ret = execute_cmd(CMD_CHOSEN, 0, 0);

	if(ret == address) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL - 2]\n");
	}
}

void test_26() {
	alt_putstr("\nTEST: 26\n");
	alt_putstr("Update Running change running thread");
	execute_cmd(CMD_RESET, 0, 0);

	int address_a = 100;
	int address_b = 200;
	int ret = 0;
	int tid_a = execute_cmd(CMD_CREATE, 1, address_a);
	ret = execute_cmd(CMD_INSERT, 1, tid_a);
	if(ret == -1) {
		alt_putstr("[TEST FAIL - 1]\n");
		return;
	}

	int tid_b = execute_cmd(CMD_CREATE, 1, address_b);
	ret = execute_cmd(CMD_INSERT, 1, tid_b);
	if(ret == -1) {
		alt_putstr("[TEST FAIL - 2]\n");
		return;
	}

	execute_cmd(CMD_UPDATE_RUNNING, 0, tid_a);
	int running_a = execute_cmd(CMD_CHOSEN, 0, 0);
	execute_cmd(CMD_UPDATE_RUNNING, 0, tid_b);
	int running_b = execute_cmd(CMD_CHOSEN, 0, 0);
	if(address_a == running_a && address_b == running_b) {
		alt_putstr("[TEST PASS]\n");
	} else {
		alt_putstr("[TEST FAIL - 3]\n");
	}
}

static void irqhandler_1 (void *context, alt_u32 id)
{
	execute_cmd(CMD_INT_ACK, 0, 0);
	alt_printf ("\n=================\ninterrupt occurred\n=================\n");
}

int main()
{ 
	alt_putstr("Hello from Nios II! :D\n");
	test_1();
	test_2();
	test_3();
	test_4();
	test_5();
	test_6();
	test_7();
	test_8();
	test_9();
	test_10();
	test_11();
	test_12();
	test_13();
	test_14();
	test_15();
	test_16();
	test_17();
	test_18();
	test_19();

	test_22();
	test_23();
	test_24();
	test_25();
	test_26();

//	executar testes de interrupcao separadamente
//  problema para tratar fim da interrupcao
	alt_irq_register( SCHEDULER_0_IRQ, 0, (void*)irqhandler_1);
//	test_20();
//	test_21();

	alt_putstr("\n[DONE]\n");


  /* Event loop never exits. */
  while (1);

  return 0;
}
