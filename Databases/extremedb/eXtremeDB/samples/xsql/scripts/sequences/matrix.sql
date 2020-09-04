create table Securities(symbol varchar primary key, price sequence(real));
insert into Securities values ('AAA', [4.0, 4.2, 3.9, 4.3, 4.1]);
insert into Securities values ('BAB', [1, 1, 1, 1]);
insert into Securities values ('AAB', [2.0, 2.1, 2.0, 2.1, 2.2]);
insert into Securities values ('BAD', [4, 7, 5, 2]);
insert into Securities values ('AAC', [0.60, 0.59, 0.58, 0.62, 0.63]);
insert into Securities values ('BAA', [2, 2, 2, 2]);
select cov(price) from Securities where symbol like 'A%';
select cor(price) from Securities where symbol like 'A%';
