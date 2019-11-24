----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 09/26/2019 09:32:16 PM
-- Design Name: 
-- Module Name: DataConsumer - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity DataConsumer is
    Port (
        CLK: in std_logic;
--        RST: in std_logic;
        DATA_STREAM_START: in std_logic;
        DATA_IN: in std_logic_vector(31 downto 0);
        DATA_VALID: in std_logic;
        DATA_REQ: out std_logic;
        DATA_CONSUME_LATENCY: in std_logic_vector(31 downto 0);
        DATA_CONSUMER_START: in std_logic
    );
end DataConsumer;

architecture Behavioral of DataConsumer is
    signal sig_data: std_logic_vector(31 downto 0);
    signal sig_data_req: std_logic;
    
    attribute MARK_DEBUG: string;
    attribute MARK_DEBUG of DATA_IN: signal is "TRUE";
    attribute MARK_DEBUG of DATA_VALID: signal is "TRUE";
    attribute MARK_DEBUG of DATA_REQ: signal is "TRUE";
    attribute MARK_DEBUG of DATA_CONSUMER_START: signal is "TRUE";
begin
    DATA_REQ <= sig_data_req;
    
    process (CLK) is
    begin
        if (rising_edge(CLK)) then
            if (DATA_CONSUMER_START = '0') then
                sig_data <= (others => '0');
            elsif (DATA_VALID = '1') then
                sig_data <= DATA_IN;
            else
                sig_data <= (others => '0');
            end if;
        end if;
    end process;
    
    process (CLK) is
        variable interval: integer := 0;
    begin
        if (rising_edge(CLK)) then
            if (DATA_CONSUMER_START = '0') then
                sig_data_req <= '0';
                interval := 0;
            else
                if (interval >= to_integer(unsigned(DATA_CONSUME_LATENCY)) AND interval <= to_integer(unsigned(DATA_CONSUME_LATENCY)) + 15) then
                    sig_data_req <= '1';
                    if (interval = to_integer(unsigned(DATA_CONSUME_LATENCY)) + 15) then
                        interval := 0;
                    else
                        interval := interval + 1;
                    end if;
				else
				    sig_data_req <= '0';
					interval := interval + 1;
                end if;
            end if;
        end if;
    end process;
end Behavioral;
