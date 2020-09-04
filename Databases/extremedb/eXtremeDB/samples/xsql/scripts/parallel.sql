create table  par(x integer primary key, y integer);
insert into   par values (1,10),(2,20),(3,30),(4,40),(5,50),(6,60),(7,70),(8,80);
select * from par where x parallel in [1,2,3,4,5,6];
select * from par where x parallel is not null;
select * from par where x parallel between 1 and 5;
select * from par where x parallel in (2,4,6,8);

