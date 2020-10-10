library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;

entity modcore_v1_0_S00_AXI is
	generic (
		-- Users to add parameters here
--        U_DEF_SRC_LEN : integer :=256;
		-- User parameters ends
		-- Do not modify the parameters beyond this line

		-- Width of S_AXI data bus
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		-- Width of S_AXI address bus
		C_S_AXI_ADDR_WIDTH	: integer	:= 7 -- 32 register address
	);
	port (
		-- Users to add ports here
		g_reset: out std_logic;
		dma_start: out std_logic;
		intr_done: out std_logic;
		reg_in_addr: in std_logic_vector(4 downto 0); -- 32 regs
		reg_in_addr_valid: in std_logic;
        reg_in_data: in std_logic_vector(31 downto 0);
        reg_in_data_valid: in std_logic;
        reg_out_addr: in std_logic_vector(4 downto 0);
        reg_out_addr_valid: in std_logic;     
        reg_out_data: out std_logic_vector(31 downto 0);
        reg_out_data_valid: out std_logic;
   		-- User ports ends
		-- Do not modify the ports beyond this line

		-- Global Clock Signal
		S_AXI_ACLK	: in std_logic;
		-- Global Reset Signal. This Signal is Active LOW
		S_AXI_ARESETN	: in std_logic;
		-- Write address (issued by master, acceped by Slave)
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		-- Write channel Protection type. This signal indicates the
    		-- privilege and security level of the transaction, and whether
    		-- the transaction is a data access or an instruction access.
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		-- Write address valid. This signal indicates that the master signaling
    		-- valid write address and control information.
		S_AXI_AWVALID	: in std_logic;
		-- Write address ready. This signal indicates that the slave is ready
    		-- to accept an address and associated control signals.
		S_AXI_AWREADY	: out std_logic;
		-- Write data (issued by master, acceped by Slave) 
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		-- Write strobes. This signal indicates which byte lanes hold
    		-- valid data. There is one write strobe bit for each eight
    		-- bits of the write data bus.    
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		-- Write valid. This signal indicates that valid write
    		-- data and strobes are available.
		S_AXI_WVALID	: in std_logic;
		-- Write ready. This signal indicates that the slave
    		-- can accept the write data.
		S_AXI_WREADY	: out std_logic;
		-- Write response. This signal indicates the status
    		-- of the write transaction.
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		-- Write response valid. This signal indicates that the channel
    		-- is signaling a valid write response.
		S_AXI_BVALID	: out std_logic;
		-- Response ready. This signal indicates that the master
    		-- can accept a write response.
		S_AXI_BREADY	: in std_logic;
		-- Read address (issued by master, acceped by Slave)
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		-- Protection type. This signal indicates the privilege
    		-- and security level of the transaction, and whether the
    		-- transaction is a data access or an instruction access.
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		-- Read address valid. This signal indicates that the channel
    		-- is signaling valid read address and control information.
		S_AXI_ARVALID	: in std_logic;
		-- Read address ready. This signal indicates that the slave is
    		-- ready to accept an address and associated control signals.
		S_AXI_ARREADY	: out std_logic;
		-- Read data (issued by slave)
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		-- Read response. This signal indicates the status of the
    		-- read transfer.
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		-- Read valid. This signal indicates that the channel is
    		-- signaling the required read data.
		S_AXI_RVALID	: out std_logic;
		-- Read ready. This signal indicates that the master can
    		-- accept the read data and response information.
		S_AXI_RREADY	: in std_logic
	);
end modcore_v1_0_S00_AXI;

architecture arch_imp of modcore_v1_0_S00_AXI is
    type t_regfile is array (31 downto 0) of std_logic_vector(31 downto 0);
    signal regfile: t_regfile;
    type state_type is (idle, axi_write1, axi_write2, axi_write3, axi_read1, reg_in1, reg_out1);
    signal cur_state: state_type; 
    signal sig_axi_wready: std_logic;
    signal sig_axi_awready: std_logic;
    signal write_strobe_valid: std_logic;
    signal sig_axi_bvalid: std_logic;
    signal sig_axi_bresp: std_logic_vector(1 downto 0);
    signal sig_axi_rdata: std_logic_vector(31 downto 0);
    signal sig_axi_rvalid: std_logic;
    signal sig_axi_arready: std_logic;
    signal sig_axi_rresp: std_logic_vector(1 downto 0);
    signal sig_reg_out_data: std_logic_vector(31 downto 0);
    signal sig_reg_out_data_valid: std_logic;
    signal sig_control_reg: std_logic_vector(31 downto 0);
    signal sig_status_reg: std_logic_vector(31 downto 0);
    signal sig_dma_start: std_logic;
    signal sig_g_reset: std_logic := '0';
    signal sig_g_reset2: std_logic := '0';
    signal sig_g_reset3: std_logic := '0';
    signal sig_intr_done: std_logic := '0';
    signal sig_reg_cap: std_logic_vector(31 downto 0);
    signal sig_reg_src: std_logic_vector(31 downto 0);
    signal sig_reg_len: std_logic_vector(31 downto 0);
    signal sig_reg_tgt: std_logic_vector(31 downto 0);
    attribute MARK_DEBUG : string;
    attribute MARK_DEBUG of sig_control_reg: signal is "TRUE";
--    attribute MARK_DEBUG of sig_axi_wready: signal is "TRUE";
--    attribute MARK_DEBUG of write_strobe_valid: signal is "TRUE";
--    attribute MARK_DEBUG of sig_axi_bvalid: signal is "TRUE";
    attribute MARK_DEBUG of cur_state:signal is "TRUE";
    attribute MARK_DEBUG of S_AXI_WSTRB: signal is "TRUE";
    attribute MARK_DEBUG of S_AXI_AWADDR: signal is "TRUE";
    attribute MARK_DEBUG of S_AXI_WDATA: signal is "TRUE";
    attribute MARK_DEBUG of S_AXI_RREADY: signal is "TRUE";
    attribute MARK_DEBUG of S_AXI_ARVALID: signal is "TRUE";
    attribute MARK_DEBUG of S_AXI_ARADDR: signal is "TRUE"; 
    attribute MARK_DEBUG of S_AXI_AWVALID: signal is "TRUE";
    attribute MARK_DEBUG of S_AXI_WVALID: signal is "TRUE";
    attribute MARK_DEBUG of sig_intr_done: signal is "TRUE";
--    attribute MARK_DEBUG of sig_reg_len: signal is "TRUE";
--    attribute MARK_DEBUG of sig_reg_tgt: signal is "TRUE";
--    attribute MARK_DEBUG of sig_g_reset: signal is "TRUE";
--    attribute MARK_DEBUG of sig_g_reset2: signal is "TRUE";
--    attribute MARK_DEBUG of sig_g_reset3: signal is "TRUE";
begin

    S_AXI_AWREADY <= sig_axi_awready;
    S_AXI_WREADY <= sig_axi_wready;
    S_AXI_BVALID <= sig_axi_bvalid;
    S_AXI_BRESP <= sig_axi_bresp;
    S_AXI_ARREADY <= sig_axi_arready;
    S_AXI_RDATA <= sig_axi_rdata;
    S_AXI_RVALID <= sig_axi_rvalid;
    S_AXI_RRESP <= sig_axi_rresp;
    reg_out_data <= sig_reg_out_data;
    reg_out_data_valid <= sig_reg_out_data_valid;
    dma_start <= sig_dma_start;
    sig_g_reset <= (sig_g_reset2 XOR sig_g_reset3) when sig_g_reset2 = '1' else '0';
    g_reset <= sig_g_reset;
    intr_done <= sig_intr_done;
    --debug
    sig_reg_src <= regfile(2);
    sig_reg_len <= regfile(3);
    sig_reg_tgt <= regfile(4);
    
	process (S_AXI_ACLK) is
	   variable raddr: integer;
	   variable waddr: integer;
	begin
	   if rising_edge(S_AXI_ACLK) then
	       if (sig_g_reset = '1' OR S_AXI_ARESETN = '0') then
                cur_state <= idle;
                sig_axi_bvalid <= '0';
                sig_axi_bresp <= b"00";
                sig_axi_rdata <= (others => '0');
                sig_axi_rvalid <= '0';
                sig_axi_arready <= '0';
                sig_axi_rresp <= "00";
                sig_axi_awready <= '0';
                sig_axi_wready <= '0';
                sig_reg_out_data <= (others => '0');
	            sig_reg_out_data_valid <= '0';
	            write_strobe_valid <= '0';
                waddr := 0;
                raddr := 0;
	       else
            case cur_state is
                when idle =>
                if (S_AXI_AWVALID = '1' AND S_AXI_WVALID = '1') then
                    waddr := to_integer(unsigned(S_AXI_AWADDR(6 downto 2))); -- 32 registers
                    cur_state <= axi_write1;                    
                elsif (S_AXI_ARVALID = '1') then
                    sig_axi_arready <= '1';
                    raddr := to_integer(unsigned(S_AXI_ARADDR(6 downto 2))); -- 32 registers;
                    sig_axi_rvalid <= '0';
                    cur_state <= axi_read1;
                elsif (reg_in_addr_valid = '1') then
                    waddr := to_integer(unsigned(reg_in_addr));
                    sig_reg_cap <= reg_in_data;
                    cur_state <= reg_in1;
                elsif (reg_out_addr_valid = '1') then
                    raddr := to_integer(unsigned(reg_out_addr));
                    cur_state <= reg_out1;
                else
                    cur_state <= idle;
                    sig_axi_bvalid <= '0';
                    sig_axi_bresp <= b"00";
                    sig_axi_rdata <= (others => '0');
                    sig_axi_rvalid <= '0';
                    sig_axi_arready <= '0';
                    sig_axi_rresp <= "00";
                    sig_axi_awready <= '0';
                    sig_axi_wready <= '0';
                    sig_reg_out_data <= (others => '0');
	                sig_reg_out_data_valid <= '0';
	                write_strobe_valid <= '0';
                    waddr := 0;
                    raddr := 0;
					if (regfile(0)(1) = '1') then
						regfile(0)(1) <= '0';
					end if;	
					
					if (regfile(0)(0) = '1') then
					   regfile(0)(0) <= '0';
					end if;				   
                end if;
                
               when axi_write1 =>
                write_strobe_valid <= S_AXI_WSTRB(0) AND S_AXI_WSTRB(1) AND S_AXI_WSTRB(2) AND S_AXI_WSTRB(3);
                sig_axi_awready <= '1';
                sig_axi_wready <= '1';
                cur_state <= axi_write2;
            
               when axi_write2 =>
                if (write_strobe_valid = '1') then
                    regfile(waddr) <= S_AXI_WDATA;
                    sig_axi_bvalid <= '1';
                    sig_axi_bresp <= b"00";
                    cur_state <= axi_write3;
                else
                    sig_axi_bvalid <= '0';
                    sig_axi_bresp <= b"00"; 
                    cur_state <= axi_write1; -- go back until write strobe all set
                end if;
                
               when axi_write3 =>
                if (S_AXI_BREADY = '1') then
                    sig_axi_bvalid <= '0';
                    cur_state <= idle;
                else
                    cur_state <= cur_state;
                end if;
                
               when axi_read1 =>
                sig_axi_rdata <= regfile(raddr);
                sig_axi_rvalid <= '1';
                sig_axi_rresp <= "00";
                cur_state <= idle;
                   
               when reg_in1 =>
                if (reg_in_data_valid = '1') then
                    regfile(waddr) <= sig_reg_cap; 
                    cur_state <= idle;
                else 
                    cur_state <= reg_in1;
                end if;
                
               when reg_out1 =>
                sig_reg_out_data_valid <= '1';
                sig_reg_out_data <= regfile(raddr);
                cur_state <= idle;                           
            end case;	               
	       end if;	   
	   end if;
	end process;
	
	-- handle control register
	process (S_AXI_ACLK) is
	begin
	   if rising_edge(S_AXI_ACLK) then
	       if sig_g_reset = '1' OR S_AXI_ARESETN = '0' then
	           sig_dma_start <= '0';
	           sig_intr_done <= '0';
	       end if;
	       
--	       sig_control_reg <= regfile(0);
	       
           if regfile(0)(0) = '1' then
               sig_dma_start <= '1';
           else 
               sig_dma_start <= '0';
           end if;
           
           if regfile(0)(1) = '1' then
               sig_intr_done <= '1';
              -- regfile(0)(1) <= '0'; -- clear intr_done bit
           else
               sig_intr_done <= '0';
           end if;
           
           if regfile(0)(2) = '1' then
               sig_g_reset2 <= '1';
               sig_g_reset3 <= sig_g_reset2;
           else
               sig_g_reset3 <= '0';
               sig_g_reset2 <= '0';
           end if;	           
	   end if;
	end process;

	-- handle status register
	sig_status_reg <= regfile(1);
--	process (S_AXI_ACLK) is
--	begin
--	   if rising_edge(S_AXI_ACLK) then
--	       if or_reduce(sig_status_reg) /= '0' then

--	       end if;
--	   end if;
--	end process;

end arch_imp;
