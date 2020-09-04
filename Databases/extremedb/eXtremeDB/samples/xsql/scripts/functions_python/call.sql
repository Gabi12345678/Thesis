--
-- Create procedure which takes an integer, an array and a string. It will perform some operations on arguments and return back a string
--

create function mycall(i int, a array(int), s string) returns string in 'python' as '
    var = 0
    for v in a:
        var += v
    var *= i
    return s + str(var)
';
   
    
select mycall(2, [3,4,5], 'result: ');
    
