LIBRARY ieee;
USE ieee.std_logic_1164.all;
entity scheduler_adapter is
Generic ( 
		C_MAX_THREADS: integer := 8;
		C_DWIDTH:      integer := 32
	);
	port ( 
		clock, resetn : in std_logic;
		read, write, chipselect : in std_logic;
		address:	in std_logic_vector(2 downto 0);
		writedata:	in std_logic_vector(C_DWIDTH-1 downto 0);			
			
		i_return: 	in std_logic_vector (C_DWIDTH-1 downto 0);
		i_status:	in std_logic_vector (5 downto 0);
		i_command: 	out std_logic_vector (3 downto 0);
		i_priority:	out std_logic_vector (C_DWIDTH-1 downto 0);
		i_parameter:	out std_logic_vector (C_DWIDTH-1 downto 0);
			
		readdata: 	out std_logic_vector(31 downto 0)
		);
end scheduler_adapter;

architecture Structure of scheduler_adapter is
	signal reg_command  : 	std_logic_vector (C_DWIDTH-1 downto 0);
	signal reg_parameter: 	std_logic_vector (C_DWIDTH-1 downto 0);
	signal reg_priority :	std_logic_vector (C_DWIDTH-1 downto 0);
	signal status_32    :   std_logic_vector (C_DWIDTH-1 downto 0);
	
	signal load   :	STD_LOGIC;
begin	
	status_32(C_DWIDTH-1 downto 6) <= (others => '0');
	status_32(5 downto 0) <= i_status;

	process (clock, resetn)
	begin
		--reseta todos os sinais
		if resetn = '0' then
				reg_command <= (others => '0');
				reg_priority <= (others => '0');
				reg_parameter <= (others => '0');
				load <= '0';
		else 
			if (rising_edge(clock)) then
				if(chipselect = '1') then
					if(write = '1' and read = '0') then
						case address is
							when "000" =>
								reg_command <= writedata;
							when "001" =>
								reg_priority <= writedata;
							when "010" =>
								reg_parameter <= writedata;
								load <= '1';
							when others => null;
 
						end case;
					elsif(write = '0' and read = '1') then
							case address is
								--verificar se os valores de address estao corretos
								when "011" =>
									readdata <= i_return;
								when "100" =>
									readdata <= status_32;
								when others => null;
							end case;
					else	
						load <= '0';
					end if;
				else	
					load <= '0';
				end if;
			end if;
		end if;
	end process;
	
	--input pronto para ser enviado para a arquitetura.
	process (clock)
	begin
		--reseta todos os sinais
		if (rising_edge(clock)) then
			if(load = '1') then
				i_command <= reg_command(3 downto 0);
				i_parameter <= reg_parameter(C_DWIDTH-1 downto 0);
				i_priority <= reg_priority(C_DWIDTH-1 downto 0);		
			else
				i_command <= (others => '0');
				i_parameter <= (others => '0');
				i_priority <= (others => '0');
			end if;
		end if;
	end process;
END Structure;