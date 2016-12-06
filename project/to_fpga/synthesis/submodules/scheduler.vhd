---------------------------------------------------------
-- Company: Software and Hardware Integration Lab - LISHA
-- Engineer: Hugo Marcondes
-- Design Name: Hardware Scheduler
-- Project Name: Hybrid Hw/Sw Components
---------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity Scheduler is
	Generic ( 
		C_MAX_THREADS: integer := 8;
		C_DWIDTH:      integer := 32
	);

	Port ( 
		p_clk: 			in std_logic;
		p_reset: 		in std_logic;
		p_command: 		in std_logic_vector (3 downto 0);
		p_priority:		in std_logic_vector (31 downto 0);
		p_parameter:	in std_logic_vector (C_DWIDTH-1 downto 0);

		p_return:		out std_logic_vector (C_DWIDTH-1 downto 0);
		p_status:		out std_logic_vector (5 downto 0);
		p_interrupt:	out std_logic
	);
end Scheduler;

architecture Behavioral of Scheduler is
	-- Scheduler Commands
	constant C_CMD_CREATE: 			std_logic_vector (3 downto 0) := "0001";
	constant C_CMD_DESTROY: 		std_logic_vector (3 downto 0) := "0010";
	constant C_CMD_INSERT: 			std_logic_vector (3 downto 0) := "0011";
	constant C_CMD_REMOVE: 			std_logic_vector (3 downto 0) := "0100";
	constant C_CMD_REMOVE_HEAD: 	std_logic_vector (3 downto 0) := "0101";
	constant C_CMD_UPDATE_RUNNING: 	std_logic_vector (3 downto 0) := "0110";
	constant C_CMD_SET_QUANTUM:		std_logic_vector (3 downto 0) := "0111";
	constant C_CMD_ENABLE: 			std_logic_vector (3 downto 0) := "1000";
	constant C_CMD_DISABLE: 		std_logic_vector (3 downto 0) := "1001";
	constant C_CMD_INT_ACK: 		std_logic_vector (3 downto 0) := "1010";
	constant C_CMD_GETID: 			std_logic_vector (3 downto 0) := "1011";
	constant C_CMD_CHOSEN: 			std_logic_vector (3 downto 0) := "1100";
	constant C_CMD_SIZE: 			std_logic_vector (3 downto 0) := "1101";
	constant C_CMD_RSTICKS: 		std_logic_vector (3 downto 0) := "1110";
	constant C_CMD_RESET: 		std_logic_vector (3 downto 0) := "1111";

	-- State Machine Type
	type t_state is ( idle, destroy, preinsert, insert, remove, exit_ok, exit_error, acknowledge_int, reset_time, pregetid, getid, reset );
	signal s_state : t_state ;


	-- Internal Memory
	type t_obj_table 	is array (0 to C_MAX_THREADS-1) of std_logic_vector(C_DWIDTH-1 downto 0);
	type t_order_table 	is array (0 to C_MAX_THREADS-1) of std_logic_vector(15 downto 0);
	type t_linkedlist 	is array (0 to C_MAX_THREADS-1) of integer range 0 to C_MAX_THREADS;

	signal s_obj_table:			t_obj_table;
	signal s_order_table:		t_order_table;
	signal s_prev_table:		t_linkedlist;
	signal s_next_table:		t_linkedlist;
	signal s_tid_bitmap:		std_logic_vector (C_MAX_THREADS-1 downto 0);
	signal s_enqueue_bitmap:	std_logic_vector (C_MAX_THREADS-1 downto 0);

	signal s_running_tid:		integer range 0 to C_MAX_THREADS:=0;		
	signal s_command_tid:		integer range 0 to C_MAX_THREADS:=0;
	signal s_free_tid:			integer range 0 to C_MAX_THREADS:=0;
	signal s_head_tid:			integer range 0 to C_MAX_THREADS:=0;
	signal s_tail_tid:			integer range 0 to C_MAX_THREADS:=0;
	signal s_size:				integer range 0 to C_MAX_THREADS:=0;
	signal s_list_empty:		std_logic;

	-- Status Signals
	signal s_done:				std_logic;
	signal s_error:				std_logic;
	signal s_full:				std_logic;

	--search TID signals

	signal s_stid_obj_ptr:		std_logic_vector (C_DWIDTH-1 downto 0);
	signal s_stid_done:			std_logic;
	signal s_stid_start:		std_logic;
	signal s_stid_found:		integer range 0 to C_MAX_THREADS:=0;

	--searchingLinkList signals
	signal s_search_reset:		std_logic;
	signal s_search_order:		std_logic_vector (31 downto 0);
	signal s_found_tid: 		integer range 0 to C_MAX_THREADS:=0;
	signal s_search_done:		std_logic;
	signal s_return:			std_logic_vector (C_DWIDTH-1 downto 0);

	--timeManagement signals
	signal s_quantum_ticks:		std_logic_vector (C_DWIDTH-1 downto 0);
	signal s_schedule_enabled: 	std_logic;
	signal s_reschedule:		std_logic;
	signal s_int_ack:			std_logic;
	signal s_time_reset:		std_logic;
	signal s_counter: 			std_logic_vector (C_DWIDTH-1 downto 0);
begin

	s_list_empty 	<=	'1' when s_head_tid = 0 else '0';
	p_return 		<=	s_return;
	p_status 		<=	s_schedule_enabled & s_reschedule & s_done & s_full & s_list_empty & s_error;

	-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	-- Controls the allocation of tid to new threads .
	SearchFreeTID: process ( p_clk, p_reset )

	variable v_or_vector:	std_logic_vector (C_MAX_THREADS downto 0);
	begin
		if p_clk 'event and p_clk = '1' then
			if p_reset = '0' then
				s_free_tid 	<= 1;
				s_full 		<= '0';
			else
				v_or_vector(0):= '0' or (not s_tid_bitmap(0));
				for row in 1 to C_MAX_THREADS-1 loop
					v_or_vector(row) := v_or_vector(row-1) or (not s_tid_bitmap(row)) ;
				end loop;
				s_full <= not v_or_vector(C_MAX_THREADS-1);
				for row in 1 to C_MAX_THREADS loop
					if ( s_tid_bitmap(row-1)='0') then
						s_free_tid <= row;
					end if;
				end loop;
			end if;
		end if;
	end process;
	-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	-- Main processes responsible to control the state machine

	mainProcess: process ( p_clk , p_reset , p_command)
	begin
		if p_clk 'event and p_clk = '1' then
			-- Reset Handling
			if p_reset = '0' then
				s_tid_bitmap <= (others => '0');
				s_enqueue_bitmap <= (others => '0');
				for i in 0 to C_MAX_THREADS-1 loop
					s_obj_table(i) 		<= (others => '0');
					s_order_table(i) 	<= (others => '0');
					s_prev_table(i) 	<= 0;
					s_next_table(i) 	<= 0;
				end loop;
				s_running_tid 		<= 0;
				s_head_tid 			<= 0;
				s_tail_tid 			<= 0;
				s_return 			<= (others => '0');
				s_done 				<= '0';
				s_error 			<= '0';
				s_schedule_enabled 	<= '0';
				s_quantum_ticks 	<= (others => '1');
				s_int_ack 			<= '0';
				s_state 			<= idle;
				s_size <= 0;
			else
				-- Finite State Machine Controller
				case s_state is
					when idle =>
						case p_command is
							when C_CMD_CREATE =>
								s_done <= '0';
								s_error <= '0';
								if ( s_full = '0') then
									s_command_tid 	<= s_free_tid ;
									s_tid_bitmap( s_free_tid-1) 	<= '1';
									s_order_table ( s_free_tid-1) 	<= p_priority;
									s_obj_table ( s_free_tid-1) 	<= p_parameter(C_DWIDTH-1 downto 0);
									s_return 		<= conv_std_logic_vector (s_free_tid, C_DWIDTH);
									s_state 		<= exit_ok;
								else
									s_state <= exit_error ;
								end if ;
	
							when C_CMD_DESTROY =>
								s_done 			<= '0';
								s_error 		<= '0';
								s_command_tid 	<= conv_integer(p_parameter) ;
								s_state 		<= destroy;
	
							when C_CMD_INSERT =>
								s_done 			<= '0';
								s_error 		<= '0';
								s_command_tid 	<= conv_integer(p_parameter);
								s_order_table(conv_integer(p_parameter)-1) 		<= p_priority;
								s_enqueue_bitmap(conv_integer(p_parameter)-1) 	<= '1';
								
								if ( s_head_tid = 0) then
									--Insiro direto na cabeca da fila
									s_head_tid 	<= conv_integer( p_parameter );
									s_tail_tid 	<= conv_integer( p_parameter );
									s_size 		<= s_size + 1;
									s_state 	<= exit_ok;
								else -- Procuro posicao
									s_search_order 	<= p_priority ;
									s_search_reset 	<= '1';
									s_state		 	<= preinsert ;
								end if;
	
							when C_CMD_REMOVE =>
								s_done 	<= '0';
								s_error <= '0';
								if (s_enqueue_bitmap( conv_integer ( p_parameter )-1) = '0') then -- Not in queue!
									s_return 	<= (others => '0');
									s_state 	<= exit_ok;
								else
									s_command_tid 	<= conv_integer(p_parameter ) ;
									s_state 		<= remove;
								end if;
	
							when C_CMD_REMOVE_HEAD =>
								s_done 	<= '0';
								s_error <= '0';
								if ( s_head_tid = 0) then -- There is no HEAD !
									s_return <= (others => '0');
									s_state <= exit_ok;
								else
									s_command_tid <= s_head_tid;
									s_state <= remove;
								end if ;
	
							when C_CMD_UPDATE_RUNNING =>
								s_done 	<= '0';
								s_error <= '0';
								s_running_tid <= conv_integer(p_parameter);
								s_state <= exit_ok;
	
							when C_CMD_SET_QUANTUM =>
								s_done 	<= '0';
								s_error <= '0';
								s_quantum_ticks <= p_parameter;
								s_time_reset 	<= '1';
								s_state <= reset_time;
	
							when C_CMD_RSTICKS =>
								s_done 	<= '0';
								s_error <= '0';
								s_time_reset <= '1';
								s_state <= reset_time;
	
							when C_CMD_ENABLE =>
								s_done 	<= '0';
								s_error <= '0';
								s_schedule_enabled <= '1';
								s_state <= exit_ok;
	
							when C_CMD_DISABLE =>
								s_done 	<= '0';
								s_error <= '0';
								s_schedule_enabled <= '0';
								s_state <= exit_ok;
	
							when C_CMD_INT_ACK =>
								s_done 	<= '0';
								s_error <= '0';
								s_int_ack <= '1';
								s_state <= acknowledge_int;
	
							when C_CMD_GETID =>
								s_done 	<= '0';
								s_error <= '0';
								s_stid_obj_ptr <= p_parameter;
								s_stid_start <= '1';
								s_state <= pregetid;
	
							when C_CMD_CHOSEN =>
								s_done 	<= '0';
								s_error <= '0';
								if ( s_running_tid = 0) then
									s_return <= (others=>'0');
								else
									s_return <= s_obj_table ( s_running_tid-1);
								end if ;
								s_state <= exit_ok;
	
							when C_CMD_SIZE =>
								s_done <= '0';
								s_error <= '0';
								s_return <= conv_std_logic_vector ( s_size+1, C_DWIDTH);
								s_state <= exit_ok;
							
							when C_CMD_RESET =>
								s_tid_bitmap <= (others => '0');
								s_enqueue_bitmap <= (others => '0');
								for i in 0 to C_MAX_THREADS-1 loop
									s_obj_table(i) 		<= (others => '0');
									s_order_table(i) 	<= (others => '0');
									s_prev_table(i) 	<= 0;
									s_next_table(i) 	<= 0;
								end loop;
								s_running_tid 		<= 0;
								s_head_tid 			<= 0;
								s_tail_tid 			<= 0;
								s_return 			<= (others => '0');
								s_done 				<= '0';
								s_error 			<= '0';
								s_schedule_enabled 	<= '0';
								s_quantum_ticks 	<= (others => '1');
								s_int_ack 			<= '0';
								s_size <= 0;
								s_state <= exit_ok;
	
							when others => null;
						end case;
					
					when preinsert =>
						s_search_reset <= '0';
						s_state <= insert ;
					
					when insert =>
						if ( s_search_done = '1') then
							if ( s_found_tid = 0) then -- Insert in tail
								s_tail_tid <= s_command_tid;
								s_next_table ( s_tail_tid -1) 	<= s_command_tid;
								s_prev_table (s_command_tid-1) 	<= s_tail_tid ;
								s_next_table (s_command_tid-1) <= 0;
							else -- Insert in middle
								if ( s_prev_table ( s_found_tid -1) = 0) then -- inserting in the HEAD
									s_head_tid <= s_command_tid;
								end if ;
								s_next_table (s_command_tid-1) <= s_found_tid;
								s_prev_table (s_command_tid-1) <= s_prev_table( s_found_tid -1);
								if ( s_prev_table ( s_found_tid -1) > 0) then
									s_next_table ( s_prev_table ( s_found_tid -1)-1) <= s_command_tid;
								end if ;
								s_prev_table ( s_found_tid -1) <= s_command_tid;
							end if ;
							s_return <= conv_std_logic_vector (s_command_tid, C_DWIDTH);
							s_size <= s_size + 1;
							s_state <= exit_ok;
						end if ;
					
					when destroy =>
						s_tid_bitmap (s_command_tid-1) <= '0'; -- Free tid
						s_obj_table  (s_command_tid-1) <= (others => '0');
						s_order_table (s_command_tid-1) <= (others => '0');
						if (s_enqueue_bitmap(s_command_tid-1) = '1') then -- TID enqueue
							s_state <= remove;
						else
							s_state <= exit_ok;
						end if ;
					
					when remove =>
						s_return <= s_obj_table  (s_command_tid-1);
						s_enqueue_bitmap(s_command_tid-1) <= '0';
						if ( s_prev_table (s_command_tid-1) = 0) then -- is head
							s_head_tid <= s_next_table (s_command_tid-1);
						else
							s_next_table ( s_prev_table (s_command_tid-1)-1) <= s_next_table(s_command_tid-1);
						end if ;
						if ( s_next_table (s_command_tid-1) = 0) then -- is tail
							s_tail_tid <= s_prev_table (s_command_tid-1);
						else
							s_prev_table ( s_next_table (s_command_tid-1)-1) <= s_prev_table(s_command_tid-1);
						end if ;
						s_next_table (s_command_tid-1) <= 0;
						s_prev_table (s_command_tid-1) <= 0;
						s_size <= s_size - 1;
						s_state <= exit_ok;
						
					when exit_ok =>
						s_done <= '1';
						s_state <= idle;
					
					when exit_error =>
						s_done <= '1';
						s_error <= '1';
						s_state <= idle;
	
					when acknowledge_int =>
						s_done <= '1';
						s_int_ack <= '0';
						s_state <= idle;
					
					when reset_time =>
						s_done <= '1';
						s_time_reset <= '0';
						s_state <= idle;
					
					when pregetid =>
						s_stid_start <= '0';
						s_state <= getid;
					
					when getid =>
						if ( s_stid_done = '1') then
							if ( s_stid_found = 0) then
								s_return <= (others => '0');
								s_state <= exit_error ;
							else
								s_return <= conv_std_logic_vector ( s_stid_found , C_DWIDTH);
								s_state <= exit_ok;
							end if ;
						end if ;
						
					when others =>
						null ;
				end case;
			end if ;
		end if ;
	end process;
-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
-- Searchs the TID from the command obj
-- s_stid_obj_ptr : 	std_logic_vector (0 to C_DWIDTH-1);
-- s_stid_done: 		std_logic ;
-- s_stid_start : 		std_logic ;
-- s_stid_found: 		integer range 0 to C MAX THREADS := 0;
-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
SearchTID: process( p_clk , s_stid_start )
begin
	if p_clk 'event and p_clk = '1' then
		if s_stid_start = '1' then
			s_stid_found <= 1;
			s_stid_done <= '0';
		elsif s_stid_done = '0' then
			if ( s_obj_table  ( s_stid_found -1) = s_stid_obj_ptr ) then -- Achou
				s_stid_done <= '1'; -- Achou o TID da thread
			else
				if ( s_stid_found = C_MAX_THREADS) then -- N  ao tem
					s_stid_found <= 0;
					s_stid_done <= '1';
				else
					s_stid_found <= s_stid_found + 1; -- Vou para o pr  oximo
				end if ;
			end if ;
		end if ;
	end if ;
end process;
-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
-- Searchs the position of insertion in the queue
-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
searchingLinkList : process( p_clk , s_search_reset )
begin
	if p_clk 'event and p_clk = '1' then
	--s_search_done <= '0';
		if s_search_reset = '1' then
			s_found_tid <= s_head_tid;
			s_search_done <= '0';
		elsif s_search_done = '0' then
			if ( s_order_table ( s_found_tid -1) <= s_search_order) then -- Increment s_found_tid
				if ( s_next_table ( s_found_tid -1) = 0) then -- Se estou no tail ... sai com 0
					s_found_tid <= 0;
					s_search_done <= '1';
				else -- Vai para o pr  oximo da fila ...
					s_found_tid <= s_next_table ( s_found_tid -1);
				end if ;
			else
				s_search_done <= '1';
			end if ;
		end if ;
	end if ;
end process;
-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
-- Controls Rescheduling Interrupts
-- s_quantum_ticks:		std_logic_vector (0 to C_DWIDTH-1);
-- s schedule enabled: 	std_logic ;
-- s_reschedule:		std_logic ;
-- s_int_ack:			std_logic ;
-- s_time_reset:		std_logic ;
-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
-- Don't interrupt in the middle of an command! Wait command completation.
p_interrupt <= '1' when (s_reschedule = '1' and s_schedule_enabled = '1' and s_state = idle ) else '0';

timeManagement: process(p_clk, s_time_reset )
begin
	if p_clk 'event and p_clk = '1' then
		if s_time_reset = '1' then
			s_counter <= s_quantum_ticks;
			s_reschedule <= '0';
		else
			--if (s_reschedule = '1' and s schedule enabled = '1' and s_state = idle ) then
			if ( s_counter = 0) then
				if ( ( s_running_tid > 0) and
				 ( s_head_tid > 0) and
				 ( s_order_table ( s_running_tid -1) >= s_order_table( s_head_tid -1)) ) then
					-- Reschedule CPU
					s_reschedule <= '1';
				end if ;
				s_counter <= s_quantum_ticks;
			else
				s_counter <= s_counter - 1;
			end if ;

			if ( s_int_ack = '1') then
				s_reschedule <= '0';
				s_counter <= s_quantum_ticks;
			end if ;
		end if ;
	end if ;
end process;
end Behavioral ;