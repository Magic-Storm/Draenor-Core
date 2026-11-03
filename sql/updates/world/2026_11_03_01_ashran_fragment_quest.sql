-- Uncovering the Artifact Fragments (36133) asked for 50 of currency 390.
-- Retail asks for 5 Artifact Fragments (currency 944). Stored counts use precision 100.
UPDATE `quest_template_objective`
SET `type` = 17, `objectid` = 944, `amount` = 500
WHERE `questid` = 36133 AND `type` = 4 AND `objectid` = 390;
