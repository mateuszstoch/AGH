--zadanie 1

select c.CompanyName, 
(Select round(sum(od.Quantity * od.UnitPrice * (1-od.Discount )),2) from Orders o 
join [Order Details] od on od.OrderID = o.OrderID 
where year(o.OrderDate)=1996 and c.CustomerID  = o.CustomerID ) from Customers c 


-- zapytanie do sprawdzenia wyniku
select c.CompanyName, od.Quantity * od.UnitPrice * (1-od.Discount),o.orderID  from Customers c 
join Orders o on o.CustomerID = c.CustomerID 
join [Order Details] od on od.OrderID  = o.OrderID 
where year(o.OrderDate)  = 1996 and c.CompanyName = 'Ana Trujillo Emparedados y helados'

--zadanie 2 

select c.CompanyName, 
round((Select sum(od.Quantity * od.UnitPrice * (1-od.Discount )) from Orders o 
join [Order Details] od on od.OrderID = o.OrderID 
where year(o.OrderDate) = 1996 and c.CustomerID  = o.CustomerID )+(select sum(o.Freight)  FROM  orders o 
where c.CustomerID = o.CustomerID and year(o.OrderDate) = 1996),2) from Customers c 

-- zapytanie do sprawdzenia wyniku
select c.CompanyName, o.Freight, o.OrderID  from Customers c 
join Orders o on o.CustomerID = c.CustomerID 
where year(o.OrderDate)  = 1996 and c.CompanyName = 'Ana Trujillo Emparedados y helados'

--zadanie 3

select c.CompanyName, 
(Select round(max(od.Quantity * od.UnitPrice * (1-od.Discount )),2) from Orders o 
join [Order Details] od on od.OrderID = o.OrderID 
where year(o.OrderDate)=1997 and c.CustomerID  = o.CustomerID ) from Customers c 
order by 2 desc


-- zapytanie do sprawdzenia wyniku
select c.CompanyName, round(od.Quantity * od.UnitPrice * (1-od.Discount),2),o.orderID  from Customers c 
join Orders o on o.CustomerID = c.CustomerID 
join [Order Details] od on od.OrderID  = o.OrderID 
where year(o.OrderDate)  = 1997 and c.CompanyName = 'Simons bistro'

