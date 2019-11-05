----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 09/21/2019 02:42:58 PM
-- Design Name: 
-- Module Name: RdBuff - Behavioral
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

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity RdBuff is
    Port (
         CLK: in std_logic;
         RDBUFF_G_START: in std_logic;
         RDATA: in std_logic_vector(31 downto 0);
         RDATA_VALID: in std_logic;
         RDBUFF_ALMOST_FULL: out std_logic;
         RDBUFF_EMPTY: out std_logic;
         OUTDATA: out std_logic_vector (31 downto 0);
         OUTVALID: out std_logic;
         OUTREQ: in std_logic
    );
end RdBuff;

architecture Behavioral of RdBuff is
    type t_RDBUFF_ENTRY is array (255 downto 0) of std_logic_vector(511 downto 0); -- 128Kb (64Byte) buff
    signal sig_RDBuff: t_RDBUFF_ENTRY;
    signal sig_inCnt: integer;
    signal sig_outCnt: integer;
    signal sig_in_beat_idx: integer;
    signal sig_out_beat_idx: integer;
    signal sig_outdata: std_logic_vector (31 downto 0);
    signal sig_outvalid: std_logic;
    signal sig_outreq: std_logic;
    signal sig_inIdx: integer;
    signal sig_outIdx: integer;
    
    signal sig_rdbuff_almost_full: std_logic;
    signal sig_rdbuff_empty: std_logic;  
    
   	attribute MARK_DEBUG: string;
	attribute MARK_DEBUG of sig_inCnt: signal is "TRUE";
	attribute MARK_DEBUG of sig_outCnt: signal is "TRUE"; 
begin

    OUTDATA <= sig_outdata;
    OUTVALID <= sig_outvalid;
    RDBUFF_ALMOST_FULL <= sig_rdbuff_almost_full;
    RDBUFF_EMPTY <= sig_rdbuff_empty;
    
    process (CLK) is
    begin
        if (rising_edge(CLK)) then
            if (RDBUFF_G_START = '0') then
                sig_inCnt <= 0;
                sig_inIdx <= 0;
                sig_in_beat_idx <= 0;
            else 
                if (RDATA_VALID = '1' AND sig_rdbuff_almost_full = '0') then
                    sig_RDBuff(sig_inIdx)(sig_in_beat_idx * 32 + 31 downto sig_in_beat_idx * 32) <= RDATA;
                    sig_in_beat_idx <= sig_in_beat_idx + 1;
                    if (sig_in_beat_idx = 16) then
                        sig_in_beat_idx <= 0;
                        sig_inCnt <= sig_inCnt + 1;
                        sig_inIdx <= to_integer(unsigned(std_logic_vector(to_unsigned(sig_inCnt,32)) AND x"0000_00FF"));
                    end if;
                end if;
            end if;
        end if;
    end process;
    
    process (CLK) is
    begin
        if (rising_edge(CLK)) then
            if (RDBUFF_G_START = '0') then
                sig_outCnt <= 0;
                sig_outIdx <= 0;
                sig_out_beat_idx <= 0;
            else
                if (OUTREQ = '1' AND sig_rdbuff_empty = '0') then
                    sig_outdata <= sig_RDBuff(sig_outIdx)(sig_out_beat_idx * 32 + 31 downto sig_out_beat_idx * 32);
                    sig_outvalid <= '1';
                    sig_out_beat_idx <= sig_out_beat_idx + 1;
                    if (sig_out_beat_idx = 16) then
                        sig_out_beat_idx <= 0;
                        sig_outCnt <= sig_outCnt + 1;
                        sig_outIdx <= to_integer(unsigned(std_logic_vector(to_unsigned(sig_outCnt, 32)) AND x"0000_00FF"));
                    end if;
                else 
                    sig_outvalid <= '0';
                end if;
            end if;
        end if;        
    end process;
    
    process (CLK) is
    begin
        if (rising_edge(CLK)) then
            if (RDBUFF_G_START = '0') then
                sig_rdbuff_almost_full <= '0';
                sig_rdbuff_empty <= '0';
            else
                if (sig_inCnt = sig_outCnt + 150) then
                    sig_rdbuff_almost_full <= '0';
                elsif (sig_inCnt = sig_outCnt + 200) then
                    sig_rdbuff_almost_full <= '1';
				elsif (sig_inCnt = sig_outCnt) then
					sig_rdbuff_empty <= '1';
                else
                    sig_rdbuff_empty <= '0';
                    sig_rdbuff_almost_full <= '0';
                end if;
            end if;
        end if;   
    end process;

end Behavioral;
