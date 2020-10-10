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
--         RDBUFF_G_START: in std_logic;
         RDBUFF_STREAM_START: in std_logic;
         RDATA: in std_logic_vector(31 downto 0);
         RDATA_VALID: in std_logic;
         RDBUFF_FULL: out std_logic;
         RDBUFF_EMPTY: out std_logic;
         OUTDATA: out std_logic_vector (31 downto 0);
         OUTVALID: out std_logic;
         OUTREQ: in std_logic;
         RDBUFF_SEQ_ERR: out std_logic
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
    
    signal sig_rdbuff_full: std_logic;
    signal sig_rdbuff_empty: std_logic; 
    signal rdbuff_sig_seq_err: std_logic; 
    constant int_Rdbuff_wrap_around: integer := (4096 * 4); --4096 stream wrap-around
--   	attribute MARK_DEBUG: string;
--	attribute MARK_DEBUG of sig_inCnt: signal is "TRUE";
--	attribute MARK_DEBUG of sig_outCnt: signal is "TRUE"; 
--	attribute MARK_DEBUG of sig_rdbuff_full: signal is "TRUE"; 
--	attribute MARK_DEBUG of sig_outvalid: signal is "TRUE";
--	attribute MARK_DEBUG of RDATA_VALID: signal is "TRUE";
--	attribute MARK_DEBUG of rdbuff_sig_seq_err: signal is "TRUE";
--	attribute MARK_DEBUG of RDATA: signal is "TRUE";

begin

    OUTDATA <= sig_outdata;
    OUTVALID <= sig_outvalid;
    RDBUFF_FULL <= sig_rdbuff_full;
    RDBUFF_EMPTY <=  '0'; -- sig_rdbuff_empty;
    RDBUFF_SEQ_ERR <= rdbuff_sig_seq_err;
        
    process (CLK) is
        variable rdbuffblkcnt: integer;
    begin
        if (rising_edge(CLK)) then
            if (RDBUFF_STREAM_START = '0') then
                sig_inCnt <= 0;
                sig_inIdx <= 0;
                sig_in_beat_idx <= 0;
                rdbuff_sig_seq_err <= '0';
                rdbuffblkcnt := 1;
            else 
                if (RDATA_VALID = '1') then
                    sig_RDBuff(sig_inIdx)(sig_in_beat_idx * 32 + 31 downto sig_in_beat_idx * 32) <= RDATA;
                    sig_in_beat_idx <= sig_in_beat_idx + 1;
                    if (sig_in_beat_idx = 0 AND to_integer(unsigned(RDATA)) /= rdbuffblkcnt) then
                        rdbuff_sig_seq_err <= '1';
                    end if;
                    
                    if (sig_in_beat_idx = 15) then
                        sig_in_beat_idx <= 0;
                        sig_inCnt <= sig_inCnt + 1;
                        sig_inIdx <= to_integer(unsigned(std_logic_vector(to_unsigned(sig_inCnt + 1,32)) AND x"0000_00FF"));
                        if (rdbuffblkcnt = int_Rdbuff_wrap_around) then
                            rdbuffblkcnt := 1;
                        else
                            rdbuffblkcnt := rdbuffblkcnt + 1;
                        end if;
                    end if;
                end if;
            end if;
        end if;
    end process;
    
    process (CLK) is
    begin
        if (rising_edge(CLK)) then
            if (RDBUFF_STREAM_START = '0') then
                sig_outCnt <= 0;
                sig_outIdx <= 0;
                sig_out_beat_idx <= 0;
            else
                if (OUTREQ = '1' AND sig_rdbuff_empty = '0') then
                    sig_outdata <= sig_RDBuff(sig_outIdx)(sig_out_beat_idx * 32 + 31 downto sig_out_beat_idx * 32);
                    sig_outvalid <= '1';
                    sig_out_beat_idx <= sig_out_beat_idx + 1;
                    if (sig_out_beat_idx = 15) then
                        sig_out_beat_idx <= 0;
                        sig_outCnt <= sig_outCnt + 1;
                        sig_outIdx <= to_integer(unsigned(std_logic_vector(to_unsigned(sig_outCnt + 1, 32)) AND x"0000_00FF"));
                    end if;
                else 
                    sig_outvalid <= '0';
                    sig_outdata <= (others => '0');
                end if;
            end if;
        end if;        
    end process;
    
    process (CLK) is
    begin
        if (rising_edge(CLK)) then
            if (RDBUFF_STREAM_START = '0') then
                sig_rdbuff_full <= '0';
                sig_rdbuff_empty <= '0';
            else
                if (sig_inCnt = sig_outCnt) then
                    sig_rdbuff_empty <= '1';
                elsif (sig_inCnt = sig_outCnt + 1 AND OUTREQ = '1' AND RDATA_VALID = '0') then
                    sig_rdbuff_empty <= '1';
                elsif (sig_inCnt = sig_outCnt + 256) then
                    sig_rdbuff_full <= '1';
                elsif (sig_inCnt = sig_outCnt + 255 AND OUTREQ = '0' AND RDATA_VALID = '1') then
                    sig_rdbuff_full <= '1';
                else  
                    sig_rdbuff_empty <= '0';
                    sig_rdbuff_full <= '0';
                end if;
            end if;
        end if;   
    end process;

end Behavioral;
