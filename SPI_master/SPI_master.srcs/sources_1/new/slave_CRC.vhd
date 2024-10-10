library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.numeric_std.ALL;

entity slave_CRC is
    Port ( 
        clk        : in std_logic;         -- System clock
        reset      : in std_logic;         -- Asynchronous active low reset
        miso       : in std_logic;         -- Master In Slave Out
        sclk       : in std_logic;         -- SPI clock
        ss_n       : in std_logic;         -- Slave Select (active low)
        ss_n_out   : out std_logic;        -- Debug output for ss_n
        miso_out , miso_out2  : out std_logic;        -- Debug output for miso
        sclk_out   : out std_logic;        -- Debug output for sclk
        clk_out    : out std_logic;        -- System clock debug output
        clk_out_wiz,reset_out: out std_logic;        -- Generated clock debug output
        rx         : out std_logic_vector(7 downto 0); -- Received data output
        received_data_correctly: out std_logic_vector (1 downto 0):="11";--in idle state it will be 00, if received correctly it will be 01 and wrong it will be 10 
        leds       : out std_logic_vector(3 downto 0) := x"0"  -- LED debug output
    );
end slave_CRC;

architecture behavioural of slave_CRC is
    signal state        : integer := 0;
    signal bit_counter  : integer := 0;  -- Bit counter for 8-bit transfers
    signal rx_buffer,crc_buffer    : std_logic_vector(7 downto 0) := (others => '0'); -- Receive buffer
    
    signal crc_divider  : std_logic_vector(7 downto 0) := x"07"; -- Receive buffer
    signal crc_reminder : std_logic_vector(7 downto 0) :=x"00";
    signal sclk_sync    : std_logic := '0';  -- Synchronized SPI clock signal
    signal sclk_prev    : std_logic := '0';  -- Previous state of the SPI clock for edge detection
    signal combined_data_crc :std_logic_vector(15 downto 0 ) :=x"0000";
    -- Clocking Wizard Signals
    signal clk_wiz_reset   : std_logic := '0';  -- Reset signal for Clocking Wizard
    signal clk_wiz_locked  : std_logic;         -- Indicates the Clocking Wizard is locked
    signal clk_generated   : std_logic;         -- Clock output from the Clocking Wizard

    -- Instantiate the Clocking Wizard component
    component clk_wiz_0
        port (
            clk_in1   : in std_logic;   -- Clock input to the Clocking Wizard
            reset     : in std_logic;   -- Reset signal for the Clocking Wizard
            clk_out1  : out std_logic;  -- Generated clock output
            locked    : out std_logic   -- Locked signal indicating clock stability
        );
    end component;
    
begin
    -- Instantiate the Clocking Wizard to generate a new clock frequency
    --25MHZ worked 
    clk_wiz_inst : clk_wiz_0
        port map (
            clk_in1  => clk,            -- Connect the input clock to the system clock
            reset    => reset,          -- Reset signal to the Clocking Wizard
            clk_out1 => clk_generated,  -- Generated clock from the Clocking Wizard
            locked   => clk_wiz_locked  -- Locked signal indicating clock stability
        );
        
    -- Debug outputs
    ss_n_out <= ss_n;
    --miso_out <= miso;
    sclk_out <= sclk_sync;  -- Output the synchronized SPI clock
    rx <= rx_buffer;
    clk_out <= clk;
    clk_out_wiz <= clk_generated;
   -- reset_out<=reset;
    -- Synchronize the SPI clock to the system clock domain
--    process(clk_generated, reset)
--    begin
--        if reset = '1' then
           
--        elsif rising_edge(clk_generated) then
--        end if;
--    end process;
    
    -- SPI communication process using the synchronized clock signal
    process(clk_generated, reset)
    begin
        if reset = '1' then
             sclk_prev <= '0';
            sclk_sync <= '0';
            rx_buffer <= (others => '0');
            bit_counter <= 0;
            state <= 0;
            leds<=x"0";    
            crc_reminder<=x"00";
            crc_buffer<=x"00";
            combined_data_crc<=x"0000";
            received_data_correctly<="11";
            
        elsif rising_edge(clk_generated) then
            
            sclk_prev <= sclk_sync;
            sclk_sync <= sclk;
            if state = 0 then 
            
              -- received_data_correctly<="11";  
                if ss_n = '0' then 
              --  received_data_correctly<="11";
                    state <= 1;
                end if;
                
            elsif state = 1 then  -- Detect rising edge of synchronized SPI clock
            
              --  received_data_correctly<="11";
           -- sclk_prev <= sclk_sync;
            --sclk_sync <= sclk;
                if sclk_prev = '0' and sclk_sync = '1' then
                
                    if bit_counter<8 then
                    
                           rx_buffer(7 - bit_counter) <= miso;
                        bit_counter <= bit_counter + 1;
                        miso_out <= miso;
                    elsif bit_counter>=8 and bit_counter<16 then
                    
                           crc_buffer(15 - bit_counter) <= miso;
                            bit_counter <= bit_counter + 1;
                            miso_out2<=miso;
                            
                        miso_out <= miso;
                     end if;
                    if bit_counter = 15 then
                        bit_counter <= 0;
                        state <= 2;  -- All bits received, change state
                    end if;
                end if;
                
            elsif state = 2 then
                
                -- Transfer data to LEDs or another process
               -- leds(0)<='1';
               
               --let's check the crc data 
                combined_data_crc<=rx_buffer & crc_buffer;
                
      
                 for i in 0 to combined_data_crc'high loop
                        -- Shift left and append next data bit
                       -- crc_reminder <= std_logic_vector(shift_left(unsigned(crc_reminder), 1));
                        --crc_reminder(0) <= combined_data_crc(15 - i);  -- Append bit from MSB to LSB
                        crc_reminder<=crc_reminder(7 downto 1) & combined_data_crc(15 - i);
                       
                        -- XOR with divider if MSB is '1'
                        if crc_reminder(7) = '1' then
                            crc_reminder <= crc_reminder xor crc_divider;
                        end if;
                    end loop;
                
                 
                 
                 
                if crc_reminder=x"00" then 
                    
                    received_data_correctly<="00";
                   -- leds(0)<='1';
                    leds(0) <= rx_buffer(4);
                    leds(1) <= rx_buffer(5);
                    leds(2) <= rx_buffer(6);
                    leds(3) <= rx_buffer(7);
                --  rx_buffer <= (others => '0');
                  --  sclk_prev <= '0';
                    --sclk_sync <= '0';
                    --bit_counter <= 0;
                    state <= 0;  -- Reset state to wait for the next SPI transfer
                else 
                
                    state<=1;
                    received_data_correctly<="10";
                    crc_reminder<=x"00";
                    crc_buffer<=x"00";
                    combined_data_crc<=x"0000";
                    rx_buffer<=x"00";
                    sclk_prev<='0';
                    sclk_sync<='0';
                                 
                end if;
            end if;
        end if;
    end process;

end behavioural;
