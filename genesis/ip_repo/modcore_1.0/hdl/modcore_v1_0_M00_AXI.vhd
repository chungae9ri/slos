library ieee;
--library unisim;
--use unisim.vcomponents.all;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity modcore_v1_0_M00_AXI is
	generic (
		-- Users to add parameters here
        C_M_AXI_BEAT_SIZE : integer	:= 4; -- 4 byte size per beat
		-- User parameters ends
		-- Do not modify the parameters beyond this line
		-- Burst Length. Supports 1, 2, 4, 8, 16, 32, 64, 128, 256 burst lengths
		C_M_AXI_BURST_LEN	: integer	:= 16;
		-- Thread ID Width
		C_M_AXI_ID_WIDTH	: integer	:= 1;
		-- Width of Address Bus
		C_M_AXI_ADDR_WIDTH	: integer	:= 32;
		-- Width of Data Bus
		C_M_AXI_DATA_WIDTH	: integer	:= 32
	);
	port (
		-- Users to add ports here
		g_reset: in std_logic;
		dma_start: in std_logic;
		reg_in_addr: out std_logic_vector(4 downto 0);
		reg_in_addr_valid: out std_logic;
		reg_in_data: out std_logic_vector(31 downto 0);
		reg_in_data_valid: out std_logic;
		reg_out_addr: out std_logic_vector(4 downto 0);
		reg_out_addr_valid: out std_logic;
		reg_out_data: in std_logic_vector(31 downto 0);
		reg_out_data_valid: in std_logic;
        mdproc_data_in: out std_logic_vector(31 downto 0);
        mdproc_data_in_valid: out std_logic;
        mdproc_data_out_req: out std_logic;
        mdproc_data_out: in std_logic_vector(31 downto 0);
        mdproc_data_out_valid: in std_logic;
        dma_intr: out std_logic;
		-- User ports ends
		M_AXI_ACLK	: in std_logic;
		M_AXI_ARESETN	: in std_logic;
		M_AXI_AWID	: out std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		M_AXI_AWADDR	: out std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
		M_AXI_AWLEN	: out std_logic_vector(7 downto 0);
		M_AXI_AWSIZE	: out std_logic_vector(2 downto 0);
		M_AXI_AWBURST	: out std_logic_vector(1 downto 0);
		M_AXI_AWLOCK	: out std_logic;
		M_AXI_AWCACHE	: out std_logic_vector(3 downto 0);
		M_AXI_AWPROT	: out std_logic_vector(2 downto 0);
		M_AXI_AWQOS	: out std_logic_vector(3 downto 0);
		M_AXI_AWVALID	: out std_logic;
		M_AXI_AWREADY	: in std_logic;
		M_AXI_WDATA	: out std_logic_vector(31 downto 0);
		M_AXI_WSTRB	: out std_logic_vector(3 downto 0);
		M_AXI_WLAST	: out std_logic;
		M_AXI_WVALID	: out std_logic;
		M_AXI_WREADY	: in std_logic;
		M_AXI_BID	: in std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		M_AXI_BRESP	: in std_logic_vector(1 downto 0);
		M_AXI_BVALID	: in std_logic;
		M_AXI_BREADY	: out std_logic;
		M_AXI_ARID	: out std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		M_AXI_ARADDR	: out std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
		M_AXI_ARLEN	: out std_logic_vector(7 downto 0);
		M_AXI_ARSIZE	: out std_logic_vector(2 downto 0);
		M_AXI_ARBURST	: out std_logic_vector(1 downto 0);
		M_AXI_ARLOCK	: out std_logic;
		M_AXI_ARCACHE	: out std_logic_vector(3 downto 0);
		M_AXI_ARPROT	: out std_logic_vector(2 downto 0);
		M_AXI_ARQOS	: out std_logic_vector(3 downto 0);
		M_AXI_ARVALID	: out std_logic;
		M_AXI_ARREADY	: in std_logic;
		M_AXI_RID	: in std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		M_AXI_RDATA	: in std_logic_vector(31 downto 0);
		M_AXI_RRESP	: in std_logic_vector(1 downto 0);
		M_AXI_RLAST	: in std_logic;
		M_AXI_RVALID	: in std_logic;
		M_AXI_RREADY	: out std_logic
	);
end modcore_v1_0_M00_AXI;

architecture implementation of modcore_v1_0_M00_AXI is
	type buff_type is array (0 to 15) of std_logic_vector(31 downto 0);
	signal tgtbuff : buff_type := (others => (others =>'0'));
	type state is ( idle, prep1, prep2, prep3, write1, write2, bresp1, read1, read2, read3, mdproc1, mdproc2, intr1);
	signal cur_state  : state ; 
    signal sig_reg_in_addr: std_logic_vector(4 downto 0);
    signal sig_reg_in_addr_valid: std_logic;
    signal sig_reg_in_data: std_logic_vector(31 downto 0);
    signal sig_reg_in_data_valid: std_logic;
    signal sig_reg_out_addr: std_logic_vector(4 downto 0);
    signal sig_reg_out_addr_valid: std_logic;
--	signal sig_src_addr: std_logic_vector(31 downto 0);
--	signal sig_tgt_addr: std_logic_vector(31 downto 0);
	signal sig_src_len: std_logic_vector(31 downto 0);
	signal sig_axi_awaddr: std_logic_vector(31 downto 0);
--	signal sig_axi_awlen: std_logic_vector(7 downto 0);
--	signal sig_axi_awsize: std_logic_vector(2 downto 0);
--	signal sig_axi_awburst: std_logic_vector(1 downto 0);
--	signal sig_axi_awlock: std_logic;
--	signal sig_axi_awcache: std_logic_vector(3 downto 0);
--	signal sig_axi_awprot: std_logic_vector(2 downto 0);
--	signal sig_axi_awqos: std_logic_vector(3 downto 0);
	signal sig_axi_awvalid: std_logic;
	signal sig_axi_wdata: std_logic_vector(31 downto 0);
	signal sig_axi_wstrb: std_logic_vector(3 downto 0);
	signal sig_axi_wlast: std_logic;
	signal sig_axi_wvalid: std_logic;
	signal sig_axi_bready: std_logic;
--	signal sig_axi_arid: std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
	signal sig_axi_araddr: std_logic_vector(31 downto 0);
--	signal sig_axi_arlen: std_logic_vector(7 downto 0);
--	signal sig_axi_arsize: std_logic_vector(2 downto 0);
--	signal sig_axi_arburst: std_logic_vector(1 downto 0);
--	signal sig_axi_arlock: std_logic;
--	signal sig_axi_arcache: std_logic_vector(3 downto 0);
--	signal sig_axi_arprot: std_logic_vector(2 downto 0);
--	signal sig_axi_arqos: std_logic_vector(3 downto 0);
	signal sig_axi_arvalid: std_logic;
	signal sig_axi_rready: std_logic;
	signal sig_dma_start: std_logic;
	signal sig_dma_start2: std_logic;
	signal sig_dma_pulse: std_logic;
	signal sig_mdproc_data_in: std_logic_vector(31 downto 0);
    signal sig_mdproc_data_in_valid: std_logic;
    signal sig_mdproc_data_out_req: std_logic;
    signal sig_dma_intr: std_logic;
	attribute MARK_DEBUG : string;
	attribute MARK_DEBUG of cur_state: signal is "TRUE";
--	attribute MARK_DEBUG of sig_axi_awaddr: signal is "TRUE";
--	attribute MARK_DEBUG of sig_src_len: signal is "TRUE";
--	attribute MARK_DEBUG of sig_axi_araddr: signal is "TRUE";
--	attribute MARK_DEBUG of M_AXI_RDATA: signal is "TRUE";
--	attribute MARK_DEBUG of M_AXI_RVALID: signal is "TRUE";
--	attribute MARK_DEBUG of sig_mdproc_data_out_req: signal is "TRUE";
--	attribute MARK_DEBUG of sig_mdproc_data_in: signal is "TRUE";
	attribute MARK_DEBUG of sig_dma_start: signal is "TRUE";
--	attribute MARK_DEBUG of sig_dma_pulse: signal is "TRUE";
--	attribute MARK_DEBUG of reg_out_data_valid: signal is "TRUE";
--	attribute MARK_DEBUG of reg_out_data: signal is "TRUE";
	attribute MARK_DEBUG of mdproc_data_in_valid: signal is "TRUE";
	attribute MARK_DEBUG of mdproc_data_in: signal is "TRUE";
	attribute MARK_DEBUG of mdproc_data_out: signal is "TRUE";
	attribute MARK_DEBUG of mdproc_data_out_valid: signal is "TRUE";
	attribute MARK_DEBUG of sig_dma_intr: signal is "TRUE";
--	attribute MARK_DEBUG of sig_axi_awvalid: signal is "TRUE";
	attribute MARK_DEBUG of sig_axi_wvalid: signal is "TRUE";
--	attribute MARK_DEBUG of sig_axi_wlast: signal is "TRUE";
	attribute MARK_DEBUG of sig_axi_wdata: signal is "TRUE";
--	attribute MARK_DEBUG of sig_axi_bready: signal is "TRUE";
--	attribute MARK_DEBUG of M_AXI_WREADY: signal is "TRUE";
--	attribute MARK_DEBUG of M_AXI_AWREADY: signal is "TRUE";
--	attribute MARK_DEBUG of M_AXI_BVALID: signal is "TRUE";
	attribute MARK_DEBUG of M_AXI_AWADDR: signal is "TRUE";
begin
    reg_in_addr <= sig_reg_in_addr;
    reg_in_addr_valid <= sig_reg_in_addr_valid;
    reg_in_data <= sig_reg_in_data;
    reg_in_data_valid <= sig_reg_in_data_valid;
    reg_out_addr <= sig_reg_out_addr;
    reg_out_addr_valid <= sig_reg_out_addr_valid;
    mdproc_data_in <= sig_mdproc_data_in;
    mdproc_data_in_valid <= sig_mdproc_data_in_valid;
    mdproc_data_out_req <= sig_mdproc_data_out_req;
	M_AXI_AWID <= (others => '0');
	M_AXI_AWADDR <= sig_axi_awaddr;
	M_AXI_AWLEN	<= x"0f"; --Burst len 16 beats per burst
	M_AXI_AWSIZE <= "010"; -- beat size 4 bytes
	M_AXI_AWBURST <= "01";--INCR burst type is usually used
	M_AXI_AWLOCK <= '0'; 
	M_AXI_AWCACHE <= "0010"; --Update value to 4'b0011 if coherent accesses to be used via the Zynq ACP port.
	M_AXI_AWPROT <= "000";
	M_AXI_AWQOS	<= x"0";
	M_AXI_AWVALID <= sig_axi_awvalid;
	M_AXI_WDATA	<= sig_axi_wdata;
	M_AXI_WSTRB	<= sig_axi_wstrb;	
	M_AXI_WLAST	<= sig_axi_wlast;
	M_AXI_WVALID <= sig_axi_wvalid;
	M_AXI_BREADY <= sig_axi_bready;	--Write Response (B)
	M_AXI_ARID <= (others => '0');
	M_AXI_ARADDR <= sig_axi_araddr;	
	M_AXI_ARLEN	<= x"0f"; --Burst len 16 beats per burst
	M_AXI_ARSIZE <= "010"; -- 4 bytes beat size
	M_AXI_ARBURST <= "01"; --INCR burst type
	M_AXI_ARLOCK <= '0'; 
	M_AXI_ARCACHE <= "0010"; 	--4'b0011, normal memory not cacheable, bufferable
	M_AXI_ARPROT <= "000";
	M_AXI_ARQOS	<= x"0";
	M_AXI_ARVALID <= sig_axi_arvalid;
	M_AXI_RREADY <= sig_axi_rready;
	dma_intr <= sig_dma_intr;
	
	sig_dma_pulse <= (sig_dma_start XOR sig_dma_start2) when sig_dma_start = '1' else '0';
	process(M_AXI_ACLK)                                                          
	begin                                                                             
	  if (rising_edge (M_AXI_ACLK)) then                                                      
	    if (g_reset = '1' ) then                                                
            sig_dma_start <= '0';                                                   
            sig_dma_start2 <= '0';                                                          
	    else                                                                                       
            sig_dma_start <= dma_start; 
            sig_dma_start2 <= sig_dma_start;                                                                     
	    end if;                                                                       
	  end if;                                                                         
	end process; 
	
	process(M_AXI_ACLK) is  
	   variable srcbuffidx: integer;
	   variable tgtbuffidx: integer;
	   variable remainlen: integer;  
	   constant burstsize: integer := C_M_AXI_BURST_LEN * C_M_AXI_BEAT_SIZE;                                                   
	begin
	   if rising_edge(M_AXI_ACLK) then
	       if (g_reset = '1' OR M_AXI_ARESETN = '0') then
	           cur_state <= idle;
	       else
	           case cur_state is
	           -- all outer valid input trigger signal come during idle state
	           when idle =>
	               if (sig_dma_pulse = '1') then
	                   sig_reg_out_addr <= "00010"; -- src_addr
	                   sig_reg_out_addr_valid <= '1';
	                   sig_axi_arvalid <= '0';
	                   sig_axi_rready <= '0';
	                   cur_state <= prep1;
	               else
	                   sig_reg_out_addr_valid <= '0';
	                   sig_reg_in_addr <= (others => '0');
                       sig_reg_in_addr_valid <= '0';
                       sig_reg_in_data <= (others => '0');
                       sig_reg_in_data_valid <= '0';
                       sig_reg_out_addr <= (others => '0');
                       sig_reg_out_addr_valid <= '0';
                       srcbuffidx := 0;
                       tgtbuffidx := 0;
                       remainlen := 0;
                       tgtbuff <= (others => (others => '0'));
                       sig_dma_intr <= '0';
	                   cur_state <= idle;
	               end if;
	               
	           when prep1=>
	               if (reg_out_data_valid = '1') then
	                   sig_axi_araddr <= reg_out_data;
	                   sig_reg_out_addr <= "00011"; -- src_len
	                   sig_reg_out_addr_valid <= '1'; -- read next register
	                   cur_state <= prep2;
	               else
						sig_reg_out_addr_valid <= '0';
	                   cur_state <= prep1;
	               end if;
	               
	           when prep2=>
	               if (reg_out_data_valid = '1') then
	                   sig_src_len <= reg_out_data;
	                   remainlen := to_integer(unsigned(reg_out_data));
	                   sig_reg_out_addr <= "00100"; -- tgt_addr
	                   sig_reg_out_addr_valid <= '1'; -- read next register
	                   cur_state <= prep3;
	               else
						sig_reg_out_addr_valid <= '0';
	                   cur_state <= prep2;
	               end if;
	               
	           when prep3=>
	               if (reg_out_data_valid = '1') then
	                   sig_axi_awaddr <= reg_out_data;
	                   sig_reg_out_addr_valid <= '0';
	                   cur_state <= read1;
	               else
						sig_reg_out_addr_valid <= '0';
	                   cur_state <= prep3;
	               end if;
	               	               
	           when read1=>
	               sig_axi_arvalid <= '1';
				   sig_axi_rready <= '1';
	               srcbuffidx := 0;
	               cur_state <= read2;
	               
	           when read2 =>
	               if (M_AXI_ARREADY = '1' AND sig_axi_arvalid = '1') then
                       sig_axi_arvalid <= '0';
	               end if;
	               if (M_AXI_RVALID = '1') then
                       sig_mdproc_data_in <= M_AXI_RDATA;
                       srcbuffidx := srcbuffidx + 1;
                       --if (srcbuffidx = C_M_AXI_BURST_LEN - 1) then
					   if (M_AXI_RLAST = '1') then
                           sig_axi_rready <= '0';
                           cur_state <= read3;
                       else 
                           sig_mdproc_data_in_valid <= '1';
                           sig_axi_rready <= '1';
                           cur_state <= read2;
                       end if;
                   else 
                       cur_state <= read2;	               
	               end if;
				   
			   when read3 =>
	               tgtbuffidx := 0;
                   sig_mdproc_data_out_req <= '1';
                   sig_mdproc_data_in_valid <= '0';
				   cur_state <= mdproc1;
						   
	           when mdproc1 =>
	               if (mdproc_data_out_valid = '1') then
	               	   sig_axi_wvalid <= '0';
	               	   sig_axi_bready <= '0';
	               	   sig_axi_awvalid <= '0';
	               	   sig_axi_wlast <= '0';
	               	   sig_axi_wstrb <= "0000";
	               	   sig_axi_bready <= '0';
	                   tgtbuff(tgtbuffidx) <= mdproc_data_out;
	                   tgtbuffidx := tgtbuffidx + 1;
	                   cur_state <= mdproc2;
	                else 
	                   cur_state <= mdproc1;
	                end if;

	           when mdproc2 =>
	               if (mdproc_data_out_valid = '1') then
	                   tgtbuff(tgtbuffidx) <= mdproc_data_out;
	                   if (tgtbuffidx = 15) then
	                       sig_mdproc_data_out_req <= '0';
	                       tgtbuffidx := 0;
					   else
						   tgtbuffidx := tgtbuffidx + 1;
	                   end if;
					   cur_state <= mdproc2; 
				   else
				       tgtbuffidx := 0;
					   cur_state <= write1;
	               end if;
	               
	           when write1 =>
	               sig_axi_awvalid <= '1';
				   sig_axi_wvalid <= '1';
				   sig_axi_wstrb <= "1111";
				   sig_axi_wdata <= tgtbuff(tgtbuffidx);
				   -- slave has M_AXI_WREADY 1 by default.
				   -- Let's wait for slave to be really ready
--				   if (M_AXI_WREADY = '1') then
--				      cur_state <= write1;
--				   else
                       tgtbuffidx := tgtbuffidx + 1;
                       cur_state <= write2;
--                   end if;
	               
	           when write2 =>
	           	   if (M_AXI_AWREADY = '1') then
				       sig_axi_awvalid <= '0';
				   else
				       sig_axi_awvalid <= sig_axi_awvalid;
				   end if;
				   
	               if (M_AXI_WREADY = '1') then
                       sig_axi_wvalid <= '1';
                       sig_axi_wstrb <= "1111";
                       sig_axi_wdata <= tgtbuff(tgtbuffidx);
	 
	                   if (tgtbuffidx = 15) then
	                       sig_axi_wlast <= '1';
	                       remainlen := remainlen - burstsize;
	                       cur_state <= bresp1;
	                   else
						   cur_state <= write2;
	                   end if;
	                   tgtbuffidx := tgtbuffidx + 1;
	               else	
					   cur_state <= write2;
	               end if;
	               
	           when bresp1 =>
	           	   sig_axi_wvalid <= '0';
	           	   
	               if (M_AXI_BVALID = '1') then
	                   sig_axi_bready <= '1';
	                   if (remainlen > 0) then
	                       sig_axi_araddr <= std_logic_vector(unsigned(sig_axi_araddr) + to_unsigned(burstsize, 31));
	                       sig_axi_awaddr <= std_logic_vector(unsigned(sig_axi_awaddr) + to_unsigned(burstsize, 31));
	                       cur_state <= read1;
	                   else 
	                       sig_dma_intr <= '1';
	                       cur_state <= idle;
	                   end if;
	               else
	                   cur_state <= bresp1;
	               end if;
	           when intr1 =>
	           end case;	      
	       end if;
	   end if;	
	end process;	  
end implementation;
