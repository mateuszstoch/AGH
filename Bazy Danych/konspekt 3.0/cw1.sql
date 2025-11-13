--zadanie 1
select p.ProductName ,p.UnitPrice , s.CompanyName from Products p 
inner join Suppliers s on p.SupplierID = s.SupplierID 
where p.UnitPrice BETWEEN 20 and 30

--zadanie 2
select p.ProductName,p.UnitsInStock  from Products p 
inner join Suppliers s on p.SupplierID = s.SupplierID 
where s.CompanyName = 'Tokyo Traders'

--zadanie 3
select c.CustomerID from customers c left outer join orders o 
on c.CustomerID = o.CustomerID and year(o.OrderDate) = '1997' where orderdate is null

--zadanie 4
select p.ProductName ,s.Phone  from Products p 
inner join Suppliers s on p.SupplierID  = s.SupplierID where p.UnitsInStock <= 0

--zadanie 5
select o.OrderID,o.OrderDate, c.CompanyName, c.Phone  from Orders o 
inner join Customers c on c.CustomerID  = o.CustomerID 
where year(o.OrderDate) = '1997' and month(o.OrderDate) = '3'