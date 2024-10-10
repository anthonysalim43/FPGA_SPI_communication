library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity master_tb is
    -- No ports in a testbench
end master_tb;

architecture Behavioral of master_tb is

    -- Component declaration for the Unit Under Test (UUT)
    component spi_master
        Port ( clk         : in STD_LOGIC;
               reset       : in STD_LOGIC;
               button      : in STD_LOGIC;
               sclk        : buffer STD_LOGIC;
               ss_n        : out STD_LOGIC;
               mosi        : out STD_LOGIC;
               miso        : in  STD_LOGIC;
               led         : out STD_LOGIC;
               bit_counter_dbg : out STD_LOGIC_VECTOR 
             );
    end component;

    -- Testbench signals
    signal clk_tb         : STD_LOGIC := '0';
    signal reset_tb       : STD_LOGIC := '0';
    signal button_tb      : STD_LOGIC := '0';
    signal sclk_tb        : STD_LOGIC;
    signal ss_n_tb        : STD_LOGIC;
    signal mosi_tb        : STD_LOGIC;
    signal miso_tb        : STD_LOGIC := '0';  -- Slave's response (usually not relevant for master test)
    signal led_tb         : STD_LOGIC;
    signal bit_counter_dbg_tb : STD_LOGIC_VECTOR(2 downto 0); -- Testbench signal to monitor bit counter
    -- Clock period definition
    constant clk_period : time := 10 ns;

begin
    -- Instantiate the Unit Under Test (UUT)
    uut: spi_master
        Port map (
            clk     => clk_tb,
            reset   => reset_tb,
            button  => button_tb,
            sclk    => sclk_tb,
            ss_n    => ss_n_tb,
            mosi    => mosi_tb,
            miso    => miso_tb,  -- MISO signal, not used here but can be extended in future
            led     => led_tb,
            bit_counter_dbg => bit_counter_dbg_tb  -- Monitor the bit counter
        );

    -- Clock process: Toggle clock every half period
    clk_process : process
    begin
        clk_tb <= '0';
        wait for clk_period / 2;
        clk_tb <= '1';
        wait for clk_period / 2;
    end process;

    -- Stimulus process: Apply reset, then test button press and release
    stimulus_process : process
    begin
        -- Apply reset
        reset_tb <= '1';
        wait for 20 ns;
        reset_tb <= '0';

        -- Wait for some time and then press the button
        wait for 50 ns;
        button_tb <= '1';
        wait for 200 ns;
        button_tb <= '0'; -- Release the button

        -- Run the simulation for enough time to observe SPI operation
        wait for 500 ns;

        -- End simulation
        wait;
    end process;

end Behavioral;
