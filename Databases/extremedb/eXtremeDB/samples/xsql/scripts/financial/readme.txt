This directory "samples/xsql/scripts/financial" contains 
sample SQL scripts, command files (shell scripts) and data files 
that are intended to facilitate running examples that demonstrate 
the xsql utility. These files and the example SQL operations they 
demonstrate are fully described in chapter 3 of the eXtremeSQL User 
Guide. 

Following is a brief description of the files and their usage:

Command files:
	xsql.bat (xsql.sh) Launch xsql with argument list
	x.bat (x.sh)			Run query examples 1 thru 15
	f.bat (x.sh)			Run simple statistical queries
	g.bat (x.sh)			Run aggregate query examples

SQL scripts (inserts and queries):
	quote.sql				Define sample table Quote for samples
	insert.sql				Insert sample data into Quote table
	count.sql				Simple query to display record count
	account.sql				Define and insert into Account table 
	IBM-q1-2013.sql			Define and insert into Quote table
	query.sql				Complex queries on imported trade data
	simple.sql				Define simple table for sequence samples
	strings.sql				Define and insert into strings table
	top_close.sql			Complex query to find top close value
	x3.sql - x15.sql		Example queries 1 thru 15
	f1.sql - f5.sql			Statistical function queries 1 thru 5
	g6.sql - g11.sql		Statistical aggregate queries 6 thru 11

Sample data files:

Sample data in CSV format:
	account.csv				Sample for import into Account table 
	IBM-q1-2013.csv			Sample data for examples 3 thru 15
