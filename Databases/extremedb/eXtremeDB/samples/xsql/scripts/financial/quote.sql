create table Quote(symbol char(21) primary key, day sequence(unsigned(4) asc), 
low sequence(float), high sequence(float), open sequence(float), close sequence(float), volume sequence(unsigned(4)));