-- Trade Agreement: Arakkoa Outcasts (item 128294, spell 189205).
-- Using it raises the garrison cache cap from 500 to 1000 resources.

DELETE FROM `spell_script_names` WHERE `spell_id` = 189205;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(189205, 'spell_garrison_trade_agreement_arakkoa');
