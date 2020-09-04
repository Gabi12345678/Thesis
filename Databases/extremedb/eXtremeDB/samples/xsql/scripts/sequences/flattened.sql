create table DataSeq (Symbol varchar primary key, tPrice sequence(double));
insert into DataSeq values ('AAA', [1,2,3]);
insert into DataSeq values ('BBB', [4,5,6]);
select flattened Symbol, seq_max(tPrice) from DataSeq;
select flattened Symbol, tPrice from DataSeq;
