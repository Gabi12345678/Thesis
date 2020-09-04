create function twice(x integer) returns integer in 'lua' as 'return x+x end';
select twice(2);
create function sum_int_array(a array(integer)) returns integer in 'lua' as 'local s = 0 ; for i,v in ipairs(a) do s = s + v ; end ; return s ; end';
select sum_int_array([1,2,3,4,5]);
create function dbl_int_array(a array(integer)) returns array(integer) in 'lua' as 'local ret = {} ; for i,v in ipairs(a) do ret[i] = 2*v ; end ; return ret ; end';
select dbl_int_array([1,2,3,4,5]);
