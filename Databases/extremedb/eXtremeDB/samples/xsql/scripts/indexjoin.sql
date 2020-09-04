create table A (
    a int,
    b int, 
    c int
);
create index idx on A (a,b);
explain select * from A a1, A a2 where a1.a=a2.a and a1.b=a2.b;
explain select * from A a1, A a2 where a1.a=a2.a and a1.b=a2.b and a1.c = 54;
explain select * from A a1, A a2 where a1.a=a2.a and a1.b=a2.b and a2.c = 54;

