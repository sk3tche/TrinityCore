ALTER TABLE `worldstates`
ROW_FORMAT=DEFAULT,
CHANGE `entry` `entry` INT(10) UNSIGNED DEFAULT '0' NOT NULL,
CHANGE `value` `value` INT(10) UNSIGNED DEFAULT '0' NOT NULL,
CHANGE `comment` `comment` TINYTEXT;