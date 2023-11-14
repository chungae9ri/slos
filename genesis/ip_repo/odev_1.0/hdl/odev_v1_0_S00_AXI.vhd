library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity odev_v1_0_S00_AXI is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line

		-- Width of S_AXI data bus
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		-- Width of S_AXI address bus
		C_S_AXI_ADDR_WIDTH	: integer	:= 5
	);
	port (
		-- Users to add ports here
		TRIG_G_START : out std_logic;
		S_G_PULSE : out std_logic;
		S_STREAM_START: out std_logic;
		S_SRC_ADDR : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_SRC_LEN : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
        S_ITAB_FULL : in std_logic;
        S_IN_TRANS_VALID: out std_logic;
        S_IN_TRANS_DONE: in std_logic;
        S_CONSUME_LATENCY: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
        S_INTR_DONE: out std_logic;
        S_ITAB_EMPTY: in std_logic;
        S_RDBUFF_FULL: in std_logic;
        S_RDBUFF_EMPTY: in std_logic;
        S_CONSUMER_START: out std_logic;
        S_SEQ_ERR: in std_logic;
        S_SEQ_ERR_CHK_OUT: out std_logic;
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
end odev_v1_0_S00_AXI;

architecture arch_imp of odev_v1_0_S00_AXI is

	-- AXI4LITE signals
	signal axi_awaddr	: std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_awready	: std_logic;
	signal axi_wready	: std_logic;
	signal axi_bresp	: std_logic_vector(1 downto 0);
	signal axi_bvalid	: std_logic;
	signal axi_araddr	: std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_arready	: std_logic;
	signal axi_rdata	: std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal axi_rresp	: std_logic_vector(1 downto 0);
	signal axi_rvalid	: std_logic;

	-- Example-specific design signals
	-- local parameter for addressing 32 bit / 64 bit C_S_AXI_DATA_WIDTH
	-- ADDR_LSB is used for addressing 32/64 bit registers/memories
	-- ADDR_LSB = 2 for 32 bits (n downto 2)
	-- ADDR_LSB = 3 for 64 bits (n downto 3)
	constant ADDR_LSB  : integer := (C_S_AXI_DATA_WIDTH/32)+ 1;
	constant OPT_MEM_ADDR_BITS : integer := 2;
	------------------------------------------------
	---- Signals for user logic register space example
	--------------------------------------------------
	---- Number of Slave Registers 8
	signal reg_ctrl	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal reg_status	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal reg_addr	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal reg_len	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal reg_latency	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal reg_itab_full_cnt	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal reg_rdbuff_full_cnt	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal reg_itab_empty_cnt	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg_rden	: std_logic;
	signal slv_reg_wren	: std_logic;
	signal reg_data_out	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal byte_index	: integer;
	signal aw_en	: std_logic;
--	
	signal sig_intr_done : std_logic;
	signal sig_trig_g_start: std_logic;
	signal sig_src_addr: std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal sig_src_len: std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
    signal sig_itab_full: std_logic;
    signal sig_in_trans_valid: std_logic;
    signal sig_before: std_logic;
    signal sig_after: std_logic;
    signal sig_consumer_start: std_logic;
	type AXIS_STATE is (IDLE, 
	                   WARMINGUP, 
	                   RECEIVING, 
	                   WRITING, 
	                   DONE_WRITING, 
	                   FULL_CHK_1,
	                   CLOSING, 
	                   FULL, 
	                   EMPTY);
	signal slave_state : AXIS_STATE;
	signal sig_stream_start: std_logic;
	signal sig_seq_err_chk_out: std_logic;
	signal sig_seq_addr: integer;
	-- Control register bit mask  
	constant CTRL_GBL_START_BIT: integer := 0; 
	constant CTRL_INTR_DONE_BIT: integer := 1; 
	constant CTRL_IN_TRANS_BIT: integer := 2; 
	constant CTRL_STREAM_START_BIT: integer := 3;
	constant CTRL_CONSUMER_START_BIT: integer := 4;
	-- Status register bit mask
	constant STAT_ITAB_EMPTY_BIT: integer := 0; 
	constant STAT_RDBUF_EMPTY_BIT: integer := 1; 
	constant STAT_ITAB_FULL_BIT: integer := 2; 
	constant STAT_TRANS_DONE_BIT: integer := 3; 
	constant STAT_RDBUFF_FULL_BIT: integer := 4;
	constant STAT_SEQ_ERR_BIT: integer := 5;
	
--	attribute MARK_DEBUG : string;
--	attribute MARK_DEBUG of reg_ctrl : signal is "TRUE";
--	attribute MARK_DEBUG of reg_status : signal is "TRUE";
--    attribute MARK_DEBUG of slave_state : signal is "TRUE";
--    attribute MARK_DEBUG of reg_addr : signal is "TRUE";
--    attribute MARK_DEBUG of sig_seq_err_chk_out : signal is "TRUE";
--    attribute MARK_DEBUG of sig_seq_addr: signal is "TRUE";
begin
	-- I/O Connections assignments

	S_AXI_AWREADY	<= axi_awready;
	S_AXI_WREADY	<= axi_wready;
	S_AXI_BRESP	<= axi_bresp;
	S_AXI_BVALID	<= axi_bvalid;
	S_AXI_ARREADY	<= axi_arready;
	S_AXI_RDATA	<= axi_rdata;
	S_AXI_RRESP	<= axi_rresp;
	S_AXI_RVALID	<= axi_rvalid;
	S_CONSUME_LATENCY <= reg_latency;
	S_INTR_DONE <= reg_ctrl(CTRL_INTR_DONE_BIT); --sig_intr_done;
	S_CONSUMER_START <= sig_consumer_start;
	S_SEQ_ERR_CHK_OUT <= sig_seq_err_chk_out;
	-- Implement axi_awready generation
	-- axi_awready is asserted for one S_AXI_ACLK clock cycle when both
	-- S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
	-- de-asserted when reset is low.

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_awready <= '0';
	      aw_en <= '1';
	    else
	      if (axi_awready = '0' and S_AXI_AWVALID = '1' and S_AXI_WVALID = '1' and aw_en = '1') then
	        -- slave is ready to accept write address when
	        -- there is a valid write address and write data
	        -- on the write address and data bus. This design 
	        -- expects no outstanding transactions. 
	           axi_awready <= '1';
	           aw_en <= '0';
	        elsif (S_AXI_BREADY = '1' and axi_bvalid = '1') then
	           aw_en <= '1';
	           axi_awready <= '0';
	      else
	        axi_awready <= '0';
	      end if;
	    end if;
	  end if;
	end process;

	-- Implement axi_awaddr latching
	-- This process is used to latch the address when both 
	-- S_AXI_AWVALID and S_AXI_WVALID are valid. 

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_awaddr <= (others => '0');
	    else
	      if (axi_awready = '0' and S_AXI_AWVALID = '1' and S_AXI_WVALID = '1' and aw_en = '1') then
	        -- Write Address latching
	        axi_awaddr <= S_AXI_AWADDR;
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_wready generation
	-- axi_wready is asserted for one S_AXI_ACLK clock cycle when both
	-- S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is 
	-- de-asserted when reset is low. 

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_wready <= '0';
	    else
	      if (axi_wready = '0' and S_AXI_WVALID = '1' and S_AXI_AWVALID = '1' and aw_en = '1') then
	          -- slave is ready to accept write data when 
	          -- there is a valid write address and write data
	          -- on the write address and data bus. This design 
	          -- expects no outstanding transactions.           
	          axi_wready <= '1';
	      else
	        axi_wready <= '0';
	      end if;
	    end if;
	  end if;
	end process; 

	-- Implement memory mapped register select and write logic generation
	-- The write data is accepted and written to memory mapped registers when
	-- axi_awready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted. Write strobes are used to
	-- select byte enables of slave registers while writing.
	-- These registers are cleared when reset (active low) is applied.
	-- Slave register write enable is asserted when valid address and data are available
	-- and the slave is ready to accept the write address and write data.
	slv_reg_wren <= axi_wready and S_AXI_WVALID and axi_awready and S_AXI_AWVALID ;

	process (S_AXI_ACLK)
	variable loc_addr :std_logic_vector(OPT_MEM_ADDR_BITS downto 0); 
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      reg_ctrl <= (others => '0');
--	      reg_status <= (others => '0');
	      reg_addr <= (others => '0');
	      reg_len <= (others => '0');
	      reg_latency <= (others => '0');
--	      reg_itab_full_cnt <= (others => '0');
--	      reg_rdbuff_full_cnt <= (others => '0');
--	      reg_itab_empty_cnt <= (others => '0');
	    else
	      loc_addr := axi_awaddr(ADDR_LSB + OPT_MEM_ADDR_BITS downto ADDR_LSB);
	      if (slv_reg_wren = '1') then
	        case loc_addr is
	        -- control register
	          when b"000" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 0
	                reg_ctrl(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	            -- status register : read-only
	          when b"001" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 1
	                -- status register is read-only
	                --reg_status(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	            -- source address
	          when b"010" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 2
	                reg_addr(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	            -- source len
	          when b"011" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 3
	                reg_len(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	            -- total transfer count
	           when b"100" =>
                  for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
                    if ( S_AXI_WSTRB(byte_index) = '1' ) then
                      -- Respective byte enables are asserted as per write strobes                   
                      -- slave registor 4
                      reg_latency(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
                    end if;
                  end loop;
                  --  not necessary
               when b"101" =>
                    for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
                      if ( S_AXI_WSTRB(byte_index) = '1' ) then
                        -- Respective byte enables are asserted as per write strobes                   
                        -- slave registor 5
--                        reg_itab_full_cnt(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
                      end if;
                    end loop;
                    
              when b"110" =>
                      for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
                        if ( S_AXI_WSTRB(byte_index) = '1' ) then
                          -- Respective byte enables are asserted as per write strobes                   
                          -- slave registor 6
--                          reg_rdbuff_full_cnt(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
                        end if;
                      end loop;
                      
              when b"111" =>
                        for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
                          if ( S_AXI_WSTRB(byte_index) = '1' ) then
                            -- Respective byte enables are asserted as per write strobes                   
                            -- slave registor 7
--                            reg_itab_empty_cnt(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
                          end if;
                        end loop;
	          when others =>
	            reg_ctrl <= reg_ctrl;
	            --reg_status <= reg_status;
	            reg_addr <= reg_addr;
	            reg_len <= reg_len;
	            reg_latency <= reg_latency;
--	            reg_itab_full_cnt <= reg_itab_full_cnt;
--	            reg_rdbuff_full_cnt <= reg_rdbuff_full_cnt;
--	            reg_itab_empty_cnt <= reg_itab_empty_cnt;
	        end case;
	      end if;       
          -- clear intr done bit.
          if (reg_ctrl(CTRL_INTR_DONE_BIT) = '1') then
               reg_ctrl(CTRL_INTR_DONE_BIT) <= '0';
          end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement write response logic generation
	-- The write response and response valid signals are asserted by the slave 
	-- when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.  
	-- This marks the acceptance of address and indicates the status of 
	-- write transaction.

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_bvalid  <= '0';
	      axi_bresp   <= "00"; --need to work more on the responses
	    else
	      if (axi_awready = '1' and S_AXI_AWVALID = '1' and axi_wready = '1' and S_AXI_WVALID = '1' and axi_bvalid = '0'  ) then
	        axi_bvalid <= '1';
	        axi_bresp  <= "00"; 
	      elsif (S_AXI_BREADY = '1' and axi_bvalid = '1') then   --check if bready is asserted while bvalid is high)
	        axi_bvalid <= '0';                                 -- (there is a possibility that bready is always asserted high)
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_arready generation
	-- axi_arready is asserted for one S_AXI_ACLK clock cycle when
	-- S_AXI_ARVALID is asserted. axi_awready is 
	-- de-asserted when reset (active low) is asserted. 
	-- The read address is also latched when S_AXI_ARVALID is 
	-- asserted. axi_araddr is reset to zero on reset assertion.

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_arready <= '0';
	      axi_araddr  <= (others => '1');
	    else
	      if (axi_arready = '0' and S_AXI_ARVALID = '1') then
	        -- indicates that the slave has acceped the valid read address
	        axi_arready <= '1';
	        -- Read Address latching 
	        axi_araddr  <= S_AXI_ARADDR;           
	      else
	        axi_arready <= '0';
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_arvalid generation
	-- axi_rvalid is asserted for one S_AXI_ACLK clock cycle when both 
	-- S_AXI_ARVALID and axi_arready are asserted. The slave registers 
	-- data are available on the axi_rdata bus at this instance. The 
	-- assertion of axi_rvalid marks the validity of read data on the 
	-- bus and axi_rresp indicates the status of read transaction.axi_rvalid 
	-- is deasserted on reset (active low). axi_rresp and axi_rdata are 
	-- cleared to zero on reset (active low).  
	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then
	    if S_AXI_ARESETN = '0' then
	      axi_rvalid <= '0';
	      axi_rresp  <= "00";
	    else
	      if (axi_arready = '1' and S_AXI_ARVALID = '1' and axi_rvalid = '0') then
	        -- Valid read data is available at the read data bus
	        axi_rvalid <= '1';
	        axi_rresp  <= "00"; -- 'OKAY' response
	      elsif (axi_rvalid = '1' and S_AXI_RREADY = '1') then
	        -- Read data is accepted by the master
	        axi_rvalid <= '0';
	      end if;            
	    end if;
	  end if;
	end process;

	-- Implement memory mapped register select and read logic generation
	-- Slave register read enable is asserted when valid address is available
	-- and the slave is ready to accept the read address.
	slv_reg_rden <= axi_arready and S_AXI_ARVALID and (not axi_rvalid) ;

	process (reg_ctrl, reg_status, reg_addr, reg_len, reg_latency, reg_itab_full_cnt, reg_rdbuff_full_cnt, reg_itab_empty_cnt, axi_araddr, S_AXI_ARESETN, slv_reg_rden)
	variable loc_addr :std_logic_vector(OPT_MEM_ADDR_BITS downto 0);
	begin
	    -- Address decoding for reading registers
	    loc_addr := axi_araddr(ADDR_LSB + OPT_MEM_ADDR_BITS downto ADDR_LSB);
	    case loc_addr is
	      when b"000" =>
	        reg_data_out <= reg_ctrl;
	      when b"001" =>
	        reg_data_out <= reg_status;
	      when b"010" =>
	        reg_data_out <= reg_addr;
	      when b"011" =>
	        reg_data_out <= reg_len;
	      when b"100" =>
	        reg_data_out <= reg_latency;
	      when b"101" =>
	        reg_data_out <= reg_itab_full_cnt;
	      when b"110" =>
	        reg_data_out <= reg_rdbuff_full_cnt;
	      when b"111" =>
	        reg_data_out <= reg_itab_empty_cnt;
	      when others =>
	        reg_data_out  <= (others => '0');
	    end case;
	end process; 

	-- Output register or memory read data
	process( S_AXI_ACLK ) is
	begin
	  if (rising_edge (S_AXI_ACLK)) then
	    if ( S_AXI_ARESETN = '0' ) then
	      axi_rdata  <= (others => '0');
	    else
	      if (slv_reg_rden = '1') then
	        -- When there is a valid read address (S_AXI_ARVALID) with 
	        -- acceptance of read address by the slave (axi_arready), 
	        -- output the read dada 
	        -- Read address mux
	          axi_rdata <= reg_data_out;     -- register read data
	      end if;   
	    end if;
	  end if;
	end process;


	-- Add user logic here
	TRIG_G_START <= sig_trig_g_start;
	S_SRC_ADDR <= sig_src_addr;
	S_SRC_LEN <= sig_src_len;
	S_IN_TRANS_VALID <= sig_in_trans_valid;
	-- generate pulse for global start and stop
	S_G_PULSE <= sig_before AND (NOT sig_after) when sig_before = '1' else
	             (NOT sig_before) AND sig_after;
	             
	S_STREAM_START <= sig_stream_start;
	
	-- slave state machine S
	process(S_AXI_ACLK) is
	begin
		if (rising_edge(S_AXI_ACLK)) then
			if ( S_AXI_ARESETN = '0' ) then
				sig_trig_g_start <= '0';
				sig_src_addr <= (others => '0');
				sig_src_len <= (others => '0');
				sig_in_trans_valid <= '0';
				slave_state <= IDLE;
				sig_before <= '0';
				sig_after <= '0';
			else
			    -- itab empty status update
			    if (S_ITAB_EMPTY = '1') then
			        reg_status(STAT_ITAB_EMPTY_BIT) <= '1';
			    else
			        reg_status(STAT_ITAB_EMPTY_BIT) <= '0';
			    end if;
			    -- RDBUFF empty/full status update
			    if (S_RDBUFF_FULL = '1') then
			        reg_status(STAT_RDBUFF_FULL_BIT) <= '1';
			    elsif (S_RDBUFF_EMPTY = '1') then
			        reg_status(STAT_RDBUF_EMPTY_BIT) <= '1';
			    else 
			        reg_status(STAT_RDBUFF_FULL_BIT) <= '0';
			        reg_status(STAT_RDBUF_EMPTY_BIT) <= '0';
			    end if;
			    -- SEQ error check
			    if (S_SEQ_ERR = '1') then
			        reg_status(STAT_SEQ_ERR_BIT) <= '1';
			    else
			        reg_status(STAT_SEQ_ERR_BIT) <= '0';
			    end if;
			    -- slave state machine
				case (slave_state) is
					when IDLE =>
						if (reg_ctrl(CTRL_GBL_START_BIT) = '1') then
							sig_trig_g_start <= '1';
							sig_before <= '1';
							sig_after <= sig_before;
							-- S_G_PULSE generation
							slave_state <= WARMINGUP;
							-- odev start address is sequence check
							-- start address. Currently it is hardcoded and
							-- should be the same as the value in the PS kernel
							-- odev task
							sig_seq_addr <= 16#18000000#;
							sig_seq_err_chk_out <= '0';
						else
							sig_trig_g_start <= '0';
							sig_before <= '0';
							sig_after <= sig_before;
							sig_src_addr <= (others => '0');
							sig_src_len <= (others => '0');
							sig_in_trans_valid <= '0';
							slave_state <= IDLE;
							reg_status <= (others => '0');
						end if;

					when WARMINGUP =>
						sig_before <= '1';
						sig_after <= sig_before;
						reg_status(STAT_TRANS_DONE_BIT) <= '0';
						slave_state <= RECEIVING;
                        
					when RECEIVING =>
						if (reg_ctrl(CTRL_GBL_START_BIT) = '1') then
							if (reg_ctrl(CTRL_IN_TRANS_BIT) = '1') then
								sig_src_addr <= reg_addr;
								sig_src_len <= reg_len;
								sig_in_trans_valid <= '1';
								reg_status(STAT_TRANS_DONE_BIT) <= '0';
								slave_state <= WRITING;
								if (sig_seq_addr /= to_integer(unsigned(reg_addr))) then
								    if (reg_addr /= x"1800_0000") then
								        sig_seq_err_chk_out <= '1';
							        else
								        sig_seq_addr <= 16#18000100#;
								    end if;
							    else
							        sig_seq_addr <= sig_seq_addr + 256;
							    end if;
							else    
								slave_state <= RECEIVING; 
							end if;
						else 
							sig_trig_g_start <= '0';
							sig_in_trans_valid <= '0';
							sig_before <= '0';
							sig_after <= sig_before;
							-- generate closing pulse
							slave_state <= CLOSING;
						end if;
					-- generate a S_G_PULSE     
					when WRITING =>
						if (reg_ctrl(CTRL_GBL_START_BIT) = '1') then
                            if (S_IN_TRANS_DONE = '0') then
								sig_in_trans_valid <= '0';
								reg_status(STAT_TRANS_DONE_BIT) <= '0';
								slave_state <= WRITING;
							else
							    reg_status(STAT_TRANS_DONE_BIT) <= '1';
								slave_state <= DONE_WRITING;
							end if;
						else 
							sig_trig_g_start <= '0';
							sig_in_trans_valid <= '0';
							sig_before <= '0';
							sig_after <= sig_before;
							-- generate closing pulse
							slave_state <= CLOSING;						
						end if;
					
					when DONE_WRITING =>
                        if (reg_ctrl(CTRL_GBL_START_BIT) = '1') then
                            if (reg_ctrl(CTRL_IN_TRANS_BIT) = '1') then 
                                sig_in_trans_valid <= '0';
                                reg_status(STAT_TRANS_DONE_BIT) <= '1';
                                slave_state <= DONE_WRITING;
                            else
                                -- temporary block future put_to_itab() call
                                reg_status(STAT_ITAB_FULL_BIT) <= '1';
                                slave_state <= FULL_CHK_1;
                            end if;
                        else 
                            sig_trig_g_start <= '0';
                            sig_in_trans_valid <= '0';
                            sig_before <= '0';
                            sig_after <= sig_before;
                            slave_state <= CLOSING;	
                        end if;
                        
                    when FULL_CHK_1 =>
                        -- one more clock delay after S_ITAB_FULL
                        if (S_ITAB_FULL = '1') then
                            sig_in_trans_valid <= '0';
                            reg_status(STAT_TRANS_DONE_BIT) <= '0'; 
                            reg_status(STAT_ITAB_FULL_BIT) <= '1';							   
                            slave_state <= FULL;                                 
                        else
                            sig_in_trans_valid <= '0';
                            -- must release blocking put_to_itab() call
                            reg_status(STAT_ITAB_FULL_BIT) <= '0';
                            reg_status(STAT_TRANS_DONE_BIT) <= '0';
                            slave_state <= RECEIVING;
                        end if;
                        
					when CLOSING =>
						sig_before <= '0';
						sig_after <= sig_before;
						slave_state <= IDLE;
                       
					when FULL =>
						if (reg_ctrl(CTRL_GBL_START_BIT) = '1')  then
							if (S_ITAB_FULL = '1') then
								reg_status(STAT_ITAB_FULL_BIT) <= '1';
								slave_state <= FULL;
							else
							    sig_in_trans_valid <= '0';
								reg_status(STAT_ITAB_FULL_BIT) <= '0';
								reg_status(STAT_TRANS_DONE_BIT) <= '0'; 
								slave_state <= RECEIVING;
							end if;
						else 
							sig_trig_g_start <= '0';
							sig_in_trans_valid <= '0';
							sig_before <= '0';
							sig_after <= sig_before;
							-- generate closing pulse
							slave_state <= CLOSING;	
						end if;
					-- Not necessary
					-- when EMPTY =>
					when others =>
						sig_trig_g_start <= '0';
						sig_in_trans_valid <= '0';
						sig_before <= '0';
						sig_after <= '0';
						slave_state <= IDLE;
				end case;
			end if;
		end if;
	end process;
	-- slave state machine E
	
	process (S_AXI_ACLK) is
	begin
	   if (rising_edge(S_AXI_ACLK)) then
           if (reg_ctrl(CTRL_STREAM_START_BIT) = '1') then
               sig_stream_start <= '1';
           else 
               sig_stream_start <= '0';
           end if;
	   end if;
	end process;

	process (S_AXI_ACLK) is
	begin
	   if (rising_edge(S_AXI_ACLK)) then
           if (reg_ctrl(CTRL_CONSUMER_START_BIT) = '1') then
               sig_consumer_start <= '1';
           else 
               sig_consumer_start <= '0';
           end if;
	   end if;
	end process;
	
	process (S_AXI_ACLK)
	begin
	   if (rising_edge(S_AXI_ACLK)) then
            if (reg_ctrl(CTRL_INTR_DONE_BIT) = '1') then
                sig_intr_done <= '1';
            else 
                sig_intr_done <= '0';
            end if;
       end if;	
	end process;
	
	process (S_ITAB_FULL, S_ITAB_EMPTY, S_RDBUFF_FULL)
	begin
	   if (reg_ctrl(CTRL_GBL_START_BIT) = '0') then
	       reg_itab_empty_cnt <= (others => '0');
	       reg_itab_full_cnt <= (others => '0');
	       reg_rdbuff_full_cnt <= (others => '0');
       else
           if (S_ITAB_FULL = '1') then
               reg_itab_full_cnt <= std_logic_vector(to_unsigned(to_integer(unsigned(reg_itab_full_cnt)) + 1, 32));
           end if;
           if (S_ITAB_EMPTY = '1') then
               reg_itab_empty_cnt <= std_logic_vector(to_unsigned(to_integer(unsigned(reg_itab_empty_cnt)) + 1, 32));
           end if;
           if (S_RDBUFF_FULL = '1') then
               reg_rdbuff_full_cnt <= std_logic_vector(to_unsigned(to_integer(unsigned(reg_rdbuff_full_cnt)) + 1, 32));    
           end if;
	   end if;
	end process;
	-- User logic ends

end arch_imp;
