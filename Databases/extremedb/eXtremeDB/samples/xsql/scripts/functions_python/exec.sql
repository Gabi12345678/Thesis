create table t(i integer primary key, k string);
insert into t values ([1,2], ['hello', 'world']);   

create function execfn(i integer) returns string in 'python' as '
    cursor = current_session.cursor()
    cursor.execute("select k from t where i=?", (i,))
    row = cursor.fetchone()
    cursor.close()
    return row[0]
';
    
select execfn(1);
    
