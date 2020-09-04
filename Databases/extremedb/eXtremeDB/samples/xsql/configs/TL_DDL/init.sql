CREATE TABLE Employee (name string, dept_no unsigned(2));
CREATE UNIQUE INDEX Iname ON Employee(name) USING btree;
CREATE INDEX Idept ON Employee(dept_no) USING btree;
INSERT INTO Employee(name, dept_no) VALUES (['Luke Skywalker', 'Han Solo', 'Darth Vader'], [1,1,2]);
