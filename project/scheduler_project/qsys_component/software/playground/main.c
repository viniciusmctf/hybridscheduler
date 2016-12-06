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
#include "sys/alt_irq.h"
#include "sys/alt_irq_entry.h"
#include "HAL/inc/io.h"

void reset(){
	//SEND
	IOWR_32DIRECT(0x0000, 0, 0x0000000e);
	IOWR_32DIRECT(0x0004, 0, 0);
	IOWR_32DIRECT(0x0008, 0, 0);

	//READ
	int status, done;
	do {
		status = IORD_32DIRECT(0x0010,0);
		done = status & 0x00000008;
	} while(done == 0);

	alt_printf("Status: %x\n", status);
	int _return = IORD_32DIRECT(0x000c, 0);
	alt_printf("return: %x\n", _return);
}

void update(int parameter){
	//SEND
	IOWR_32DIRECT(0x0000, 0, 0x00000006);
	IOWR_32DIRECT(0x0004, 0, 0);
	IOWR_32DIRECT(0x0008, 0, parameter);

	//READ
	int status, done;
	do {
		status = IORD_32DIRECT(0x0010,0);
		done = status & 0x00000008;
	} while(done == 0);

	alt_printf("Status: %x\n", status);
	int _return = IORD_32DIRECT(0x000c, 0);
	alt_printf("return: %x\n", _return);
}

void enable(){
	//SEND
	IOWR_32DIRECT(0x0000, 0, 0x00000008);
	IOWR_32DIRECT(0x0004, 0, 0);
	IOWR_32DIRECT(0x0008, 0, 0);

	//READ
	int status, done;
	do {
		status = IORD_32DIRECT(0x0010,0);
		done = status & 0x00000008;
	} while(done == 0);

	alt_printf("Status: %x\n", status);
	int _return = IORD_32DIRECT(0x000c, 0);
	alt_printf("return: %x\n", _return);
}

void disable(){
	//SEND
	IOWR_32DIRECT(0x0000, 0, 0x00000009);
	IOWR_32DIRECT(0x0004, 0, 0);
	IOWR_32DIRECT(0x0008, 0, 0);

	//READ
	int status, done;
	do {
		status = IORD_32DIRECT(0x0010,0);
		done = status & 0x00000008;
	} while(done == 0);

	alt_printf("Status: %x\n", status);
	int _return = IORD_32DIRECT(0x000c, 0);
	alt_printf("return: %x\n", _return);
}

void create(int priority, int parameter){
	//SEND
	IOWR_32DIRECT(0x0000, 0, 0x00000001);
	IOWR_32DIRECT(0x0004, 0, priority);
	IOWR_32DIRECT(0x0008, 0, parameter);

	//READ
	int status, done;
	do {
		status = IORD_32DIRECT(0x0010,0);
		done = status & 0x00000008;
	} while(done == 0);

	alt_printf("Status: %x\n", status);
	int _return = IORD_32DIRECT(0x000c, 0);
	alt_printf("return: %x\n", _return);
}

void insert(int priority, int parameter){
	//SEND
	IOWR_32DIRECT(0x0000, 0, 0x00000003);
	IOWR_32DIRECT(0x0004, 0, priority);
	IOWR_32DIRECT(0x0008, 0, parameter);

	//READ
	int status, done;
	do {
		status = IORD_32DIRECT(0x0010,0);
		done = status & 0x00000008;
	} while(done == 0);

	alt_printf("Status: %x\n", status);
	int _return = IORD_32DIRECT(0x000c, 0);
	alt_printf("return: %x\n", _return);
}

void size(){
	//SEND
	IOWR_32DIRECT(0x0000, 0, 0x0000000d);
	IOWR_32DIRECT(0x0004, 0, 0);
	IOWR_32DIRECT(0x0008, 0, 0);

	//READ
	int status, done;
	do {
		status = IORD_32DIRECT(0x0010,0);
		done = status & 0x00000008;
	} while(done == 0);


	alt_printf("Status: %x\n", status);
	int _return = IORD_32DIRECT(0x000c, 0);
	alt_printf("return: %x\n", _return);
}

void int_ack(){
	//SEND
	IOWR_32DIRECT(0x0000, 0, 0x0000000a);
	IOWR_32DIRECT(0x0004, 0, 0);
	IOWR_32DIRECT(0x0008, 0, 0);

	//READ
	int status, done;
	do {
		status = IORD_32DIRECT(0x0010,0);
		done = status & 0x00000008;
	} while(done == 0);


	alt_printf("Status: %x\n", status);
	int _return = IORD_32DIRECT(0x000c, 0);
	alt_printf("return: %x\n", _return);
}

static void irqhandler (void *context, alt_u32 id)
{
	int_ack();
	alt_printf ("=================\ninterrupt occurred\n");
}

int main()
{
	alt_irq_register( SCHEDULER_0_IRQ, 0, (void*)irqhandler);
	alt_putstr("========================\n");


	alt_printf("\n1\n");
	enable();

	alt_printf("\n2\n");
	disable();

	alt_printf("\n3\n");
	enable();

	alt_printf("\nSIZE\n");
	size();

	alt_printf("\n6\n");
	create(1, 1);

	alt_printf("\n7\n");
	create(2, 2);

	alt_printf("\n8\n");
	create(1, 3);

	alt_printf("\n9\n");
	create(1, 4);

	alt_printf("\nSIZE\n");
	size();

	alt_printf("\n10\n");
	create(1, 5);

	alt_printf("\n11\n");
	create(1, 6);

	alt_printf("\nSIZE\n");
	size();

	alt_printf("\n12\n");
	insert(2, 2);

	alt_printf("\n0\n");
	reset();

	alt_printf("\nUPDATE\n");
	update(2);

	alt_printf("\nSIZE\n");
	size();

	alt_printf("\n13\n");
	create(1, 7);

	alt_printf("\n14\n");
	create(1, 8);

	alt_printf("\n15\n");
	create(1, 9);

	alt_printf("\n16\n");
	insert(1, 1);

	alt_printf("\nSIZE\n");
	size();

  while (1);

  return 0;
}

