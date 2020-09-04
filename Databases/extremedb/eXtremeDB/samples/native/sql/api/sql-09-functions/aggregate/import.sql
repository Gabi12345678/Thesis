insert or update into trade
  select *
  from 
       foreign table(path='data.csv', skip=1) as trade;
  
