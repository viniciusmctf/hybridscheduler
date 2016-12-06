LIBRARY ieee;
USE ieee.std_logic_1164.all;
ENTITY scheduler_interface IS
	Generic ( 
		C_MAX_THREADS: integer := 8;
		C_DWIDTH:      integer := 32
	);
	PORT ( clock, resetn : IN STD_LOGIC;
		read, write, chipselect : IN STD_LOGIC;
		address : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
		writedata : IN STD_LOGIC_VECTOR(C_DWIDTH-1 DOWNTO 0);
		readdata : OUT STD_LOGIC_VECTOR(C_DWIDTH-1 DOWNTO 0);
		interrupt : out std_logic
	);
END scheduler_interface;

ARCHITECTURE Structure OF scheduler_interface IS
	signal local_command: 	std_logic_vector (3 downto 0);
	signal local_priority:	std_logic_vector (C_DWIDTH-1 DOWNTO 0);
	signal local_parameter:	std_logic_vector (C_DWIDTH-1 DOWNTO 0);
	
	signal local_return:		std_logic_vector (C_DWIDTH-1 DOWNTO 0);
	signal local_status:		std_logic_vector (5 downto 0);
	
	component scheduler_adapter IS
	Generic ( 
		C_MAX_THREADS: integer := 8;
		C_DWIDTH:      integer := 32
	);
		port ( clock, resetn : IN STD_LOGIC;
			read, write, chipselect : IN STD_LOGIC;
			address : 		IN STD_LOGIC_VECTOR(2 DOWNTO 0);
			writedata : 	IN STD_LOGIC_VECTOR(C_DWIDTH-1 DOWNTO 0);			
			
			i_return: 		in std_logic_vector (C_DWIDTH-1 DOWNTO 0);
			i_status:		in std_logic_vector (5 downto 0);
			i_command: 		out std_logic_vector (3 downto 0);
			i_priority:		out std_logic_vector (C_DWIDTH-1 DOWNTO 0);
			i_parameter:	out std_logic_vector (C_DWIDTH-1 DOWNTO 0);
			
			readdata : 		OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
		);
	end component;
	
	component Scheduler is
	Generic ( 
		C_MAX_THREADS: integer := 8;
		C_DWIDTH:      integer := 32
	);
	Port ( 
		p_clk: 			in std_logic;
		p_reset: 		in std_logic;
		p_command: 		in std_logic_vector (3 downto 0);
		p_priority:		in std_logic_vector (C_DWIDTH-1 downto 0);
		p_parameter:	in std_logic_vector (C_DWIDTH-1 downto 0);

		p_return:		out std_logic_vector (C_DWIDTH-1 downto 0);
		p_status:		out std_logic_vector (5 downto 0);
		p_interrupt:	out std_logic
	);	
	end component;
	
BEGIN
	
	Adapter_block: scheduler_adapter generic map (C_MAX_THREADS, C_DWIDTH) port map ( clock, resetn, read, write, chipselect, address, writedata,
	local_return, local_status, 
	local_command, local_priority, local_parameter,
	readdata
	);
	
	Scheduler_block: Scheduler generic map (C_MAX_THREADS, C_DWIDTH) port map (clock, resetn, local_command, local_priority, local_parameter,
					local_return, local_status, interrupt);
					
					
END Structure;