create table market_data (
    sec integer(4),
    dt date,
    rt double,
    cc smallint null,
    mc double null,
    un tinyint null,
    primary key (dt, sec)
);
create index idx_market_data on market_data (sec, dt);
insert into market_data values (1, '2011-01-01', 1.0, 1, 1, 1);
insert into market_data values (2, '2011-01-01', 1.0, 1, 1, 1);
insert into market_data values (2, '2011-01-02', 1.0, 1, 1, 1);
insert into market_data values (2, '2011-01-03', 1.0, 1, 1, 1);
insert into market_data values (2, '2011-01-04', 1.0, 1, 1, 1);
insert into market_data values (3, '2015-12-30', 1.0, 1, 1, 1);



create table cal(cid integer(4), dt date);

insert into cal values(10001, '1994-01-04');
insert into cal values(10002, '1994-01-05');
insert into cal values(10003, '1994-01-06');
insert into cal values(10004, '1994-01-07');
insert into cal values(10005, '2015-12-30');


select cl.dt, cl.cid, mk.rt, mk.sec from cal as cl, market_data as mk where mk.dt = cl.dt;
