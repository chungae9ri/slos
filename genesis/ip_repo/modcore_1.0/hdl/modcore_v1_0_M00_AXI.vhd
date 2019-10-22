library ieee;
library unisim;
use unisim.vcomponents.all;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity modcore_v1_0_M00_AXI is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line

		-- Base address of targeted slave
		C_M_TARGET_SLAVE_BASE_ADDR	: std_logic_vector	:= x"30000000";
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
		-- M_TRIG_MEM_CPY : in std_logic;
		-- Initiate AXI transactions
		M_DMA_IRQ_DONE : out std_logic;
		INIT_AXI_TXN	: in std_logic;
        M_SRC_ADDR : in std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
        M_SRC_LEN : in std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
        M_TGT_ADDR : in std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
        M_DMA_IRQ : out std_logic;
        M_INTR_DONE : in std_logic;
		-- User ports ends
		-- Do not modify the ports beyond this line
		-- Asserts when transaction is complete
		-- kyi TXN_DONE	: out std_logic;
		-- Global Clock Signal.
		M_AXI_ACLK	: in std_logic;
		-- Global Reset Singal. This Signal is Active Low
		M_AXI_ARESETN	: in std_logic;
		-- Master Interface Write Address ID
		M_AXI_AWID	: out std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		-- Master Interface Write Address
		M_AXI_AWADDR	: out std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
		-- Burst length. The burst length gives the exact number of transfers in a burst
		M_AXI_AWLEN	: out std_logic_vector(7 downto 0);
		-- Burst size. This signal indicates the size of each transfer in the burst
		M_AXI_AWSIZE	: out std_logic_vector(2 downto 0);
		-- Burst type. The burst type and the size information, 
    -- determine how the address for each transfer within the burst is calculated.
		M_AXI_AWBURST	: out std_logic_vector(1 downto 0);
		-- Lock type. Provides additional information about the
    -- atomic characteristics of the transfer.
		M_AXI_AWLOCK	: out std_logic;
		-- Memory type. This signal indicates how transactions
    -- are required to progress through a system.
		M_AXI_AWCACHE	: out std_logic_vector(3 downto 0);
		-- Protection type. This signal indicates the privilege
    -- and security level of the transaction, and whether
    -- the transaction is a data access or an instruction access.
		M_AXI_AWPROT	: out std_logic_vector(2 downto 0);
		-- Quality of Service, QoS identifier sent for each write transaction.
		M_AXI_AWQOS	: out std_logic_vector(3 downto 0);
		-- Write address valid. This signal indicates that
    -- the channel is signaling valid write address and control information.
		M_AXI_AWVALID	: out std_logic;
		-- Write address ready. This signal indicates that
    -- the slave is ready to accept an address and associated control signals
		M_AXI_AWREADY	: in std_logic;
		-- Master Interface Write Data.
		M_AXI_WDATA	: out std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
		-- Write strobes. This signal indicates which byte
    -- lanes hold valid data. There is one write strobe
    -- bit for each eight bits of the write data bus.
		M_AXI_WSTRB	: out std_logic_vector(C_M_AXI_DATA_WIDTH/8-1 downto 0);
		-- Write last. This signal indicates the last transfer in a write burst.
		M_AXI_WLAST	: out std_logic;
		-- Write valid. This signal indicates that valid write
    -- data and strobes are available
		M_AXI_WVALID	: out std_logic;
		-- Write ready. This signal indicates that the slave
    -- can accept the write data.
		M_AXI_WREADY	: in std_logic;
		-- Master Interface Write Response.
		M_AXI_BID	: in std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		-- Write response. This signal indicates the status of the write transaction.
		M_AXI_BRESP	: in std_logic_vector(1 downto 0);
		-- Write response valid. This signal indicates that the
    -- channel is signaling a valid write response.
		M_AXI_BVALID	: in std_logic;
		-- Response ready. This signal indicates that the master
    -- can accept a write response.
		M_AXI_BREADY	: out std_logic;
		-- Master Interface Read Address.
		M_AXI_ARID	: out std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		-- Read address. This signal indicates the initial
    -- address of a read burst transaction.
		M_AXI_ARADDR	: out std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
		-- Burst length. The burst length gives the exact number of transfers in a burst
		M_AXI_ARLEN	: out std_logic_vector(7 downto 0);
		-- Burst size. This signal indicates the size of each transfer in the burst
		M_AXI_ARSIZE	: out std_logic_vector(2 downto 0);
		-- Burst type. The burst type and the size information, 
    -- determine how the address for each transfer within the burst is calculated.
		M_AXI_ARBURST	: out std_logic_vector(1 downto 0);
		-- Lock type. Provides additional information about the
    -- atomic characteristics of the transfer.
		M_AXI_ARLOCK	: out std_logic;
		-- Memory type. This signal indicates how transactions
    -- are required to progress through a system.
		M_AXI_ARCACHE	: out std_logic_vector(3 downto 0);
		-- Protection type. This signal indicates the privilege
    -- and security level of the transaction, and whether
    -- the transaction is a data access or an instruction access.
		M_AXI_ARPROT	: out std_logic_vector(2 downto 0);
		-- Quality of Service, QoS identifier sent for each read transaction
		M_AXI_ARQOS	: out std_logic_vector(3 downto 0);
		-- Write address valid. This signal indicates that
    -- the channel is signaling valid read address and control information
		M_AXI_ARVALID	: out std_logic;
		-- Read address ready. This signal indicates that
    -- the slave is ready to accept an address and associated control signals
		M_AXI_ARREADY	: in std_logic;
		-- Read ID tag. This signal is the identification tag
    -- for the read data group of signals generated by the slave.
		M_AXI_RID	: in std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		-- Master Read Data
		M_AXI_RDATA	: in std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
		-- Read response. This signal indicates the status of the read transfer
		M_AXI_RRESP	: in std_logic_vector(1 downto 0);
		-- Read last. This signal indicates the last transfer in a read burst
		M_AXI_RLAST	: in std_logic;
		-- Read valid. This signal indicates that the channel
    -- is signaling the required read data.
		M_AXI_RVALID	: in std_logic;
		-- Read ready. This signal indicates that the master can
    -- accept the read data and response informationr.
		M_AXI_RREADY	: out std_logic
	);
end modcore_v1_0_M00_AXI;
  
architecture implementation of modcore_v1_0_M00_AXI is

	-- function called clogb2 that returns an integer which has the
	--value of the ceiling of the log base 2
  
	function clogb2 (bit_depth : integer) return integer is            
	 	variable depth  : integer := bit_depth;                               
	 	variable count  : integer := 1;                                       
	 begin                                                                   
	 	 for clogb2 in 1 to bit_depth loop  -- Works for up to 32 bit integers
	      if (bit_depth <= 2) then                                           
	        count := 1;                                                      
	      else                                                               
	        if(depth <= 1) then                                              
	 	       count := count;                                                
	 	     else                                                             
	 	       depth := depth / 2;                                            
	          count := count + 1;                                            
	 	     end if;                                                          
	 	   end if;                                                            
	   end loop;                                                             
	   return(count);        	                                              
	 end;                                                                    

	-- C_TRANSACTIONS_NUM is the width of the index counter for
	-- number of beats in a burst write or burst read transaction.
	 constant  C_TRANSACTIONS_NUM : integer := clogb2(C_M_AXI_BURST_LEN-1);
	-- Burst length for transactions, in C_M_AXI_DATA_WIDTHs.
	-- Non-2^n lengths will eventually cause bursts across 4K address boundaries.
	 constant  C_MASTER_LENGTH  : integer := 12;
	-- total number of burst transfers is master length divided by burst length and burst size
	 constant  C_NO_BURSTS_REQ  : integer := (C_MASTER_LENGTH-clogb2((C_M_AXI_BURST_LEN*C_M_AXI_DATA_WIDTH/8)-1));
	 --constant  C_NO_BURSTS_REQ  : integer := 1;
	-- Example State machine to initialize counter, initialize write transactions, 
	 -- initialize read transactions and comparison of read data with the 
	 -- written data words.
	 type state is ( IDLE, -- This state initiates AXI4Lite transaction  
	 							-- after the state machine changes state to INIT_WRITE
	 							-- when there is 0 to 1 transition on INIT_AXI_TXN
	 				INIT_WRITE,   -- This state initializes write transaction,
	 							-- once writes are done, the state machine 
	 							-- changes state to INIT_READ 
	 				INIT_READ,    -- This state initializes read transaction
	 							-- once reads are done, the state machine 
	 							-- changes state to INIT_COMPARE 
	 				INIT_INTR);-- This state issues the status of comparison 
	 							-- of the written data with the read data

	 signal mst_exec_state  : state ; 
  
	-- AXI4FULL signals
	--AXI4 internal temp signals
	signal axi_awaddr	: std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_awvalid	: std_logic;
	signal axi_wdata	: std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
	signal axi_wlast	: std_logic;
	signal axi_wvalid	: std_logic;
	signal axi_bready	: std_logic;
	signal axi_araddr	: std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_arvalid	: std_logic;
	signal axi_rready	: std_logic;
	--write beat count in a burst
	--signal write_index	: std_logic_vector(C_TRANSACTIONS_NUM downto 0);
	signal write_index	: integer;
	--read beat count in a burst
	-- signal read_index	: std_logic_vector(C_TRANSACTIONS_NUM downto 0);
	signal read_index	: integer;
	--size of C_M_AXI_BURST_LEN length burst in bytes
	signal burst_size_bytes	: std_logic_vector(C_TRANSACTIONS_NUM+2 downto 0);
	--The burst counters are used to track the number of burst transfers of C_M_AXI_BURST_LEN burst length needed to transfer 2^C_MASTER_LENGTH bytes of data.
	--signal write_burst_counter	: std_logic_vector(C_NO_BURSTS_REQ downto 0);
	signal write_burst_counter : integer;
	-- kyi signal read_burst_counter	: std_logic_vector(C_NO_BURSTS_REQ downto 0);
	signal burst_trans_len	: std_logic_vector(31 downto 0);  -- kyi 
	signal read_burst_counter	: integer;
	signal start_single_burst_write	: std_logic;
	signal start_single_burst_read	: std_logic;
	signal writes_done	: std_logic;
	signal reads_done	: std_logic;
	signal error_reg	: std_logic;
	signal compare_done	: std_logic;
	signal read_mismatch	: std_logic;
	signal burst_write_active	: std_logic;
	signal burst_read_active	: std_logic;
	signal expected_rdata	: std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
	--Interface response error flags
	signal write_resp_error	: std_logic;
	signal read_resp_error	: std_logic;
	signal wnext	: std_logic;
	signal rnext	: std_logic;
	signal init_txn_ff	: std_logic;
	signal init_txn_ff2	: std_logic;
	signal init_txn_edge	: std_logic;
	signal init_txn_pulse	: std_logic;
-- BRAM out ports S
	signal cascadeout_A : std_logic;
	signal cascadeout_B : std_logic;
	signal dbiterr : std_logic;
	signal ECCPARITY : std_logic_vector(7 downto 0);
	signal RDADDRECC : std_logic_vector (8 downto 0);
	signal SBITERR : std_logic;
	signal DOADO : std_logic_vector(31 downto 0);
	signal DOPADOP : std_logic_vector(3 downto 0);
	signal DOBDO : std_logic_vector(31 downto 0);
    signal DOPBDOP : std_logic_vector(3 downto 0);
    signal ADDRARDADDR, BRAMAddr : std_logic_vector(15 downto 0);
    signal ENARDEN : std_logic;
    signal REGCEAREGCE : std_logic;
    signal RSTRAMARSTRAM : std_logic;
    signal RSTREGARSTREG : std_logic;
    signal DIADI : std_logic_vector(31 downto 0);
    signal DIPADIP : std_logic_vector(3 downto 0);
    signal DIPBDIP : std_logic_vector(3 downto 0);
    signal ADDRBWRADDR : std_logic_vector(15 downto 0);
    signal ENBWREN : std_logic;
    signal REGCEB : std_logic;
    signal RSTRAMB : std_logic;
    signal RSTREGB : std_logic;
    signal WEA : std_logic_vector(3 downto 0);
    signal WEBWE : std_logic_vector(7 downto 0);
    signal DIBDI : std_logic_vector(31 downto 0);
-- BRAM out ports E
	-- kyi S
	signal dma_irq : std_logic;
	signal reg_src_addr : std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
	signal reg_tgt_addr : std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
	type RDMem_type is array (0 to 17) of std_logic_vector(31 downto 0);
	signal RDMem : RDMem_type := (others => (others =>'0'));
	signal RDMemAddr : integer; -- 16 rows of 32 bit word
--	attribute MARK_DEBUG : string;
--	attribute MARK_DEBUG of dma_irq : signal is "TRUE";
--	attribute MARK_DEBUG of reg_src_addr : signal is "TRUE";
--	attribute MARK_DEBUG of reg_tgt_addr : signal is "TRUE";
--	attribute MARK_DEBUG of init_txn_pulse : signal is "TRUE";
--	attribute MARK_DEBUG of start_single_burst_write : signal is "TRUE";
--	attribute MARK_DEBUG of burst_write_active : signal is "TRUE";
--	attribute MARK_DEBUG of axi_bready : signal is "TRUE";
--	attribute MARK_DEBUG of axi_awaddr : signal is "TRUE";
--	attribute MARK_DEBUG of axi_awvalid : signal is "TRUE";
--	attribute MARK_DEBUG of axi_wdata : signal is "TRUE";
--	attribute MARK_DEBUG of axi_wvalid : signal is "TRUE";
--	attribute MARK_DEBUG of axi_wlast : signal is "TRUE";
--	attribute MARK_DEBUG of wnext : signal is "TRUE";
--	attribute MARK_DEBUG of write_index : signal is "TRUE";
--	attribute MARK_DEBUG of write_burst_counter : signal is "TRUE";
--	attribute MARK_DEBUG of writes_done : signal is "TRUE";
--	attribute MARK_DEBUG of init_txn_ff : signal is "TRUE"; 
--	attribute MARK_DEBUG of init_txn_ff2 : signal is "TRUE";
-- 	attribute MARK_DEBUG of RDMemAddr : signal is "TRUE";
--	attribute MARK_DEBUG of RDMem : signal is "TRUE";
--	attribute MARK_DEBUG of start_single_burst_read : signal is "TRUE";
--	attribute MARK_DEBUG of burst_read_active :signal is "TRUE";
--	attribute MARK_DEBUG of axi_arvalid : signal is "TRUE";
--	attribute MARK_DEBUG of axi_araddr : signal is "TRUE";
--	attribute MARK_DEBUG of rnext : signal is "TRUE";
--	attribute MARK_DEBUG of axi_rready : signal is "TRUE";
--	attribute MARK_DEBUG of burst_trans_len : signal is "TRUE";
--	attribute MARK_DEBUG of read_burst_counter : signal is "TRUE";
--	attribute MARK_DEBUG of read_index : signal is "TRUE";
--	attribute MARK_DEBUG of reads_done : signal is "TRUE";
--    attribute MARK_DEBUG of mst_exec_state : signal is "TRUE";
--    attribute MARK_DEBUG of WEA : signal is "TRUE";
--    attribute MARK_DEBUG of BRAMAddr : signal is "TRUE";
--    attribute MARK_DEBUG of DIADI : signal is "TRUE";
--    attribute MARK_DEBUG of DOADO : signal is "TRUE";
--     attribute MARK_DEBUG of ENARDEN : signal is "TRUE";
	-- kyi E

begin
	-- I/O Connections assignments
	M_DMA_IRQ <= dma_irq;
	M_DMA_IRQ_DONE <= not dma_irq;
	--I/O Connections. Write Address (AW)
	M_AXI_AWID	<= (others => '0');
	--The AXI address is a concatenation of the target base address + active offset range
	M_AXI_AWADDR	<= std_logic_vector(unsigned(reg_tgt_addr) + unsigned(axi_awaddr));
	--Burst LENgth is number of transaction beats, minus 1
	M_AXI_AWLEN	<= std_logic_vector(to_unsigned(C_M_AXI_BURST_LEN-1,8));
	--Size should be C_M_AXI_DATA_WIDTH, in 2^SIZE bytes, otherwise narrow bursts are used
	M_AXI_AWSIZE	<= std_logic_vector( to_unsigned(clogb2((C_M_AXI_DATA_WIDTH/8)-1), 3) );
	--INCR burst type is usually used, except for keyhole bursts
	M_AXI_AWBURST	<= "01";
	M_AXI_AWLOCK	<= '0';
	--Update value to 4'b0011 if coherent accesses to be used via the Zynq ACP port. Not Allocated, Modifiable, not Bufferable. Not Bufferable since this example is meant to test memory, not intermediate cache. 
	M_AXI_AWCACHE	<= "0010";
	M_AXI_AWPROT	<= "000";
	M_AXI_AWQOS	<= x"0";
	M_AXI_AWVALID	<= axi_awvalid;
	--Write Data(W)
	M_AXI_WDATA	<= std_logic_vector(256 - unsigned(RDMem(write_index)(31 downto 24)) mod 256) & std_logic_vector(256 - unsigned(RDMem(write_index)(23 downto 16)) mod 256) & std_logic_vector(256 - unsigned(RDMem(write_index)(15 downto 8)) mod 256) & std_logic_vector(256 - unsigned(RDMem(write_index)(7 downto 0)) mod 256);
--    M_AXI_WDATA <= DOADO;
	--All bursts are complete and aligned in this example
	M_AXI_WSTRB	<= (others => '1');
	M_AXI_WLAST	<= axi_wlast;
	M_AXI_WVALID	<= axi_wvalid;
	--Write Response (B)
	M_AXI_BREADY	<= axi_bready;
	--Read Address (AR)
	M_AXI_ARID	<= (others => '0');
	M_AXI_ARADDR	<= std_logic_vector( unsigned(reg_src_addr) + unsigned( axi_araddr ) );
	--Burst LENgth is number of transaction beats, minus 1
	-- kyi M_AXI_ARLEN	<= std_logic_vector( to_unsigned(C_M_AXI_BURST_LEN - 1, 8) );
	M_AXI_ARLEN	<= x"10"; -- 16 bytes len of a burst
	--Size should be C_M_AXI_DATA_WIDTH, in 2^n bytes, otherwise narrow bursts are used
	-- kyi M_AXI_ARSIZE	<= std_logic_vector( to_unsigned( clogb2((C_M_AXI_DATA_WIDTH/8)-1),3 ));
	M_AXI_ARSIZE	<= "010"; -- 4 bytes beat size
	--INCR burst type is usually used, except for keyhole bursts
	M_AXI_ARBURST	<= "01";
	M_AXI_ARLOCK	<= '0';
	--Update value to 4'b0011 if coherent accesses to be used via the Zynq ACP port. Not Allocated, Modifiable, not Bufferable. Not Bufferable since this example is meant to test memory, not intermediate cache. 
	M_AXI_ARCACHE	<= "0010";
	M_AXI_ARPROT	<= "000";
	M_AXI_ARQOS	<= x"0";
	M_AXI_ARVALID	<= axi_arvalid;
	--Read and Read Response (R)
	M_AXI_RREADY	<= axi_rready;
	--Example design I/O
	-- kyi TXN_DONE	<= compare_done;
	--Burst size in bytes
	burst_size_bytes	<= std_logic_vector( to_unsigned((C_M_AXI_BURST_LEN * (C_M_AXI_DATA_WIDTH/8)),C_TRANSACTIONS_NUM+3) );
	init_txn_pulse	<= ( not init_txn_ff2)  and  init_txn_ff;

	DIPADIP <= (others => '0');
	DIPBDIP <= (others => '0');
	DOPADOP <= (others => '0');
	DOPBDOP <= (others => '0');

------------------------------------------------------------------
RAMB36E1_inst : RAMB36E1
generic map (
-- Address Collision Mode: "PERFORMANCE" or "DELAYED_WRITE"
RDADDR_COLLISION_HWCONFIG => "DELAYED_WRITE",
-- Collision check: Values ("ALL", "WARNING_ONLY", "GENERATE_X_ONLY" or "NONE")
SIM_COLLISION_CHECK => "ALL",
-- DOA_REG, DOB_REG: Optional output register (0 or 1)
DOA_REG => 1,
DOB_REG => 1,
EN_ECC_READ => FALSE, -- Enable ECC decoder, FALSE, TRUE
EN_ECC_WRITE => FALSE, -- Enable ECC encoder, FALSE, TRUE
-- INITP_00 to INITP_0F: Initial contents of the parity memory array
INITP_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_04 => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_05 => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_06 => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_07 => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_08 => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_09 => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_0A => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_0B => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_0C => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_0D => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_0E => X"0000000000000000000000000000000000000000000000000000000000000000",
INITP_0F => X"0000000000000000000000000000000000000000000000000000000000000000",
-- INIT_00 to INIT_7F: Initial contents of the data memory array
INIT_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_04 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_05 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_06 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_07 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_08 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_09 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_0A => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_0B => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_0C => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_0D => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_0E => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_0F => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_10 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_11 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_12 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_13 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_14 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_15 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_16 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_17 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_18 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_19 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_1A => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_1B => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_1C => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_1D => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_1E => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_1F => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_20 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_21 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_22 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_23 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_24 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_25 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_26 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_27 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_28 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_29 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_2A => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_2B => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_2C => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_2D => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_2E => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_2F => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_30 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_31 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_32 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_33 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_34 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_35 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_36 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_37 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_38 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_39 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_3A => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_3B => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_3C => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_3D => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_3E => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_3F => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_40 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_41 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_42 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_43 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_44 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_45 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_46 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_47 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_48 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_49 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_4A => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_4B => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_4C => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_4D => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_4E => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_4F => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_50 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_51 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_52 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_53 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_54 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_55 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_56 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_57 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_58 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_59 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_5A => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_5B => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_5C => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_5D => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_5E => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_5F => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_60 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_61 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_62 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_63 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_64 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_65 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_66 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_67 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_68 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_69 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_6A => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_6B => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_6C => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_6D => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_6E => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_6F => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_70 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_71 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_72 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_73 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_74 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_75 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_76 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_77 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_78 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_79 => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_7A => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_7B => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_7C => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_7D => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_7E => X"0000000000000000000000000000000000000000000000000000000000000000",
INIT_7F => X"0000000000000000000000000000000000000000000000000000000000000000",
-- INIT_A, INIT_B: Initial values on output ports
INIT_A => X"000000000",
INIT_B => X"000000000",
-- Initialization File: RAM initialization file
INIT_FILE => "NONE",
-- RAM Mode: "SDP" or "TDP"
RAM_MODE => "TDP",
-- RAM_EXTENSION_A, RAM_EXTENSION_B: Selects cascade mode ("UPPER", "LOWER", or "NONE")
RAM_EXTENSION_A => "NONE",
RAM_EXTENSION_B => "NONE",
-- READ_WIDTH_A/B, WRITE_WIDTH_A/B: Read/write width per port
READ_WIDTH_A => 36, -- 0-72
READ_WIDTH_B => 36, -- 0-36
WRITE_WIDTH_A => 36, -- 0-36
WRITE_WIDTH_B => 36, -- 0-72
-- RSTREG_PRIORITY_A, RSTREG_PRIORITY_B: Reset or enable priority ("RSTREG" or "REGCE")
RSTREG_PRIORITY_A => "RSTREG",
RSTREG_PRIORITY_B => "RSTREG",
-- SRVAL_A, SRVAL_B: Set/reset value for output
SRVAL_A => X"000000000",
SRVAL_B => X"000000000",
-- Simulation Device: Must be set to "7SERIES" for simulation behavior
SIM_DEVICE => "7SERIES",
-- WriteMode: Value on output upon a write ("WRITE_FIRST", "READ_FIRST", or "NO_CHANGE")
WRITE_MODE_A => "WRITE_FIRST",
WRITE_MODE_B => "WRITE_FIRST"
)
port map (
-- Cascade Signals: 1-bit (each) output: BRAM cascade ports (to create 64kx1)
CASCADEOUTA => open, -- 1-bit output: A port cascade
CASCADEOUTB => open, -- 1-bit output: B port cascade
-- ECC Signals: 1-bit (each) output: Error Correction Circuitry ports
DBITERR => dbiterr, -- 1-bit output: Double bit error status
ECCPARITY => ECCPARITY, -- 8-bit output: Generated error correction parity
RDADDRECC => RDADDRECC, -- 9-bit output: ECC read address
SBITERR => SBITERR, -- 1-bit output: Single bit error status
-- Port A Data: 32-bit (each) output: Port A data
DOADO => DOADO, -- 32-bit output: A port data/LSB data
DOPADOP => DOPADOP, -- 4-bit output: A port parity/LSB parity
-- Port B Data: 32-bit (each) output: Port B data
DOBDO => DOBDO, -- 32-bit output: B port data/MSB data
DOPBDOP => DOPBDOP, -- 4-bit output: B port parity/MSB parity
-- Cascade Signals: 1-bit (each) input: BRAM cascade ports (to create 64kx1)
CASCADEINA => '0', -- 1-bit input: A port cascade
CASCADEINB => '0', -- 1-bit input: B port cascade
-- ECC Signals: 1-bit (each) input: Error Correction Circuitry ports
INJECTDBITERR => '0', -- 1-bit input: Inject a double bit error
INJECTSBITERR => '0', -- 1-bit input: Inject a single bit error
-- Port A Address/Control Signals: 16-bit (each) input: Port A address and control signals (read port
-- when RAM_MODE="SDP")
ADDRARDADDR => ADDRARDADDR, -- 16-bit input: A port address/Read address
CLKARDCLK => M_AXI_ACLK, -- 1-bit input: A port clock/Read clock
ENARDEN => ENARDEN, -- ENARDEN, -- 1-bit input: A port enable/Read enable
REGCEAREGCE => REGCEAREGCE, -- REGCEAREGCE, -- 1-bit input: A port register enable/Register enable
RSTRAMARSTRAM => RSTRAMARSTRAM, -- 1-bit input: A port set/reset
RSTREGARSTREG => RSTREGARSTREG, -- 1-bit input: A port register set/reset
WEA => WEA, -- 4-bit input: A port write enable
-- Port A Data: 32-bit (each) input: Port A data
DIADI => DIADI, -- 32-bit input: A port data/LSB data
DIPADIP => b"0000", -- 4-bit input: A port parity/LSB parity
-- Port B Address/Control Signals: 16-bit (each) input: Port B address and control signals (write port
-- when RAM_MODE="SDP")
ADDRBWRADDR => ADDRBWRADDR, -- 16-bit input: B port address/Write address
CLKBWRCLK => M_AXI_ACLK, -- 1-bit input: B port clock/Write clock
ENBWREN => ENBWREN, -- 1-bit input: B port enable/Write enable
REGCEB => REGCEB, -- 1-bit input: B port register enable
RSTRAMB => RSTRAMB, -- 1-bit input: B port set/reset
RSTREGB => RSTREGB, -- 1-bit input: B port register set/reset
WEBWE => WEBWE, -- 8-bit input: B port write enable/Write enable
-- Port B Data: 32-bit (each) input: Port B data
DIBDI => DIBDI, -- 32-bit input: B port data/MSB data
DIPBDIP => b"0000" -- 4-bit input: B port parity/MSB parity
);
-- End of RAMB36E1_inst instantiation

------------------------------------------------------------------


	--Generate a pulse to initiate AXI transaction.
	process(M_AXI_ACLK)                                                          
	begin                                                                             
	  if (rising_edge (M_AXI_ACLK)) then                                              
	      -- Initiates AXI transaction delay        
	    if (M_AXI_ARESETN = '0' ) then                                                
	      init_txn_ff <= '0';                                                   
	        init_txn_ff2 <= '0';                                                          
	    else                                                                                       
	      init_txn_ff <= INIT_AXI_TXN;
	        init_txn_ff2 <= init_txn_ff;                                                                     
	    end if;                                                                       
	  end if;                                                                         
	end process; 


	----------------------
	--Write Address Channel
	----------------------

	-- The purpose of the write address channel is to request the address and 
	-- command information for the entire transaction.  It is a single beat
	-- of information.

	-- The AXI4 Write address channel in this example will continue to initiate
	-- write commands as fast as it is allowed by the slave/interconnect.
	-- The address will be incremented on each accepted address transaction,
	-- by burst_size_byte to point to the next address. 

	  process(M_AXI_ACLK)                                            
	  begin                                                                
	    if (rising_edge (M_AXI_ACLK)) then                                 
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then                                   
	        axi_awvalid <= '0';                                            
	      else                                                             
	        -- If previously not valid , start next transaction            
	        if (axi_awvalid = '0' and start_single_burst_write = '1') then 
	          axi_awvalid <= '1';                                          
	          -- Once asserted, VALIDs cannot be deasserted, so axi_awvalid
	          -- must wait until transaction is accepted                   
	        elsif (M_AXI_AWREADY = '1' and axi_awvalid = '1') then         
	          axi_awvalid <= '0';                                          
	        else                                                           
	          axi_awvalid <= axi_awvalid;                                  
	        end if;                                                        
	      end if;                                                          
	    end if;                                                            
	  end process;                                                         
	                                                                       
	-- Next address after AWREADY indicates previous address acceptance    
	  process(M_AXI_ACLK)                                                  
	  begin                                                                
	    if (rising_edge (M_AXI_ACLK)) then                                 
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then                                   
	        axi_awaddr <= (others => '0');                                 
	      else                                                             
	        if (M_AXI_AWREADY= '1' and axi_awvalid = '1') then             
	          axi_awaddr <= std_logic_vector(unsigned(axi_awaddr) + unsigned(burst_size_bytes));                 
	        end if;                                                        
	      end if;                                                          
	    end if;                                                            
	  end process;                                                         


	----------------------
	--Write Data Channel
	----------------------

	--The write data will continually try to push write data across the interface.

	--The amount of data accepted will depend on the AXI slave and the AXI
	--Interconnect settings, such as if there are FIFOs enabled in interconnect.

	--Note that there is no explicit timing relationship to the write address channel.
	--The write channel has its own throttling flag, separate from the AW channel.

	--Synchronization between the channels must be determined by the user.

	--The simpliest but lowest performance would be to only issue one address write
	--and write data burst at a time.

	--In this example they are kept in sync by using the same address increment
	--and burst sizes. Then the AW and W channels have their transactions measured
	--with threshold counters as part of the user logic, to make sure neither 
	--channel gets too far ahead of each other.

	--Forward movement occurs when the write channel is valid and ready

	  wnext <= M_AXI_WREADY and axi_wvalid;                                       
	                                                                                    
	-- WVALID logic, similar to the axi_awvalid always block above                      
	  process(M_AXI_ACLK)                                                               
	  begin                                                                             
	    if (rising_edge (M_AXI_ACLK)) then                                              
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then                                                
	        axi_wvalid <= '0';                                                          
	      else                                                                          
	        if (axi_wvalid = '0' and start_single_burst_write = '1') then               
	          -- If previously not valid, start next transaction                        
	          axi_wvalid <= '1';                                                        
	          --     /* If WREADY and too many writes, throttle WVALID                  
	          --      Once asserted, VALIDs cannot be deasserted, so WVALID             
	          --      must wait until burst is complete with WLAST */                   
	        elsif (wnext = '1' and axi_wlast = '1') then                                
	          axi_wvalid <= '0';                                                        
	        else                                                                        
	          axi_wvalid <= axi_wvalid;                                                 
	        end if;                                                                     
	      end if;                                                                       
	    end if;                                                                         
	  end process;                                                                      
	                                                                                    
	--WLAST generation on the MSB of a counter underflow                                
	-- WVALID logic, similar to the axi_awvalid always block above                      
	  process(M_AXI_ACLK)                                                               
	  begin                                                                             
	    if (rising_edge (M_AXI_ACLK)) then                                              
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then                                                
	        axi_wlast <= '0';                                                           
	        -- axi_wlast is asserted when the write index                               
	        -- count reaches the penultimate count to synchronize                       
	        -- with the last write data when write_index is b1111                       
	        -- elsif (&(write_index[C_TRANSACTIONS_NUM-1:1])&& ~write_index[0] && wnext)
	      else                                                                          
	        -- if ((((write_index = std_logic_vector(to_unsigned(C_M_AXI_BURST_LEN-2,C_TRANSACTIONS_NUM+1))) and C_M_AXI_BURST_LEN >= 2) and wnext = '1') or (C_M_AXI_BURST_LEN = 1)) then
	        if (((write_index = 14) and wnext = '1')) then
	          axi_wlast <= '1';                                                         
	          -- Deassrt axi_wlast when the last write data has been                    
	          -- accepted by the slave with a valid response                            
	        elsif (wnext = '1') then                                                    
	          axi_wlast <= '0';                                                         
	        end if;                                                                     
	      end if;                                                                       
	    end if;                                                                         
	  end process;                                                                      
	                                                                                    
	-- Burst length counter. Uses extra counter register bit to indicate terminal       
	-- count to reduce decode logic */                                                  
	-- kyi check write_index overflow
	  process(M_AXI_ACLK)                                                               
	  begin                                                                             
	    if (rising_edge (M_AXI_ACLK)) then                                              
	      if (M_AXI_ARESETN = '0' or start_single_burst_write = '1' or init_txn_pulse = '1') then               
	        write_index <= 0;                                             
	      else                                                                          
	        --if (wnext = '1' and (write_index /= std_logic_vector(to_unsigned(C_M_AXI_BURST_LEN-1,C_TRANSACTIONS_NUM+1)))) then                
	        if (wnext = '1' and (write_index /= 15)) then                
	          write_index <= write_index + 1;                                         
	        end if;                                                                     
	      end if;                                                                       
	    end if;                                                                         
	  end process;                                                                      
	                                                                                    
	-- Write Data Generator                                                             
	-- Data pattern is only a simple incrementing count from 0 for each burst  */       
	  process(M_AXI_ACLK)                                                               
	  variable  sig_one : integer := 1;                                                 
	  begin                                                                             
	    if (rising_edge (M_AXI_ACLK)) then                                              
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1' or start_single_burst_write = '1') then                                                
	        --axi_wdata <= std_logic_vector (to_unsigned(sig_one, C_M_AXI_DATA_WIDTH));            
	        axi_wdata <= (others => '0');
	        --elsif (wnext && axi_wlast)                                                
	        --  axi_wdata <= 'b0;                                                       
	      else                                                                          
	        if (wnext = '1') then                                             
	          axi_wdata <= RDMem(write_index);
	        end if;                                                                     
	      end if;                                                                       
	    end if;                                                                         
	  end process;                                                                      


	------------------------------
	--Write Response (B) Channel
	------------------------------

	--The write response channel provides feedback that the write has committed
	--to memory. BREADY will occur when all of the data and the write address
	--has arrived and been accepted by the slave.

	--The write issuance (number of outstanding write addresses) is started by 
	--the Address Write transfer, and is completed by a BREADY/BRESP.

	--While negating BREADY will eventually throttle the AWREADY signal, 
	--it is best not to throttle the whole data channel this way.

	--The BRESP bit [1] is used indicate any errors from the interconnect or
	--slave for the entire write burst. This example will capture the error 
	--into the ERROR output. 

	  process(M_AXI_ACLK)                                             
	  begin                                                                 
	    if (rising_edge (M_AXI_ACLK)) then                                  
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then                                    
	        axi_bready <= '0';                                              
	        -- accept/acknowledge bresp with axi_bready by the master       
	        -- when M_AXI_BVALID is asserted by slave                       
	      else                                                              
	        if (M_AXI_BVALID = '1' and axi_bready = '0') then               
	          axi_bready <= '1';                                            
	          -- deassert after one clock cycle                             
	        elsif (axi_bready = '1') then                                   
	          axi_bready <= '0';                                            
	        end if;                                                         
	      end if;                                                           
	    end if;                                                             
	  end process;                                                          

	------------------------------
	--Read Address Channel
	------------------------------

	--The Read Address Channel (AW) provides a similar function to the
	--Write Address channel- to provide the tranfer qualifiers for the burst.

	--In this example, the read address increments in the same
	--manner as the write address channel.

	  process(M_AXI_ACLK)										  
	  begin                                                              
	    if (rising_edge (M_AXI_ACLK)) then                               
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then                                 
	        axi_arvalid <= '0';                                          
	     -- If previously not valid , start next transaction             
	      else                                                           
	        if (axi_arvalid = '0' and start_single_burst_read = '1') then
	          axi_arvalid <= '1';                                        
	        elsif (M_AXI_ARREADY = '1' and axi_arvalid = '1') then       
	          axi_arvalid <= '0';                                        
	        end if;                                                      
	      end if;                                                        
	    end if;                                                          
	  end process;                                                       
	                                                                     
	-- Next address after ARREADY indicates previous address acceptance  
	  process(M_AXI_ACLK)                                                
	  begin                                                              
	    if (rising_edge (M_AXI_ACLK)) then                               
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1' ) then                                 
	        axi_araddr <= (others => '0');                               
	      else                                                           
	        if (M_AXI_ARREADY = '1' and axi_arvalid = '1') then          
	          axi_araddr <= std_logic_vector(unsigned(axi_araddr) + unsigned(burst_size_bytes));               
	        end if;                                                      
	      end if;                                                        
	    end if;                                                          
	  end process;                                                       


	----------------------------------
	--Read Data (and Response) Channel
	----------------------------------

	 -- Forward movement occurs when the channel is valid and ready   
	  rnext <= M_AXI_RVALID and axi_rready;                                 
	                                                                        
	                                                                        
	-- Burst length counter. Uses extra counter register bit to indicate    
	-- terminal count to reduce decode logic                                
	  process(M_AXI_ACLK)                                                   
	  begin                                                                 
	    if (rising_edge (M_AXI_ACLK)) then                                  
	      if (M_AXI_ARESETN = '0' or start_single_burst_read = '1' or init_txn_pulse = '1') then    
	        -- kyi read_index <= (others => '0');                                  
	        read_index <= 0;
		    RDMemAddr <= 0;
	      else                                                              
	        -- if (rnext = '1' and (read_index <= std_logic_vector(to_unsigned(C_M_AXI_BURST_LEN-1,C_TRANSACTIONS_NUM+1)))) then   
	        if (rnext = '1') then   
	          read_index <= read_index + 1;                               
		      RDMemAddr <= RDMemAddr + 1;
	        end if;                                                         
	      end if;                                                           
	    end if;                                                             
	  end process;   
	  
	  -- kyi S
	  -- The cascade feature is not used for Port A(RAM_EXTENSION_A set to NONE). The highest order port A address bit (ADDRARDADDR[15]) must be logic 1
	  ADDRARDADDR(15) <= '1'; 
	  ADDRARDADDR(14 downto 0) <= BRAMAddr(14 downto 0);
	  
	  process(M_AXI_ACLK)
	  begin
	       if (rising_edge(M_AXI_ACLK)) then
	           if (M_AXI_ARESETN = '0' or start_single_burst_read = '1' or start_single_burst_write = '1') then
	               BRAMAddr <= (others => '0'); -- The cascade feature is not used for Port A(RAM_EXTENSION_A set to NONE). The highest order port A address bit (ADDRARDADDR[15]) must be logic 1
	           else
	               if (rnext = '1' or wnext = '1') then
	                   BRAMAddr <= std_logic_vector(unsigned(BRAMAddr) + 1);
	               end if;
	           end if;
	       end if;
	  end process;
	  -- kyi E                                                       
	                                                                        
	--/*                                                                    
	-- The Read Data channel returns the results of the read request        
	--                                                                      
	-- In this example the data checker is always able to accept            
	-- more data, so no need to throttle the RREADY signal                  
	-- */                                                                   
	  process(M_AXI_ACLK)                                                   
	  begin                                                                 
	    if (rising_edge (M_AXI_ACLK)) then                                  
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then             
	        axi_rready <= '0';                                              
	     -- accept/acknowledge rdata/rresp with axi_rready by the master    
	      -- when M_AXI_RVALID is asserted by slave                         
	      else                                                   
	        if (M_AXI_RVALID = '1') then                         
	          if (M_AXI_RLAST = '1' and axi_rready = '1') then   
	            axi_rready <= '0';                               
	           else                                              
	             axi_rready <= '1';                              
	          end if;                                            
	        end if;                                              
	      end if;                                                
	    end if;                                                  
	  end process;  
	                                               
  	----------------------------------
	--Example design throttling
	----------------------------------

	-- For maximum port throughput, this user example code will try to allow
	-- each channel to run as independently and as quickly as possible.

	-- However, there are times when the flow of data needs to be throtted by
	-- the user application. This example application requires that data is
	-- not read before it is written and that the write channels do not
	-- advance beyond an arbitrary threshold (say to prevent an 
	-- overrun of the current read address by the write address).

	-- From AXI4 Specification, 13.13.1: "If a master requires ordering between 
	-- read and write transactions, it must ensure that a response is received 
	-- for the previous transaction before issuing the next transaction."

	-- This example accomplishes this user application throttling through:
	-- -Reads wait for writes to fully complete
	-- -Address writes wait when not read + issued transaction counts pass 
	-- a parameterized threshold
	-- -Writes wait when a not read + active data burst count pass 
	-- a parameterized threshold

	 -- write_burst_counter counter keeps track with the number of burst transaction initiated             
	 -- against the number of burst transactions the master needs to initiate                                    
	  process(M_AXI_ACLK)                                                                                        
	  begin                                                                                                      
	    if (rising_edge (M_AXI_ACLK)) then                                                                       
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then                                                                         
	        write_burst_counter <= 0;                                                              
	      else                                                                                                   
	        if (M_AXI_AWREADY = '1' and axi_awvalid = '1') then                                                  
	            write_burst_counter <= write_burst_counter + 1;                      
	        end if;                                                                                              
	      end if;                                                                                                
	    end if;                                                                                                  
	  end process;                                                                                               
	                                                                                                             
	 -- read_burst_counter counter keeps track with the number of burst transaction initiated                    
	 -- against the number of burst transactions the master needs to initiate                                    
	  process(M_AXI_ACLK)                                                                                        
	  begin                                                                                                      
	    if (rising_edge (M_AXI_ACLK)) then                                                                       
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then                                                                         
	        read_burst_counter <= 0;                                                               
	      else                                                                                                   
	        if (M_AXI_ARREADY = '1' and axi_arvalid = '1') then                                                  
	    --      if (to_integer(unsigned(burst_trans_len)) > read_burst_counter)then                                                 
	            read_burst_counter <= read_burst_counter + 1;                        
	    --     end if;                                                                                            
	        end if;                                                                                              
	      end if;                                                                                                
	    end if;                                                                                                  
	  end process;                                                                                               
	                                                                                                             
	                                                                                                             
	  MASTER_EXECUTION_PROC:process(M_AXI_ACLK)                                                                  
	  begin                                                                                                      
	    if (rising_edge (M_AXI_ACLK)) then                                                                       
	      if (M_AXI_ARESETN = '0') then                                                                         
	        -- reset condition                                                                                   
	        -- All the signals are ed default values under reset condition                                       
	        mst_exec_state     <= IDLE;                                                                  
	        compare_done       <= '0';                                                                           
	        start_single_burst_write <= '0';                                                                     
	        start_single_burst_read  <= '0';                                                                     
	        dma_irq <= '0';
	      else                                                                                                   
	        -- state transition                                                                                  
	        case (mst_exec_state) is                                                                             
	                                                                                                             
	           when IDLE =>                                                                              
	             -- This state is responsible to initiate                               
	             -- AXI transaction when init_txn_pulse is asserted 
	               if ( init_txn_pulse = '1') then       
--	                 mst_exec_state  <= INIT_WRITE;      
                     mst_exec_state  <= INIT_READ;                                                        
	                 compare_done <= '0'; 
	                 dma_irq <= '0';
	               else                                                                                          
	                 mst_exec_state  <= IDLE;  
	                 dma_irq <= '0';
		      ENARDEN <= '0';
	                 REGCEAREGCE <= '0';
	                 RSTRAMARSTRAM <= '1';   
	                 RSTREGARSTREG <= '1';  
	                 
	                 ENBWREN <= '0';
	                 REGCEB <= '0';
                     RSTRAMB <= '1';   
                     RSTREGB <= '1';                                                      
	               end if;                                                                                       
	                                                                                                             
	            when INIT_WRITE =>                                                                               
	              -- This state is responsible to issue start_single_write pulse to                              
	              -- initiate a write transaction. Write transactions will be                                    
	              -- issued until burst_write_active signal is asserted.                                         
	              -- write controller                                                                            
	                                                                                                             
	                if (axi_bready = '1' and M_AXI_BVALID = '1' and write_burst_counter >= to_integer(unsigned(burst_trans_len))) then
	                  mst_exec_state <= INIT_INTR;                                                               
			--elsif (axi_bready = '1' and M_AXI_BVALID = '1' and (write_burst_counter < (to_integer(unsigned(burst_trans_len))))) then
			        elsif (axi_bready = '1' and M_AXI_BVALID = '1') then
	                  mst_exec_state  <= INIT_READ;                                                             
	                else                                                                                         
	                   mst_exec_state  <= INIT_WRITE;                                                             
	                                                                                                             
	                   if (axi_awvalid = '0' and start_single_burst_write = '0' and burst_write_active = '0' ) then 
	                       start_single_burst_write <= '1';
	                   else                                                                                         
	                       start_single_burst_write <= '0'; --Negate to generate a pulse                              
	                   end if;  
	                
		      ENARDEN <= '1';
	                   WEA <= b"0000";
	                   REGCEAREGCE <= '1';
	                   RSTRAMARSTRAM <= '0';
	                   RSTREGARSTREG <= '0';
	                   
	                   WEBWE <= x"00";
	                   ENBWREN <= '1';
	                   REGCEB <= '1';
                       RSTRAMB <= '0';   
                       RSTREGB <= '0';
                     end if;  
                                                                                                        
	            when INIT_READ =>                                                                                
	              -- This state is responsible to issue start_single_read pulse to                               
	              -- initiate a read transaction. Read transactions will be                                      
	              -- issued until burst_read_active signal is asserted.                                          
	              -- read controller                                                                             
	                  if (M_AXI_RVALID = '1' and axi_rready = '1' and M_AXI_RLAST = '1') then                           
	                     mst_exec_state <= INIT_WRITE;                                                              
			         else 
			             mst_exec_state <= INIT_READ;
                                                                         
	                     if (axi_arvalid = '0' and burst_read_active = '0' and start_single_burst_read = '0') then    
	                       start_single_burst_read <= '1';   
	                     else                                                                                            
	                       start_single_burst_read <= '0'; -- Negate to generate a pulse                               
	                     end if; 
	                               			  
		      ENARDEN <= '1';
                        WEA <= b"1111";
                        REGCEAREGCE <= '1';
                        RSTRAMARSTRAM <= '0';
                        RSTREGARSTREG <= '0';
                        
                        WEBWE <= x"11";
                        ENBWREN <= '1';
                        REGCEB <= '1';
                        RSTRAMB <= '0';   
                        RSTREGB <= '0';
                     end if;
	                                                                                                             
	            when INIT_INTR =>
	               if (M_INTR_DONE = '0') then
	                   dma_irq <= '1';
	                   mst_exec_state <= INIT_INTR;
	               else
	                   dma_irq <= '0';
	                   mst_exec_state <= IDLE;
	               end if;
	               
	            when others  =>                                                                                  
	              mst_exec_state  <= IDLE;                                                               
	          end case  ;                                                                                        
	       end if;                                                                                               
	    end if;                                                                                                  
	  end process;                                                                                               
	                                                                                                             
	                                                                                                             
	  -- burst_write_active signal is asserted when there is a burst write transaction                           
	  -- is initiated by the assertion of start_single_burst_write. burst_write_active                           
	  -- signal remains asserted until the burst write is accepted by the slave                                  
	  process(M_AXI_ACLK)                                                                                        
	  begin                                                                                                      
	    if (rising_edge (M_AXI_ACLK)) then                                                                       
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then                                                                         
	        burst_write_active <= '0';                                                                           
	                                                                                                             
	       --The burst_write_active is asserted when a write burst transaction is initiated                      
	      else                                                                                                   
	        if (start_single_burst_write = '1') then                                                             
	          burst_write_active <= '1'; 
	        elsif (M_AXI_BVALID = '1' and axi_bready = '1') then                                                 
	          burst_write_active <= '0';                                                                         
	        end if;                                                                                              
	      end if;                                                                                                
	    end if;                                                                                                  
	  end process;       
                                                                                                             
	 -- Check for last write completion.                                                                         
	                                                                                                             
	 -- This logic is to qualify the last write count with the final write                                       
	 -- response. This demonstrates how to confirm that a write has been                                         
	 -- committed.                                                                                               
	                                                                                                             
	  process(M_AXI_ACLK)                                                                                        
	  begin                                                                                                      
	    if (rising_edge (M_AXI_ACLK)) then                                                                       
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1' or start_single_burst_write = '1') then                                                                         
	       writes_done <= '0';                                                                                   
	      --The reads_done should be associated with a rready response                                           
	      --elsif (M_AXI_RVALID && axi_rready && (read_burst_counter == {(C_NO_BURSTS_REQ-1){1}}) && axi_rlast)  
	      else                                                                                                   
	        --if (M_AXI_BVALID = '1' and (write_burst_counter(C_NO_BURSTS_REQ) = '1') and axi_bready = '1') then   
	        if (M_AXI_BVALID = '1' and axi_bready = '1') then
	          writes_done <= '1';     
	        end if;                                                                                              
	      end if;                                                                                                
	    end if;                                                                                                  
	  end process;                                                                                               
	                                                                                                             
	  -- burst_read_active signal is asserted when there is a burst write transaction                            
	  -- is initiated by the assertion of start_single_burst_write. start_single_burst_read                      
	  -- signal remains asserted until the burst read is accepted by the master                                  
	  process(M_AXI_ACLK)                                                                                        
	  begin                                                                                                      
	    if (rising_edge (M_AXI_ACLK)) then                                                                       
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then                                                                         
	        burst_read_active <= '0';                                                                            
	                                                                                                             
	       --The burst_write_active is asserted when a write burst transaction is initiated                      
	      else                                                                                                   
	        if (start_single_burst_read = '1')then                                                               
	          burst_read_active <= '1';
	        elsif (M_AXI_RVALID = '1' and axi_rready = '1' and M_AXI_RLAST = '1') then                           
	          burst_read_active <= '0';   
	        end if;                                                                                              
	      end if;                                                                                                
	    end if;                                                                                                  
	  end process;    
	                                                                                             
	                                                                                                             
	 -- Check for last read completion.                                                                          
	                                                                                                             
	 -- This logic is to qualify the last read count with the final read                                         
	 -- response. This demonstrates how to confirm that a read has been                                          
	 -- committed.                                                                                               
	                                                                                                             
	  process(M_AXI_ACLK)                                                                                        
	  begin                                                                                                      
	    if (rising_edge (M_AXI_ACLK)) then                                                                       
	      if (M_AXI_ARESETN = '0' or init_txn_pulse = '1' or start_single_burst_read = '1') then
	        reads_done <= '0';                                                                                   
	        --The reads_done should be associated with a rready response                                         
	        --elsif (M_AXI_RVALID && axi_rready && (read_burst_counter == {(C_NO_BURSTS_REQ-1){1}}) && axi_rlast)
	      else                                                                                                   
	        if (M_AXI_RVALID = '1' and axi_rready = '1' and M_AXI_RLAST = '1') then
	          reads_done <= '1';                                                                                 
	        end if;                                                                                              
	      end if;                                                                                                
	    end if;                                                                                                  
	  end process;                                                                                               

	-- Add user logic here
	process(M_AXI_ACLK)
	begin
	   if (rising_edge(M_AXI_ACLK)) then
		if (M_AXI_ARESETN = '0' or init_txn_pulse = '1') then
		   RDMem <= (others => (others => '0'));
	   	else
	   	   RDMem(RDMemAddr) <= M_AXI_RDATA;
	   	   DIADI <= M_AXI_RDATA;
		end if;
	   end if;
	end process;
 
    process(M_AXI_ACLK)
    begin
      if (rising_edge (M_AXI_ACLK)) then
         if (M_AXI_ARESETN = '0') then
            reg_src_addr <= x"0000_0000";
            burst_trans_len <= x"0000_0000";
            reg_tgt_addr <= x"0000_0000";
         else 
            reg_src_addr <= M_SRC_ADDR;
            burst_trans_len <= std_logic_vector(unsigned(M_SRC_LEN) / (C_M_AXI_BURST_LEN * C_M_AXI_DATA_WIDTH / 8)) ;
            reg_tgt_addr <= M_TGT_ADDR;
         end if;
      end if;
    end process;
	-- User logic ends

end implementation;
