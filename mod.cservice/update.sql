-- ForceLog table
-- 23/1/2001 ULtimaTe_
--
-- ChangeLog
-- 15/4/2001 ULtimaTe_
-- removed the linked ID's

CREATE TABLE forcelog (

        channelname TEXT NOT NULL,
        username TEXT NOT NULL,
        message TEXT,
        ts INT4 NOT NULL
);

