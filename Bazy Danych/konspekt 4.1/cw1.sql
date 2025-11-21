-- zadanie 1 


SELECT o.OrderID, round(sum(od.Quantity * od.UnitPrice * (1- od.Discount))+
(select sum(o2.Freight)  FROM  orders o2 where o2.OrderID = o.OrderID ),2) as cena FROM Orders o
inner join [Order Details] od on o.OrderID = od.OrderID 
where o.OrderID = 10250
group by o.OrderID


-- zadanie 2

SELECT o.OrderID, round(sum(od.Quantity * od.UnitPrice * (1- od.Discount))+
(select sum(o2.Freight)  FROM  orders o2 where o2.OrderID = o.OrderID ),2) FROM Orders o
inner join [Order Details] od on o.OrderID = od.OrderID 
group by o.OrderID

-- zadanie 3

select p.ProductName , 
(Select round(max(od.UnitPrice * od.Quantity * (1-od.Discount )),2) from [Order Details] od where p.ProductID = od.ProductID)
from Products p 

-- zapytanie do sprawdzenia wyniku
select p.ProductName ,od.UnitPrice * od.Quantity * (1-od.Discount ) from [Order Details] od 
inner join Products p on p.ProductID = od.ProductID where p.ProductName ='Chai'

-- zadanie 4

select p.ProductName , 
(Select round(max(od.UnitPrice * od.Quantity * (1-od.Discount )),2) from [Order Details] od where p.ProductID = od.ProductID 
and od.OrderID IN (Select o.OrderID from orders o where year(o.OrderDate) = 1997 )) as 'Cena'
from Products p 
order by 'Cena' desc


-- zapytanie do sprawdzenia wyniku
select p.ProductName ,od.UnitPrice * od.Quantity * (1-od.Discount ),o.OrderDate  from [Order Details] od 
inner join Products p on p.ProductID = od.ProductID 
inner join orders o on o.OrderID  = od.OrderID 
where p.ProductName ='Chai'and year(o.OrderDate) = 1997