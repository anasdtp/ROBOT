library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--==============================================================
-- MODULE: ctl_sl (SL = Suivi de Ligne / Line Following)
-- Contrôleur PID proportionnel (P-only)
-- Convertit position de ligne en correction de moteur
--
-- Équation: output = KP * position_sig
-- KP = 900
-- position_sig: -3 à +3
-- output: -2700 à +2700
--==============================================================

entity ctl_sl is
    port (
        clk         : in  std_logic;
        reset_n     : in  std_logic;
        
        -- Input from position_ligne module
        position_in : in  std_logic_vector(3 downto 0);  -- Signed -3 to +3
        ready       : in  std_logic;                     -- Data valid flag
        start_sl    : in  std_logic;
        line_lost   : in  std_logic;
        
        -- Output correction for motors
        -- Positive = turn right (reduce left motor speed / increase right motor speed)
        -- Negative = turn left  (reduce right motor speed / increase left motor speed)
        correction  : out std_logic_vector(13 downto 0); -- Signed correction value
        fin_sl      : out std_logic;

        -- KP from Nios (12-bit signed)
        kp_in       : in  std_logic_vector(11 downto 0);

        -- KD from Nios (12-bit signed)
        kd_in       : in  std_logic_vector(11 downto 0)
    );
end ctl_sl;

architecture rtl of ctl_sl is
    
    signal position_signed : signed(3 downto 0);
    signal kp_signed : signed(11 downto 0);
    signal kd_signed : signed(11 downto 0);
    signal prev_position : signed(3 downto 0);
    signal deriv_signed : signed(4 downto 0);
    signal p_term : signed(15 downto 0);
    signal d_term : signed(15 downto 0);
    signal correction_temp : signed(13 downto 0);
    signal correction_out  : signed(13 downto 0);
    
begin
    
    -- Convert input to signed
    position_signed <= signed(position_in);
    kp_signed <= signed(kp_in);
    kd_signed <= signed(kd_in);

    deriv_signed <= resize(position_signed - prev_position, 5);
    p_term <= resize(position_signed * kp_signed, 16);
    d_term <= resize(deriv_signed * kd_signed, 16);
    
    -- PD calculation: correction = KP * position + KD * derivative
    correction_temp <= resize(p_term + d_term, 14);
    
    -- Register the output
    process(clk, reset_n)
    begin
        if reset_n = '0' then
            correction_out <= to_signed(0, 14);
            fin_sl <= '0';
            prev_position <= to_signed(0, 4);
        elsif rising_edge(clk) then
            if ready = '1' then
                prev_position <= position_signed;
                if start_sl = '1' then
                    if line_lost = '1' then
                        correction_out <= to_signed(0, 14);
                        fin_sl <= '1';
                    else
                        correction_out <= correction_temp;
                        fin_sl <= '0';
                    end if;
                else
                    correction_out <= to_signed(0, 14);
                    fin_sl <= '0';
                end if;
            end if;
        end if;
    end process;
    
    -- Convert back to std_logic_vector
    correction <= std_logic_vector(correction_out);
    
end rtl;
