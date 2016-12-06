
#include "system.h"
#include "HAL/inc/io.h"
#include "sys/alt_stdio.h"
#include <sys/alt_irq.h>
//template <typename T, bool hardware>

template <typename T>
class Scheduler {
protected:
	//command value
	enum{
		CMD_CREATE 	= 0x1,
		CMD_DESTROY = 0x2,
		CMD_INSERT	= 0x3,
		CMD_REMOVE 	= 0x4,
		CMD_REMOVE_HEAD 	= 0x5,
		CMD_UPDATE_RUNNING 	= 0x6,
		CMD_SET_QUANTUM 	= 0x7,
		CMD_ENABLE 	= 0x8,
		CMD_DISABLE = 0x9,
		CMD_INT_ACK = 0xa,
		CMD_GETID 	= 0xb,
		CMD_CHOSEN 	= 0xc,
		CMD_SIZE 	= 0xd,
		CMD_RSTICKS = 0xe,
		CMD_RESET 	= 0xf
	};

	//status mask
	enum {
		STAT_ENABLE = 0x00,
		STAT_ERROR 	= 0x01,
		STAT_EMPTY	= 0x02,
		STAT_FULL 	= 0x04,
		STAT_DONE 	= 0x08,
		STAT_RESHEDULE = 0x10
	};

	//address offset
	enum {
		ADDR_CMD	= 0x00,
		ADDR_PRIORITY = 0x04,
		ADDR_PARAM 	= 0x08,
		ADDR_RETURN = 0x0c,
		ADDR_STATUS = 0x10
	};

	//send command to hardware
	static int execute_cmd(int cmd, int priority, int param) {
		//send command
		IOWR_32DIRECT(SCHEDULER_0_BASE, ADDR_CMD, cmd);
		IOWR_32DIRECT(SCHEDULER_0_BASE, ADDR_PRIORITY, priority);
		IOWR_32DIRECT(SCHEDULER_0_BASE, ADDR_PARAM, param);

		//wait done
		unsigned int status;
		do {
			status = IORD_32DIRECT(SCHEDULER_0_BASE, ADDR_STATUS);
		} while( (status & STAT_DONE) == 0);

		//check error
		if (status & STAT_ERROR) {
			return -1;
		}

		//return
		return IORD_32DIRECT(SCHEDULER_0_BASE, ADDR_RETURN);
	}

public:
//	typedef T Object;

	Scheduler(){
		reset();
		alt_putstr("[START2]\n");
		alt_ic_isr_register(SCHEDULER_0_IRQ_INTERRUPT_CONTROLLER_ID, SCHEDULER_0_IRQ,  &(Scheduler::int_handler), 0, 0);
		alt_ic_irq_enable(SCHEDULER_0_IRQ_INTERRUPT_CONTROLLER_ID, SCHEDULER_0_IRQ);

	}

	//reset hardware
	void reset() {
		execute_cmd(CMD_RESET, 0, 0);
	}

//	static void int_handler (void *context, alt_u32 id) {
	//interrupt handler
	static void int_handler(void* context) {


		//acknowledgement to hardware
		IOWR_32DIRECT(ADDR_CMD, 0, CMD_INT_ACK);
		IOWR_32DIRECT(ADDR_PARAM, 0, 0);
		alt_putstr("Interrupt!\n");

		//process to chose_another();
		T* obj = (T*)execute_cmd(CMD_REMOVE_HEAD, 0, 0);

		T* running = (T*)execute_cmd(CMD_CHOSEN, 0, 0);
		int running_tid = execute_cmd(CMD_GETID, 0, (int)running);
		int priority = running->priority;
		execute_cmd(CMD_INSERT, priority, running_tid);

		int obj_tid = execute_cmd(CMD_GETID, 0, (int)obj);
		execute_cmd(CMD_UPDATE_RUNNING, 0, obj_tid);

		//execute thread
		obj->execute();

		//original implementation
		//volatile unsigned int * sch isr = (unsigned int *)(0x41400220);
		//	 *sch_isr = *sch_isr ; // ack
		//	 Thread:: time_reschedule();

	}

	//size of ready threads queue
	unsigned int schedulables() {
		return execute_cmd(CMD_SIZE, 0, 0);
	}

	//return pointer to running thread
	T* volatile chosen() {
		T* obj = (T*)execute_cmd(CMD_CHOSEN, 0, 0);
		return const_cast <T* volatile>(obj);
	}

	//creat new thread
	void insert (T* obj) {
		int priority = obj->priority;
		//create T
		int tid = execute_cmd(CMD_CREATE, priority, (int)obj);
		//if not error (queue full)
		if(tid > 0 ){
			//if no one is running
			if(!chosen()) {
				//update running thread
				execute_cmd(CMD_UPDATE_RUNNING, 0, tid);
			} else {
				//just insert in ready queue
				execute_cmd(CMD_INSERT, priority, tid);
			}
		}
	}

	//remove object - destroy reference in hardware table
	T* remove(T* obj) {
		int tid = execute_cmd(CMD_GETID, 0, (int)obj);
		if(tid < 0) {
			//error
			return 0;
		}
		//thread exist!
		//get running thread
		T* running = chosen();
		execute_cmd(CMD_DESTROY, 0, tid);
		//if removed thread is running, change running thread
		if(obj == running) {
			running = (T*)execute_cmd(CMD_REMOVE_HEAD, 0, 0);
			tid = execute_cmd(CMD_GETID, 0, (int)running);
			execute_cmd(CMD_UPDATE_RUNNING, 0, tid);
		}
		return obj;
	}

	//suspende thread - remove from ready queue
	void suspend(T* obj) {
		int tid = execute_cmd(CMD_GETID, 0, (int)obj);
		if(tid < 0) {
			return;
		}
		//thread exist!
		//remove from ready queue
		execute_cmd(CMD_REMOVE, 0, tid);
		//if suspended thread is running, change running thread
		if(obj == chosen()){
			int head = execute_cmd(CMD_REMOVE_HEAD, 0, 0);
			tid = execute_cmd(CMD_GETID, 0, head);
			execute_cmd(CMD_UPDATE_RUNNING, 0, tid);
		}
	}

	//insert a created thread in ready queue
	void resume(T* obj) {
		int tid = execute_cmd(CMD_GETID, 0, (int)obj);
		if(tid) {
			//insert in ready queue if thread exist.
			int priority = obj->priority;
			execute_cmd(CMD_INSERT, priority, tid);
		}
	}

	//change running thread
	//if the running thread have the most priority, will run again
	T* choose() {
		T* obj = chosen();
		int priority = obj->priority;
		//insert on ready queue
		int obj_tid = execute_cmd(CMD_GETID, 0, (int)obj);
		execute_cmd(CMD_INSERT, priority, obj_tid);

		//update running thread
		obj = (T*)execute_cmd(CMD_REMOVE_HEAD, 0, 0);
		obj_tid = execute_cmd(CMD_GETID, 0, (int)obj);
		execute_cmd(CMD_UPDATE_RUNNING, 0, obj_tid);

		//return running thread
		return obj;
	}

	//change running thread
	//force to next
	T* choose_another() {
		//remove head
		T* obj = (T*)execute_cmd(CMD_REMOVE_HEAD, 0, 0);

		//put running in ready queue
		T* running = chosen();
		int running_tid = execute_cmd(CMD_GETID, 0, (int)running);
		int priority = running->priority;
		execute_cmd(CMD_INSERT, priority, running_tid);

		//set old head to run
		int obj_tid = execute_cmd(CMD_GETID, 0, (int)obj);
		execute_cmd(CMD_UPDATE_RUNNING, 0, obj_tid);

		return obj;
	}

	//force the thread to run
	T* choose(T* obj) {
		int obj_tid = execute_cmd(CMD_GETID, 0, (int)obj);
		if(!obj_tid) {
			//error - thread don't exist
			return 0;
		}
		if(!execute_cmd(CMD_REMOVE, 0, obj_tid)) {
			return 0;
		}

		T* running = chosen();
		int running_tid = execute_cmd(CMD_GETID, 0, (int)running);
		int priority = running->priority;
		execute_cmd(CMD_INSERT, priority, running_tid);
		execute_cmd(CMD_UPDATE_RUNNING, 0, obj_tid);

		return obj;
	}

	//reset quantum
	void reset_quantum() {
		execute_cmd(CMD_RSTICKS, 0, 0);
	}

	//set quantum
	void set_quantum(int quantum) {
		execute_cmd(CMD_SET_QUANTUM, 0, quantum);
		int status = IORD_32DIRECT(ADDR_STATUS, 0);
		alt_printf("Status = %x\n", status);
	}

	//enable interruption from scheduler
	void enable_schedule() {
		execute_cmd(CMD_ENABLE, 0, 0);
	}

	//disnable interruption from scheduler
	void disable_schedule() {
		execute_cmd(CMD_DISABLE, 0, 0);
	}
};

