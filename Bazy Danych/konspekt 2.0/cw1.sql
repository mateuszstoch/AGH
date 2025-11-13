--zadanie 1
select count(p.ProductID  ) from Products p  
WHERE p.UnitPrice between 10 and 20  

--zadanie 2
select MAX(p.UnitPrice) from Products p where p.UnitPrice < 20

--zadanie 3
select max(p.UnitPrice),min(p.UnitPrice ),AVG(p.UnitPrice ) 
from Products p where p.QuantityPerUnit 
like '%bottle%' 

--zadanie 4
select * from Products p 
where (select avg(UnitPrice )from products) < p.UnitPrice 

--zadanie 5
select sum(od.UnitPrice * od.Quantity * (1-od.Discount )) 
from [Order Details] od   where od.OrderID = 10250