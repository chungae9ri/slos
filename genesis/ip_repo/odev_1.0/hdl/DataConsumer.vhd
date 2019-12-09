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
        DATA_CONSUMER_START: in std_logic;
        DATA_RDBUFF_EMPTY: in std_logic;
        DATA_SEQ_ERR: out std_logic
    );
end DataConsumer;

architecture Behavioral of DataConsumer is
    signal sig_data: std_logic_vector(31 downto 0);
    signal sig_data_req: std_logic;
    signal sig_dc_seq_err: std_logic;
    signal sig_rdblkcnt: std_logic_vector(31 downto 0);
    constant int_wrap_around: integer := (4096 * 4); --4096 stream wrap-around
    
--    attribute MARK_DEBUG: string;
--    attribute MARK_DEBUG of DATA_IN: signal is "TRUE";
--    attribute MARK_DEBUG of sig_seq_err: signal is "TRUE";
--    attribute MARK_DEBUG of DATA_CONSUMER_START: signal is "TRUE";
--    attribute MARK_DEBUG of sig_rdblkcnt: signal is "TRUE";
begin
    DATA_REQ <= sig_data_req;
    DATA_SEQ_ERR <= sig_dc_seq_err;
    
    process (CLK) is
        variable rdcnt: integer;
        variable rdblkcnt: integer;
    begin
        if (rising_edge(CLK)) then
            if (DATA_STREAM_START = '0') then
                rdcnt := 0;
                rdblkcnt := 1; -- seq index starting from 0
                sig_dc_seq_err <= '0';
                sig_rdblkcnt <= (others => '0');
            else
                if (DATA_CONSUMER_START = '0') then
                    sig_data <= (others => '0');
                    sig_dc_seq_err <= '0';
                elsif (DATA_VALID = '1') then
                    sig_data <= DATA_IN;
                    if (rdcnt = 0) then
                        if (to_integer(unsigned(DATA_IN)) /= rdblkcnt) then
                           sig_dc_seq_err <= '1'; 
                           sig_rdblkcnt <= std_logic_vector(to_unsigned(rdblkcnt,32));
                        end if;
                    end if;
                    -- last 4B of 256B
                    if (rdcnt = 15) then
                        rdcnt := 0;
                        if (rdblkcnt = int_wrap_around) then
                            rdblkcnt := 1;
                        else
                            rdblkcnt := rdblkcnt + 1;
                        end if;
                    else
                        rdcnt := rdcnt + 1;
                    end if;                    
                else
                    sig_data <= (others => '0');
                    sig_dc_seq_err <= sig_dc_seq_err;
                end if;
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
            elsif (DATA_RDBUFF_EMPTY = '1') then
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
