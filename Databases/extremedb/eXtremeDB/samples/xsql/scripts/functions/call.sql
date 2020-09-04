--
-- Create procedure which takes an integer, an array and a string. It will perform some operations on arguments and return back a string
--

create function mycall(i int, a array(int), s string) returns string in 'lua' as '
    local var = 0
    for k,v in ipairs(a) do
        var = var + v
    end
    var = var * i
    return s .. var
end
';
   
    
select mycall(2, [3,4,5], 'result: ');
    
