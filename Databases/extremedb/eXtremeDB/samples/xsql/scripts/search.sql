insert into Point values (0,0);
insert into Point values (0,1);
insert into Point values (0,2);
insert into Point values (1,0);
insert into Point values (1,1);
insert into Point values (1,2);
insert into Point values (2,0);
insert into Point values (2,1);
insert into Point values (2,2);

insert into DPoint values (0,0);
insert into DPoint values (0,1);
insert into DPoint values (0,2);
insert into DPoint values (1,0);
insert into DPoint values (1,1);
insert into DPoint values (1,2);
insert into DPoint values (2,0);
insert into DPoint values (2,1);
insert into DPoint values (2,2);

trace on

select * from Point where x > 1 and y > 1;
select * from Point where x > 1;
select * from Point where x >= 1;
select * from Point where x < 1;
select * from Point where x <= 1;

select * from Point where x > 1 and y > 1 order by x;
select * from Point where x > 1 order by x;
select * from Point where x >= 1 order by x;
select * from Point where x < 1 order by x;
select * from Point where x <= 1 order by x;

select * from Point where x > 1 and y > 1 order by x desc;
select * from Point where x > 1 order by x desc;
select * from Point where x >= 1 order by x desc;
select * from Point where x < 1 order by x desc;
select * from Point where x <= 1 order by x desc;

select * from DPoint where x > 1 and y > 1;
select * from DPoint where x > 1;
select * from DPoint where x >= 1;
select * from DPoint where x < 1;
select * from DPoint where x <= 1;

select * from DPoint where x > 1 and y > 1 order by x;
select * from DPoint where x > 1 order by x;
select * from DPoint where x >= 1 order by x;
select * from DPoint where x < 1 order by x;
select * from DPoint where x <= 1 order by x;

select * from DPoint where x > 1 and y > 1 order by x desc;
select * from DPoint where x > 1 order by x desc;
select * from DPoint where x >= 1 order by x desc;
select * from DPoint where x < 1 order by x desc;
select * from DPoint where x <= 1 order by x desc;

explain select * from Point where x > 1 and y > 1;
explain select * from Point where x > 1;
explain select * from Point where x >= 1;
explain select * from Point where x < 1;
explain select * from Point where x <= 1;

explain select * from Point where x > 1 and y > 1 order by x;
explain select * from Point where x > 1 order by x;
explain select * from Point where x >= 1 order by x;
explain select * from Point where x < 1 order by x;
explain select * from Point where x <= 1 order by x;

explain select * from Point where x > 1 and y > 1 order by x desc;
explain select * from Point where x > 1 order by x desc;
explain select * from Point where x >= 1 order by x desc;
explain select * from Point where x < 1 order by x desc;
explain select * from Point where x <= 1 order by x desc;

explain select * from DPoint where x > 1 and y > 1;
explain select * from DPoint where x > 1;
explain select * from DPoint where x >= 1;
explain select * from DPoint where x < 1;
explain select * from DPoint where x <= 1;

explain select * from DPoint where x > 1 and y > 1 order by x;
explain select * from DPoint where x > 1 order by x;
explain select * from DPoint where x >= 1 order by x;
explain select * from DPoint where x < 1 order by x;
explain select * from DPoint where x <= 1 order by x;

explain select * from DPoint where x > 1 and y > 1 order by x desc;
explain select * from DPoint where x > 1 order by x desc;
explain select * from DPoint where x >= 1 order by x desc;
explain select * from DPoint where x < 1 order by x desc;
explain select * from DPoint where x <= 1 order by x desc;

