-- Motor Correction Module
-- Applique la correction PID aux commandes moteur
-- right_speed = BASE_SPEED + correction
-- left_speed = BASE_SPEED - correction

LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.numeric_std.all;

ENTITY MUX IS
PORT (
	clk : IN STD_LOGIC;
	reset_n : IN STD_LOGIC;
	
	-- Motor commands from Qsys
	motor_right_in : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
	motor_left_in : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
	
	-- Correction from PID controller
	correction_in : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
    base_speed_in : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
    start_sl : IN STD_LOGIC;
    fin_sl : IN STD_LOGIC;
    start_rot : IN STD_LOGIC;
    dir_rot : IN STD_LOGIC;
    fin_rot : IN STD_LOGIC;
	ready : IN STD_LOGIC;
	
	-- Corrected motor outputs
	motor_right_out : OUT STD_LOGIC_VECTOR(13 DOWNTO 0);
	motor_left_out : OUT STD_LOGIC_VECTOR(13 DOWNTO 0)
);
END MUX;

ARCHITECTURE rtl OF MUX IS

constant MAX_SPEED : integer := 4095; -- 12-bit PWM
signal correction_signed : signed(13 downto 0);

BEGIN

correction_signed <= signed(correction_in);

-- Apply correction around a forward base speed.
-- If position > 0 (line on right), slow right and speed up left.
process(clk, reset_n)
    variable corr : integer;
    variable right_speed : integer;
    variable left_speed : integer;
    variable base_speed : integer;
begin
    if reset_n = '0' then
        motor_right_out <= (others => '0');
        motor_left_out <= (others => '0');
    elsif rising_edge(clk) then
        if ready = '1' then
            base_speed := to_integer(unsigned(base_speed_in));
            if base_speed > MAX_SPEED then
                base_speed := MAX_SPEED;
            elsif base_speed < 0 then
                base_speed := 0;
            end if;

            if (start_rot = '1') and (fin_rot = '0') then
                if dir_rot = '0' then
                    -- Rotate left: right forward, left backward
                    motor_right_out <= '1' & '0' & std_logic_vector(to_unsigned(base_speed, 12));
                    motor_left_out  <= '1' & '1' & std_logic_vector(to_unsigned(base_speed, 12));
                else
                    -- Rotate right: right backward, left forward
                    motor_right_out <= '1' & '1' & std_logic_vector(to_unsigned(base_speed, 12));
                    motor_left_out  <= '1' & '0' & std_logic_vector(to_unsigned(base_speed, 12));
                end if;
            elsif (start_sl = '1') and (fin_sl = '0') then
                corr := to_integer(correction_signed);
                right_speed := base_speed - corr;
                left_speed := base_speed + corr;

                if right_speed < 0 then
                    right_speed := 0;
                elsif right_speed > MAX_SPEED then
                    right_speed := MAX_SPEED;
                end if;

                if left_speed < 0 then
                    left_speed := 0;
                elsif left_speed > MAX_SPEED then
                    left_speed := MAX_SPEED;
                end if;

                motor_right_out <= '1' & '0' & std_logic_vector(to_unsigned(right_speed, 12));
                motor_left_out  <= '1' & '0' & std_logic_vector(to_unsigned(left_speed, 12));
            else
                motor_right_out <= (others => '0');
                motor_left_out <= (others => '0');
            end if;
        end if;
    end if;
end process;

END rtl;
