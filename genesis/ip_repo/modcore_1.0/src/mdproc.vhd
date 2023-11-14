----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 09/22/2020 08:52:15 PM
-- Design Name: 
-- Module Name: mdproc - Behavioral
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
use IEEE.numeric_std.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity mdproc is
Port ( 
    clk: in std_logic;
    g_reset: in std_logic;
    mdproc_data_in: in std_logic_vector(31 downto 0);
    mdproc_data_in_valid: in std_logic;
    mdproc_data_out_req: in std_logic;
    mdproc_data_out: out std_logic_vector(31 downto 0);
    mdproc_data_out_valid: out std_logic
);
end mdproc;

architecture Behavioral of mdproc is
    type mdproc_state is (idle, data_in1, data_out1);
    signal cur_state: mdproc_state;
	type buff_type is array (0 to 15) of std_logic_vector(31 downto 0);
	signal srcbuff : buff_type := (others => (others =>'0'));
	signal sig_mdproc_data_out: std_logic_vector(31 downto 0);
	signal sig_mdproc_data_out_valid: std_logic;
	signal sig_srcbuff_0: std_logic_vector(31 downto 0);
--	attribute MARK_DEBUG: string;
--	attribute MARK_DEBUG of cur_state: signal is "TRUE";
--	attribute MARK_DEBUG of mdproc_data_in: signal is "TRUE";
--	attribute MARK_DEBUG of mdproc_data_in_valid: signal is "TRUE";
--	attribute MARK_DEBUG of mdproc_data_out: signal is "TRUE";
--	attribute MARK_DEBUG of mdproc_data_out_valid: signal is "TRUE";
--	attribute MARK_DEBUG of sig_srcbuff_0: signal is "TRUE";
--	attribute MARK_DEBUG of mdproc_data_out_req: signal is "TRUE";
begin
    
    mdproc_data_out <= sig_mdproc_data_out;
    mdproc_data_out_valid <= sig_mdproc_data_out_valid;
	sig_srcbuff_0 <= srcbuff(0);
    
    process (clk) is
        variable srcbuffidx: integer;
    begin
        if rising_edge(clk) then
            if (g_reset = '1') then
                sig_mdproc_data_out_valid <= '0';
                srcbuffidx := 0;
                cur_state <= idle;
            else 
                case cur_state is                
                when idle =>
                    if (mdproc_data_in_valid = '1') then
                        srcbuff(srcbuffidx) <= mdproc_data_in;
                        srcbuffidx := srcbuffidx + 1;
                        cur_state <= data_in1;
                    elsif (mdproc_data_out_req = '1') then
                       srcbuffidx := 0;
                       cur_state <= data_out1;
                    else 
                        sig_mdproc_data_out_valid <= '0';
                        srcbuffidx := 0;
                    end if;
                when data_in1 =>
                    srcbuff(srcbuffidx) <= mdproc_data_in;
                    if (srcbuffidx = 15) then
                        srcbuffidx := 0;
                        cur_state <= idle;
                    else 
                        srcbuffidx := srcbuffidx + 1;
                        cur_state <= data_in1;
                    end if;
                    
                when data_out1 => 
                    if (srcbuffidx < 16) then
                        sig_mdproc_data_out <= std_logic_vector(255 - unsigned(srcbuff(srcbuffidx)(31 downto 24))) & 
                                                std_logic_vector(255 - unsigned(srcbuff(srcbuffidx)(23 downto 16))) & 
                                                std_logic_vector(255 - unsigned(srcbuff(srcbuffidx)(15 downto 8))) & 
                                                std_logic_vector(255 - unsigned(srcbuff(srcbuffidx)(7 downto 0)));
                    end if;
                    if (srcbuffidx = 16) then
                        sig_mdproc_data_out_valid <= '0';
                        srcbuffidx := 0;
                        cur_state <= idle;
                    else
                        sig_mdproc_data_out_valid <= '1'; 
                        srcbuffidx := srcbuffidx + 1;
                        cur_state <= data_out1;
                    end if;                     
                end case;
            end if;
        end if;
    end process;

end Behavioral;
