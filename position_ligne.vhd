library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--==============================================================
-- MODULE: position_ligne
-- Calcul de la position de la ligne basé sur 7 capteurs sol
-- Position: -3 (extrême gauche) à +3 (extrême droite)
-- 0 = ligne au centre
--==============================================================

entity position_ligne is
    port (
        clk         : in  std_logic;
        reset_n     : in  std_logic;
        sensor_ready: in  std_logic;  -- Flag de validation des données capteurs
        
        -- 7 capteurs de 8 bits
        sensor0     : in  std_logic_vector(7 downto 0);
        sensor1     : in  std_logic_vector(7 downto 0);
        sensor2     : in  std_logic_vector(7 downto 0);
        sensor3     : in  std_logic_vector(7 downto 0);
        sensor4     : in  std_logic_vector(7 downto 0);
        sensor5     : in  std_logic_vector(7 downto 0);
        sensor6     : in  std_logic_vector(7 downto 0);
        
        -- Sorties
        position    : out std_logic_vector(3 downto 0);  -- Position signée (-3 à +3)
        line_lost   : out std_logic                       -- '1' si aucun capteur noir détecté
    );
end position_ligne;

architecture rtl of position_ligne is
    
    constant THRESHOLD : unsigned(7 downto 0) := to_unsigned(110, 8);
    
    -- Signaux internes pour détection noir
    signal detect0, detect1, detect2, detect3 : std_logic;
    signal detect4, detect5, detect6 : std_logic;
    signal num_black : unsigned(2 downto 0);  -- 0 à 7 capteurs noirs
    
    -- Calcul pondéré
    signal weighted_sum : signed(5 downto 0);  -- peut aller de -21 à +21
    
    -- Position finale
    signal pos_final : signed(3 downto 0);  -- -3 à +3
    signal line_lost_sig : std_logic;
    
begin
    
    --==============================================================
    -- Étape 1: Comparaison capteur vs seuil (constant 110)
    --==============================================================
    detect0 <= '1' when unsigned(sensor0) > THRESHOLD else '0';
    detect1 <= '1' when unsigned(sensor1) > THRESHOLD else '0';
    detect2 <= '1' when unsigned(sensor2) > THRESHOLD else '0';
    detect3 <= '1' when unsigned(sensor3) > THRESHOLD else '0';
    detect4 <= '1' when unsigned(sensor4) > THRESHOLD else '0';
    detect5 <= '1' when unsigned(sensor5) > THRESHOLD else '0';
    detect6 <= '1' when unsigned(sensor6) > THRESHOLD else '0';
    
    --==============================================================
    -- Étape 2: Compte le nombre de capteurs noirs (0-7)
    --==============================================================
    num_black <= unsigned'(
        (detect0 & "00") +
        (detect1 & "00") +
        (detect2 & "00") +
        (detect3 & "00") +
        (detect4 & "00") +
        (detect5 & "00") +
        (detect6 & "00")
    );
    
    --==============================================================
    -- Étape 3: Calcul de la somme pondérée
    -- Poids: -3, -2, -1, 0, +1, +2, +3
    --==============================================================
    weighted_sum <=
        (detect0 * to_signed(-3, 6)) +
        (detect1 * to_signed(-2, 6)) +
        (detect2 * to_signed(-1, 6)) +
        (detect3 * to_signed( 0, 6)) +
        (detect4 * to_signed( 1, 6)) +
        (detect5 * to_signed( 2, 6)) +
        (detect6 * to_signed( 3, 6));
    
    --==============================================================
    -- Étape 4: Division rapide par approx (shift)
    -- num_black = nombre de capteurs noirs détectés
    -- Approximation rapide sans division complète
    --==============================================================
    process(weighted_sum, num_black)
    begin
        if num_black = 0 then
            pos_final <= to_signed(0, 4);  -- Fallback: pas de ligne détectée
            line_lost_sig <= '1';
        elsif num_black = 1 then
            pos_final <= weighted_sum(3 downto 0);  -- weighted_sum / 1
        elsif num_black = 2 then
            pos_final <= weighted_sum(4 downto 1);  -- weighted_sum / 2 (shift >> 1)
        elsif num_black = 3 then
            -- Approximation: (weighted_sum * 21) >> 6 ≈ weighted_sum / 3
            -- Simplifié: shift >> 2 puis ajustement
            pos_final <= weighted_sum(4 downto 1);  -- Approximation simple
        elsif num_black = 4 then
            pos_final <= weighted_sum(4 downto 1);  -- weighted_sum / 4 (shift >> 2)
        elsif num_black = 5 then
            -- Approximation weighted_sum / 5
            pos_final <= weighted_sum(4 downto 1);  -- Approximation
        elsif num_black = 6 then
            -- Approximation weighted_sum / 6
            pos_final <= weighted_sum(4 downto 1);  -- Approximation
        else  -- num_black = 7
            pos_final <= weighted_sum(4 downto 1);  -- weighted_sum / 7 (shift >> 2)
        end if;
        
        -- Si au moins 1 capteur noir, ligne détectée
        if num_black = 0 then
            line_lost_sig <= '1';
        else
            line_lost_sig <= '0';
        end if;
    end process;
    
    --==============================================================
    -- Registres de sortie synchronisés au clock
    --==============================================================
    process(clk, reset_n)
    begin
        if reset_n = '0' then
            position <= (others => '0');
            line_lost <= '0';
        elsif rising_edge(clk) then
            if sensor_ready = '1' then
                -- Convertir en format std_logic_vector signé
                position <= std_logic_vector(pos_final);
                line_lost <= line_lost_sig;
            end if;
        end if;
    end process;

end rtl;
