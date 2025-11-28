--zadanie 1
select sum(od.Quantity * od.UnitPrice  * (1 - od.Discount )) as suma 
from [Order Details] od group by od.OrderID 
order by suma desc

--zadanie 2
select top 10 sum(od.Quantity * od.UnitPrice  * (1 - od.Discount )) as suma 
from [Order Details] od group by od.OrderID 
order by suma desc