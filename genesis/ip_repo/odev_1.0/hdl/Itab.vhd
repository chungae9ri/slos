----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 08/25/2019 12:09:04 PM
-- Design Name: 
-- Module Name: Itab - Behavioral
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
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity Itab is
generic (
    Itab_entries: integer := 512
);
port (
    clk: in std_logic;
    ITAB_G_START: in std_logic;
    SRC_ADDR_IN: in std_logic_vector (31 downto 0);
    SRC_LEN_IN: in std_logic_vector (15 downto 0);
    ITAB_IN_TRANS_VALID: in std_logic;
    ITAB_IN_TRANS_DONE: out std_logic;
    SRC_ADDR_OUT: out std_logic_vector (31 downto 0);
    SRC_LEN_OUT: out std_logic_vector(15 downto 0);
    ITAB_OUT_TRANS_REQ: in std_logic;
    ITAB_FULL: out std_logic;
    ITAB_EMPTY: out std_logic;
	ITAB_OUT_VALID: out std_logic
);
end Itab;

architecture Behavioral of Itab is
    type t_ItabMem is array (Itab_entries - 1 downto 0) of std_logic_vector (47 downto 0);
    signal ItabMem: t_ItabMem;
    signal in_pos: integer;
    signal out_pos: integer;
    signal in_count: integer;
    signal out_count: integer;
    signal sig_src_addr_out: std_logic_vector (31 downto 0);
    signal sig_src_len_out: std_logic_vector (15 downto 0);
    signal sig_itab_full: std_logic;
    signal sig_itab_empty: std_logic;
	signal sig_itab_out_valid: std_logic;
--	signal sig_mem_i: std_logic_vector (31 downto 0);
--    signal sig_mem_o: std_logic_vector (31 downto 0);
    signal sig_itab_in_trans_done: std_logic;
    signal sig_valid_src_addr: std_logic_vector(31 downto 0);
    
--	attribute MARK_DEBUG: string;
	--attribute MARK_DEBUG of ITAB_G_START : signal is "TRUE";
--	attribute MARK_DEBUG of ITAB_IN_TRANS_VALID : signal is "TRUE";
--	attribute MARK_DEBUG of SRC_ADDR_IN : signal is "TRUE";
--	attribute MARK_DEBUG of in_count : signal is "TRUE";
--	attribute MARK_DEBUG of out_count : signal is "TRUE";
--	attribute MARK_DEBUG of sig_src_addr_out: signal is "TRUE";
--	attribute MARK_DEBUG of sig_itab_out_valid : signal is "TRUE";
--	attribute MARK_DEBUG of ITAB_OUT_TRANS_REQ : signal is "TRUE";
--    attribute MARK_DEBUG of sig_itab_full: signal is "TRUE";
--	attribute MARK_DEBUG of out_pos : signal is "TRUE";
--	attribute MARK_DEBUG of in_pos : signal is "TRUE";
--	attribute MARK_DEBUG of sig_valid_src_addr : signal is "TRUE";
begin
    
    SRC_ADDR_OUT <= sig_src_addr_out;
    SRC_LEN_OUT <= sig_src_len_out;
    ITAB_FULL <= sig_itab_full;
    ITAB_EMPTY <= sig_itab_empty;
	ITAB_OUT_VALID <= sig_itab_out_valid;
	ITAB_IN_TRANS_DONE <= sig_itab_in_trans_done;
    
    process (clk)
    begin
        if (rising_edge(clk)) then
            if (ITAB_G_START = '0') then
                sig_src_addr_out <= x"0000_0000";
                sig_src_len_out <= x"0000";
                out_count <= 0;
                out_pos <= 0;
				sig_itab_out_valid <= '0';
            elsif (ITAB_OUT_TRANS_REQ = '1') then
				if (out_count < in_count) then
					sig_src_addr_out <= ItabMem(out_pos)(47 downto 16);
					sig_src_len_out <= ItabMem(out_pos)(15 downto 0);
					sig_itab_out_valid <= '1';
					out_count <= out_count + 1;
					out_pos <= to_integer(unsigned(std_logic_vector(to_unsigned(out_count + 1, 32)) AND x"0000_01FF"));
--					sig_mem_o <= ItabMem(0)(47 downto 16);
				end if;
            else
                sig_src_addr_out <= (others => '0');
                sig_src_len_out <= (others => '0');
				sig_itab_out_valid <= '0';
            end if;
        end if;
    end process;
    
    process (clk)
    begin
        if (rising_edge(clk)) then
            if (ITAB_G_START = '0') then
                in_count <= 0;
                in_pos <= 0;
                sig_itab_in_trans_done <= '0';
            elsif (ITAB_IN_TRANS_VALID = '1') then
                -- do not overwrite prev
                if (in_count - out_count < Itab_entries) then 
                    ItabMem(in_pos)(47 downto 16) <= SRC_ADDR_IN;
                    ItabMem(in_pos)(15 downto 0) <= SRC_LEN_IN;
                    in_count <= in_count + 1;
                    in_pos <= to_integer(unsigned(std_logic_vector(to_unsigned(in_count + 1,32)) AND x"0000_01FF"));
                    sig_itab_in_trans_done <= '1';
                    sig_valid_src_addr <= SRC_ADDR_IN;
                else 
                    sig_itab_in_trans_done <= '0';
                end if;
            else
                sig_itab_in_trans_done <= '0';
            end if;
        end if;
    end process;

    process (clk)
        variable cur_count: integer;
    begin
        if (rising_edge (clk)) then
            cur_count := in_count - out_count;
            if (ITAB_G_START = '0') then
				sig_itab_full <= '0';
                sig_itab_empty <= '1';
            elsif (cur_count = Itab_entries - 1 AND ITAB_IN_TRANS_VALID = '1' AND ITAB_OUT_TRANS_REQ = '0') then
                sig_itab_full <= '1';
            elsif (cur_count = Itab_entries) then
                sig_itab_full <= '1';
            elsif (cur_count = 1 AND ITAB_OUT_TRANS_REQ = '1' AND ITAB_IN_TRANS_VALID = '0') then
                sig_itab_empty <= '1';
            elsif (cur_count = 0) then
                sig_itab_empty <= '1';
            else
                sig_itab_full <= '0';
                sig_itab_empty <= '0';
            end if;
        end if;
    end process;
end Behavioral;
