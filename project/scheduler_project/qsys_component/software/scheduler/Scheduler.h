
#include "Object.h"
#include "system.h"
#include "HAL/inc/io.h"
#include "sys/alt_stdio.h"
//template <typename T>
class Scheduler {
protected:

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

	static int execute_cmd(int cmd, int priority, int param) {
			//send command
			IOWR_32DIRECT(ADDR_CMD, 0, cmd);
			IOWR_32DIRECT(ADDR_PRIORITY, 0, priority);
			IOWR_32DIRECT(ADDR_PARAM, 0, param);

			//wait done
			unsigned int status;
			do {
				status = IORD_32DIRECT(ADDR_STATUS,0);
			} while( (status & STAT_DONE) == 0);

			if (status & STAT_ERROR) {
				return -1;
			}

			//se necessário, tratar status no contexto de quem invocou este método.
			return IORD_32DIRECT(ADDR_RETURN, 0);
		}

public:
	//	typedef T Object;

	Scheduler(){

	}

	void reset() {
		execute_cmd(CMD_RESET, 0, 0);
	}

//	static void int_handler (void *context, alt_u32 id) {
	static void int_handler(void* context) {
		//volatile unsigned int * sch isr = (unsigned int *)(0x41400220);

		IOWR_32DIRECT(ADDR_CMD, 0, CMD_INT_ACK);
		IOWR_32DIRECT(ADDR_PARAM, 0, 0);
		alt_putstr("Interrupt!\n");

		Object* obj = (Object*)execute_cmd(CMD_REMOVE_HEAD, 0, 0);

		Object* running = (Object*)execute_cmd(CMD_CHOSEN, 0, 0);
		int running_tid = execute_cmd(CMD_GETID, 0, (int)running);
		int priority = running->priority;
		execute_cmd(CMD_INSERT, priority, running_tid);

		int obj_tid = execute_cmd(CMD_GETID, 0, (int)obj);
		execute_cmd(CMD_UPDATE_RUNNING, 0, obj_tid);

		obj->execute();
		//	 *sch_isr = *sch_isr ; // ack
		//	 Thread:: time_reschedule();

	}

	unsigned int schedulables() {
			return execute_cmd(CMD_SIZE, 0, 0);
		}

		Object* volatile chosen() {
			Object* obj = (Object*)execute_cmd(CMD_CHOSEN, 0, 0);
			return const_cast <Object* volatile>(obj);
		}

		void insert (Object* obj) {
			int priority = obj->priority;
			int tid = execute_cmd(CMD_CREATE, priority, (int)obj);
			if(tid > 0 ){
				if(!chosen()) {
					execute_cmd(CMD_UPDATE_RUNNING, 0, tid);
				} else {
					execute_cmd(CMD_INSERT, priority, tid);
				}
			}
		}

		Object* remove(Object* obj) {
			int tid = execute_cmd(CMD_GETID, 0, (int)obj);
			if(tid < 0) { // a arquitetura retorna erro quando nao encontra id.
				obj = 0;
			} else {
				Object* running = chosen();
				execute_cmd(CMD_DESTROY, 0, tid);
				if(obj == running) {
					running = (Object*)execute_cmd(CMD_REMOVE_HEAD, 0, 0);
					tid = execute_cmd(CMD_GETID, 0, (int)running);
					execute_cmd(CMD_UPDATE_RUNNING, 0, tid);
				}
			}
			return obj;
		}

		void suspend(Object* obj) {
			int tid = execute_cmd(CMD_GETID, 0, (int)obj);
			if(tid < 0) {
				return;
			}
			execute_cmd(CMD_REMOVE, 0, tid);
			if(obj == chosen()){
				int head = execute_cmd(CMD_REMOVE_HEAD, 0, 0);
				tid = execute_cmd(CMD_GETID, 0, head);
				execute_cmd(CMD_UPDATE_RUNNING, 0, tid);
			}
		}

		void resume(Object* obj) {
			int tid = execute_cmd(CMD_GETID, 0, (int)obj);
			if(tid) {
				int priority = obj->priority;
				execute_cmd(CMD_INSERT, priority, tid);
			}
		}

		Object* choose() {
			Object* obj = chosen();
			int priority = obj->priority;
			int obj_tid = execute_cmd(CMD_GETID, 0, (int)obj);
			execute_cmd(CMD_INSERT, priority, obj_tid);

			obj = (Object*)execute_cmd(CMD_REMOVE_HEAD, 0, 0);
			obj_tid = execute_cmd(CMD_GETID, 0, (int)obj);

			execute_cmd(CMD_UPDATE_RUNNING, 0, obj_tid);

			return obj;
		}

		Object* choose_another() {
			Object* obj = (Object*)execute_cmd(CMD_REMOVE_HEAD, 0, 0);

			Object* running = chosen();
			int running_tid = execute_cmd(CMD_GETID, 0, (int)running);
			int priority = running->priority;
			execute_cmd(CMD_INSERT, priority, running_tid);

			int obj_tid = execute_cmd(CMD_GETID, 0, (int)obj);
			execute_cmd(CMD_UPDATE_RUNNING, 0, obj_tid);

			return obj;
		}

		Object* choose(Object* obj) {
			int obj_tid = execute_cmd(CMD_GETID, 0, (int)obj);
			if(!obj_tid) {
				obj = 0;
			} else {
				if(!execute_cmd(CMD_REMOVE, 0, obj_tid)) {
					obj = 0;
				} else {
					Object* running = chosen();
					int running_tid = execute_cmd(CMD_GETID, 0, (int)running);
					int priority = running->priority;
					execute_cmd(CMD_INSERT, priority, running_tid);
					execute_cmd(CMD_UPDATE_RUNNING, 0, obj_tid);

				}
			}
			return obj;
		}
		//
		void reset_quantum() {
			execute_cmd(CMD_RSTICKS, 0, 0);
		}

		void set_quantum(int quantum) {
			execute_cmd(CMD_SET_QUANTUM, 0, quantum);
			int status = IORD_32DIRECT(ADDR_STATUS, 0);
			alt_printf("Status = %x\n", status);
		}

		void enable_schedule() {
			execute_cmd(CMD_ENABLE, 0, 0);
			int status = IORD_32DIRECT(ADDR_STATUS, 0);
			alt_printf("Status = %x\n", status);
		}

		void disable_schedule() {
			execute_cmd(CMD_DISABLE, 0, 0);
//			int status = IORD_32DIRECT(ADDR_STATUS, 0);
//			alt_printf("Status = %x\n", status);
		}
	//

};

