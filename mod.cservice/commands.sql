--
-- Selected TOC Entries:
--
\connect - rgh

--
-- TOC Entry ID 2 (OID 720196)
--
-- Name: commands Type: TABLE Owner: rgh
--

CREATE TABLE "commands" (
	"command_name" character varying(128) NOT NULL,
	"domain" character varying(128) NOT NULL,
	"level" integer DEFAULT 1000 NOT NULL,
	"flags" integer DEFAULT 0 NOT NULL,
	"comment" character varying(128) NOT NULL,
	"description" character varying(128) NOT NULL,
	"last_updated" integer DEFAULT (abstime(now()))::int4 NOT NULL,
	"last_updated_by" character varying(128) DEFAULT 'Unknown' NOT NULL,
	Constraint "commands_pkey" Primary Key ("command_name")
);

--
-- Data for TOC Entry ID 5 (OID 720196)
--
-- Name: commands Type: TABLE DATA Owner: rgh
--


INSERT INTO "commands" VALUES ('SHUTDOWN','ADMIN',950,0,'','Shuts down this service',1034532585,'Jeekay');
INSERT INTO "commands" VALUES ('QUOTE','ADMIN',950,0,'','Writes a string directly to the network',1034532902,'Jeekay');
INSERT INTO "commands" VALUES ('DEBUG','ADMIN',950,0,'','Contains various debug commands',1034532903,'Jeekay');
INSERT INTO "commands" VALUES ('SERVNOTICE','ADMIN',900,0,'','Notices a given target as the services server',1034532903,'Jeekay');
INSERT INTO "commands" VALUES ('SAY','ADMIN',900,0,'','Messages a given channel as the services client',1034532903,'Jeekay');
INSERT INTO "commands" VALUES ('REHASH','ADMIN',900,0,'','Rehashes various configurations from the database',1034532903,'Jeekay');
INSERT INTO "commands" VALUES ('REMIGNORE','ADMIN',100,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('FORCE','ADMIN',400,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('LOGS','ADMIN',501,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('CSUSPEND','ADMIN',600,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('REMOVEALL','ADMIN',600,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('SCAN','ADMIN',600,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('PURGE','ADMIN',650,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('NSUSPEND','ADMIN',700,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('GLOBALSUSPEND','ADMIN',750,0,'','No description yet',1034533256,'Jeekay');
INSERT INTO "commands" VALUES ('REMUSERID','ADMIN',750,0,'','No description yet',1034533256,'Jeekay');
INSERT INTO "commands" VALUES ('GLOBNOTICE','ADMIN',800,0,'','No description yet',1034533256,'Jeekay');
INSERT INTO "commands" VALUES ('SUSADMIN','ADMIN',800,0,'','No description yet',1034533256,'Jeekay');
INSERT INTO "commands" VALUES ('FORCE2','ADMIN',850,0,'','No description yet',1034533256,'Jeekay');
INSERT INTO "commands" VALUES ('CHGADMIN','ADMIN',850,0,'','No description yet',1034533256,'Jeekay');
INSERT INTO "commands" VALUES ('REGISTER','ADMIN',600,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('UPDATEDB','ADMIN',950,0,'','No description yet',1035067865,'Jeekay');
INSERT INTO "commands" VALUES ('CHANCOMMENT','ADMIN',1,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('USERCOMMENT','ADMIN',1,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('INVME','ADMIN',1,0,'','No description yet',1034533255,'Jeekay');
INSERT INTO "commands" VALUES ('GETLEVEL','ADMIN',800,0,'','Returns the level required to run a given command',1035077661,'Jeekay');
--
-- TOC Entry ID 3 (OID 720199)
--
-- Name: "commands_command_name_key" Type: INDEX Owner: rgh
--

CREATE INDEX commands_command_name_key ON commands USING btree (command_name);

--
-- TOC Entry ID 4 (OID 720200)
--
-- Name: "commands_level_key" Type: INDEX Owner: rgh
--

CREATE INDEX commands_level_key ON commands USING btree ("level");

