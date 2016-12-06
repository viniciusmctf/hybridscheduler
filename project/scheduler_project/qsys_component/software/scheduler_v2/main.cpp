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
#include "Scheduler.h"
#include "Object.h"
//#include "sys/alt_irq.h"
#include <sys/alt_irq.h>
//#include <priv/alt_legacy_irq.h>

int main()
{

	alt_putstr("Hello from Nios! :D\n");
	Scheduler<Object> scheduler = Scheduler<Object>();

	Object* running;
	Object t1 = Object(0x000a, 0x01);
	Object t2 = Object(0x000b, 0x02);
	Object t3 = Object(0x000c, 0x03);
	Object t4 = Object(0x000d, 0x04);
	Object t5 = Object(0x000e, 0x05);
	Object t6 = Object(0x000f, 0x07);
	Object t7 = Object(0x0010, 0x07);
	Object t8 = Object(0x0011, 0x07);
	Object t9 = Object(0x0012, 0x07);

	scheduler.insert(&t1);
	scheduler.insert(&t2);
	scheduler.insert(&t3);
	scheduler.insert(&t4);
	scheduler.insert(&t5);
	scheduler.insert(&t6);
	scheduler.insert(&t7);
	scheduler.insert(&t8);
//
	int size = scheduler.schedulables();
	alt_printf("Size = %x\n\n", size);
//
	alt_printf("OP: 1\n");
	running = (Object*)scheduler.choose();
	running->execute();

	alt_printf("OP: 2\n");
	running = (Object*)scheduler.choose();
	running->execute();

	alt_printf("OP: 3\n");
	running = (Object*)scheduler.choose_another();
	running->execute();

	alt_printf("OP: 4\n");
	running = (Object*)scheduler.choose_another();
	running->execute();

	scheduler.suspend(&t1);
	scheduler.suspend(&t2);

	alt_printf("OP: 5\n");
	running = (Object*)scheduler.choose();
	running->execute();

	alt_printf("OP: 6\n");
	scheduler.choose(&t8);
	running = (Object*)scheduler.chosen();
	running->execute();

	alt_printf("OP: 7\n");
	scheduler.remove(&t8);
	running = (Object*)scheduler.chosen();
	running->execute();

	alt_printf("OP: 8\n");
	scheduler.resume(&t1);
	running = (Object*)scheduler.choose();
	running->execute();


	alt_printf("OP: 9\n");
	scheduler.remove(&t1);
	scheduler.remove(&t2);
	size = scheduler.schedulables();
	alt_printf("Size = %x\n\n", size);
	running = (Object*)scheduler.choose();
	running->execute();

	alt_printf("OP: 10\n");
	scheduler.remove(&t3);
	scheduler.remove(&t4);
	scheduler.remove(&t5);
	scheduler.insert(&t9);
	size = scheduler.schedulables();
	alt_printf("Size = %x\n\n", size);

	scheduler.set_quantum(50000000);
	scheduler.enable_schedule();

  /* Event loop never exits. */
  while (1);

  return 0;
}
