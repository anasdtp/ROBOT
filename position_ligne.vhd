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
    
    -- THRESHOLD: Ajuster selon vos capteurs
    -- Noir (ligne) = valeur HAUTE > THRESHOLD
    -- Blanc (pas ligne) = valeur BASSE < THRESHOLD
    -- Valeur seuil calibrée: 110 (match avec line_follower.c)
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
    -- Étape 1: Détection ligne noire (valeur HAUTE = noir)
    -- Match avec logique du C: sensor_value > THRESHOLD = 110
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
    process(detect0, detect1, detect2, detect3, detect4, detect5, detect6)
        variable cnt : integer range 0 to 7;
    begin
        cnt := 0;
        if detect0 = '1' then cnt := cnt + 1; end if;
        if detect1 = '1' then cnt := cnt + 1; end if;
        if detect2 = '1' then cnt := cnt + 1; end if;
        if detect3 = '1' then cnt := cnt + 1; end if;
        if detect4 = '1' then cnt := cnt + 1; end if;
        if detect5 = '1' then cnt := cnt + 1; end if;
        if detect6 = '1' then cnt := cnt + 1; end if;
        num_black <= to_unsigned(cnt, 3);
    end process;
    
    --==============================================================
    -- Étape 3 & 4: Calcul de la somme pondérée et division
    -- Poids: -3, -2, -1, 0, +1, +2, +3
    --==============================================================
    process(detect0, detect1, detect2, detect3, detect4, detect5, detect6, num_black)
        variable temp_sum : signed(5 downto 0);
    begin
        -- Calcul de la somme pondérée
        temp_sum := to_signed(0, 6);
        if detect0 = '1' then temp_sum := temp_sum + to_signed(-3, 6); end if;
        if detect1 = '1' then temp_sum := temp_sum + to_signed(-2, 6); end if;
        if detect2 = '1' then temp_sum := temp_sum + to_signed(-1, 6); end if;
        if detect3 = '1' then temp_sum := temp_sum + to_signed( 0, 6); end if;
        if detect4 = '1' then temp_sum := temp_sum + to_signed( 1, 6); end if;
        if detect5 = '1' then temp_sum := temp_sum + to_signed( 2, 6); end if;
        if detect6 = '1' then temp_sum := temp_sum + to_signed( 3, 6); end if;
        
        weighted_sum <= temp_sum;
    end process;
    
    --==============================================================
    -- Division avec shifts (plus stable en synthèse que divisions entières)
    --==============================================================
    process(weighted_sum, num_black)
        variable temp_pos : signed(3 downto 0);
    begin
        line_lost_sig <= '0';
        if num_black = "000" then
            temp_pos := to_signed(0, 4);
            line_lost_sig <= '1';
        elsif num_black = "001" then
            -- /1
            temp_pos := resize(weighted_sum, 4);
        elsif num_black = "010" then
            -- /2
            temp_pos := resize(shift_right(weighted_sum, 1), 4);
        elsif num_black = "011" then
            -- /3 ≈ /2 (approximation)
            temp_pos := resize(shift_right(weighted_sum, 1), 4);
        elsif num_black = "100" then
            -- /4
            temp_pos := resize(shift_right(weighted_sum, 2), 4);
        elsif num_black = "101" then
            -- /5 ≈ /4 (approximation)
            temp_pos := resize(shift_right(weighted_sum, 2), 4);
        elsif num_black = "110" then
            -- /6 ≈ /4 (approximation)
            temp_pos := resize(shift_right(weighted_sum, 2), 4);
        else  -- num_black = 7
            -- /7 ≈ /4 (approximation)
            temp_pos := resize(shift_right(weighted_sum, 2), 4);
        end if;
        
        -- Saturation: forcer la position entre -3 et +3
        if temp_pos > to_signed(3, 4) then
            pos_final <= to_signed(3, 4);
        elsif temp_pos < to_signed(-3, 4) then
            pos_final <= to_signed(-3, 4);
        else
            pos_final <= temp_pos;
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
