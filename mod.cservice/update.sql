-- ForceLog table
-- 23/1/2001 ULtimaTe_

CREATE TABLE forcelog (

        channel_id INT4 CONSTRAINT levels_channel_id_ref REFERENCES channels ( id ),
        user_id INT4 CONSTRAINT levels_users_id_ref REFERENCES users ( id ),
        message TEXT,
        ts INT4 NOT NULL
);

CREATE INDEX forcelog_channelID_idx ON forcelog(channel_id);
CREATE INDEX forcelog_userID_idx ON userlog(user_id);

