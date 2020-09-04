select seq_search(day, 20130101, 20130101) as t, seq_filter_pos(volume@t > 10.0) as p, volume@p from Quote where symbol='SYM0';
select seq_search(day, 20130101, 20130101) as t, seq_filter_pos(volume@t > 20.0) as p, volume@p from Quote where symbol='SYM0';
select seq_search(day, 20140801, 20140901) as t, seq_filter_pos(volume@t > 10.0) as p, volume@p from Quote where symbol='SYM0';
select seq_search(day, 20140801, 20140901) as t, seq_materialize(seq_filter_pos(volume@t > 10.0))  as p, volume@p from Quote where symbol='SYM0';
select seq_search(day, 20140801, 20140801) as t, seq_materialize(seq_filter_pos(volume@t > 10.0))  as p, volume@p from Quote where symbol='SYM0';
select !seq_search(day, 20130101, 20130101) as t, !seq_filter_pos(volume@t > 10.0) : p, volume@p from Quote where symbol='SYM0';
select !seq_search(day, 20130101, 20130101) as t, !seq_filter_pos(volume@t > 10.0) : p, seq_last_real(close@p), seq_first_real(close@p) from Quote where symbol='SYM0';
