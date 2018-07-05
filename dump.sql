drop database if exists `counter`;
create database `counter`;
use counter;
drop table if exists `nextid`;
CREATE TABLE `nextid` (
  `id` serial not null,
  `ts` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);
lock tables `nextid` write;
insert into nextid() values();
unlock tables;