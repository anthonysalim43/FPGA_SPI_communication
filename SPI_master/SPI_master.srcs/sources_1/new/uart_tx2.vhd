library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity uart_tx2 is


    generic (
        CLOCK_FREQ : natural := 250000000; -- Default to 12 MHz
        BAUD_RATE  : natural := 10000    -- Default to 9600 baud
    );
    
    
Port ( clk_generated : in STD_LOGIC;
       data_in : in STD_LOGIC_VECTOR(7 downto 0);
       en : in STD_LOGIC;
       busy_out : out STD_LOGIC;
       tx_out : out STD_LOGIC;
       data_ready_uart : in std_logic
       
       );
end uart_tx2;

architecture Behavioral of uart_tx2 is

    constant clks_per_bit : natural := CLOCK_FREQ / BAUD_RATE; -- 12 MHz / 9600 baud = 1250 clk_cycle// natural mean that only positive 
        
    signal cnt : integer range 0 to clks_per_bit-1 := 0;
    signal busy : STD_LOGIC := '1'; -- For stability ? 
    signal bitcnt : STD_LOGIC_VECTOR(7 downto 0) := (others => '0');
    signal en_old : STD_LOGIC := '0';

begin

    busy_out <= busy;

process(clk_generated) begin
if rising_edge(clk_generated) then

    -- Logic
    if busy = '0' then
        if en = '1' then
            -- Begin transaction        
            cnt <= clks_per_bit-1; -- Hold value for whole period
            en_old <= '1';
            busy <= '1';
            tx_out <= '0';
            bitcnt <= X"00";
        else
            tx_out <= '1';
        end if;
    
    else -- Busy=1. Continuing transaction
       
       
       if  data_ready_uart ='1' then
       
            if bitcnt = 0 then -- Keep transmitting start bit
                tx_out <= '0';
            elsif bitcnt >= 1 and bitcnt < 9 then -- Transmit corresponding data bit
                tx_out <= data_in(conv_integer(bitcnt)-1);
            else -- End of transmission
                tx_out <= '1'; 
            end if;
            
            if cnt = 0 then -- Only if ready to change to the next bit, change index
                if bitcnt < 9 then -- Still have bits to transmit
                    bitcnt <= bitcnt + 1; 
                    if bitcnt < 8 then
                    cnt <= clks_per_bit-1; -- Reset the counter
                    else
                    cnt <= clks_per_bit-100; -- Reset the counter
                    end if;
                else -- No more bits to transmit
                    busy <= '0';
                    bitcnt <= X"00";
                end if;
    
            else -- if cnt not equal to 0, we decrement it by 1 , and we do this for every clock cycle till it is 0, we hold the value of the Bit in TX until it is 0 (and cnt depend on the baud rate , higher baud rate=> lower max value of cnt)
                cnt <= cnt - 1;
            end if;
        end if;
    end if;
    
end if;
end process;
    
end Behavioral;
