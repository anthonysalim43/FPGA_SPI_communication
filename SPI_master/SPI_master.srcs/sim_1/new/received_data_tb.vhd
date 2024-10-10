LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY received_data_back2_tb IS
END received_data_back2_tb;

ARCHITECTURE behavior OF received_data_back2_tb IS 

    -- Component Declaration for the Unit Under Test (UUT)
    COMPONENT received_data_back2
    PORT(
        clk : IN  std_logic;
        reset : IN  std_logic;
        button : IN  std_logic;
        miso : IN  std_logic;
        sclk : OUT  std_logic;
        ss_n : OUT  std_logic;
        mosi : OUT  std_logic;
        busy : OUT  std_logic;
        led2 : OUT  std_logic;
        led3 : OUT  std_logic;
        led4 : OUT  std_logic
    );
    END COMPONENT;

    -- Inputs
    signal clk : std_logic := '0';
    signal reset : std_logic := '0';
    signal button : std_logic := '0';
    signal miso : std_logic := '0';

    -- Outputs
    signal sclk : std_logic;
    signal ss_n : std_logic;
    signal mosi : std_logic;
    signal busy : std_logic;
    signal led2 : std_logic;
    signal led3 : std_logic;
    signal led4 : std_logic;

    -- Clock period definitions
    constant clk_period : time := 10 ns;

BEGIN

    -- Instantiate the Unit Under Test (UUT)
    uut: received_data_back2 PORT MAP (
        clk => clk,
        reset => reset,
        button => button,
        miso => miso,
        sclk => sclk,
        ss_n => ss_n,
        mosi => mosi,
        busy => busy,
        led2 => led2,
        led3 => led3,
        led4 => led4
    );

    -- Clock process definitions
    clk_process :process
    begin
        clk <= '0';
        wait for clk_period/2;
        clk <= '1';
        wait for clk_period/2;
    end process;

    -- Stimulus process
    stim_proc: process
    begin       
        -- Reset the FPGA
        reset <= '1';
        wait for 20 ns;
        reset <= '0';     
        wait for 20 ns;

        -- Simulate button press to start SPI communication
        button <= '1';
        wait for 20 ns;
        button <= '0';
        
        -- Simulating MISO input, shifting in 0xA5
        miso <= '1'; -- MSB First
        wait for 100 ns;
        miso <= '0';
        wait for 100 ns;
        miso <= '1';
        wait for 100 ns;
        miso <= '0';
        wait for 100 ns;
        miso <= '0';
        wait for 100 ns;
        miso <= '1';
        wait for 100 ns;
        miso <= '0';
        wait for 100 ns;
        miso <= '1'; -- LSB
        wait for 100 ns;

        -- Wait for the process to complete
        wait for 500 ns;

        -- Add more test scenarios as needed

        wait; -- Will stop the simulation
    end process;

END behavior;
