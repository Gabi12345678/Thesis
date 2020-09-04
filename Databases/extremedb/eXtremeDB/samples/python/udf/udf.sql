create function psum(a integer,b integer) returns integer in 'python' as '
  import sys

  exdb.init_runtime(skip_load=True)
  print exdb.get_runtime_info()
#  print "Function psum(%s,%s) in module %s" %(a,b, __name__)
  print sys.modules["exdb"]
  print "current session: %s" % current_session
  print "current runtime: %s" % current_runtime

  return a+b
';

select psum(2,3);
