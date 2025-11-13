--zadanie 1
select top 1 s.CompanyName, count(*) as activity from Suppliers s 
right join Orders o on o.ShipVia = s.SupplierID 
where year(o.ShippedDate)= 1997
GROUP by s.SupplierID,s.CompanyName order by activity desc

--zadanie 2
select o.OrderID ,o.OrderDate ,c.CompanyName ,sum(round(od.UnitPrice *(1-od.Discount)*od.Quantity,2)) as Cena
from Orders o 
inner join [Order Details] od on od.OrderID = o.OrderID 
inner join Customers c on o.CustomerID = c.CustomerID 
group by o.OrderID, o.OrderDate ,c.CompanyName ,c.CustomerID  
order by Cena desc

--zadanie 3
select o.OrderID ,o.OrderDate ,c.CompanyName ,sum(round(od.UnitPrice *(1-od.Discount)*od.Quantity,2))+o.Freight  as Cena
from Orders o 
inner join [Order Details] od on od.OrderID = o.OrderID 
inner join Customers c on o.CustomerID = c.CustomerID 
group by o.OrderID, o.OrderDate ,c.CompanyName ,c.CustomerID  ,o.Freight 
order by Cena desc

-- sprawdzenie poprawnosci 
select * from Orders o where o.OrderID =10865