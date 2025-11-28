--zadanie 1

select c.CompanyName, c.Address  from Customers c where c.CustomerID not in 
(select o.CustomerID  from Orders o where year(o.OrderDate) = 1997 and o.CustomerID  = c.CustomerID)

--zadanie 2

select c.CompanyName, c.Phone from Customers c where c.CustomerID in 
(select o.CustomerID  from Orders o join Shippers s on s.ShipperID  = o.ShipVia 
where year(o.OrderDate) = 1997 and o.CustomerID  = c.CustomerID and s.CompanyName = 'United Package')

--zadanie 3

select c.CompanyName, c.Phone from Customers c where c.CustomerID not in 
(select o.CustomerID  from Orders o join Shippers s on s.ShipperID  = o.ShipVia 
where year(o.OrderDate) = 1997 and o.CustomerID  = c.CustomerID and s.CompanyName = 'United Package')


--zadanie 4

select c.CompanyName, c.Phone from Customers c where c.CustomerID in 
(select o.CustomerID  from Orders o join [Order Details] od on od.OrderID = o.OrderID 
	join Products p on p.ProductID  = od.ProductID 
	join Categories c2 on c2.CategoryID = p.CategoryID 
	where c2.CategoryName = 'Confections')

--zadanie 5

select c.CompanyName, c.Phone from Customers c where c.CustomerID not in 
(select o.CustomerID  from Orders o join [Order Details] od on od.OrderID = o.OrderID 
	join Products p on p.ProductID  = od.ProductID 
	join Categories c2 on c2.CategoryID = p.CategoryID 
	where c2.CategoryName = 'Confections')
	
	
--zadanie 6

select c.CompanyName, c.Phone from Customers c where c.CustomerID not in 
(select o.CustomerID  from Orders o join [Order Details] od on od.OrderID = o.OrderID 
	join Products p on p.ProductID  = od.ProductID 
	join Categories c2 on c2.CategoryID = p.CategoryID 
	where c2.CategoryName = 'Confections' and year(o.OrderDate) = 1997)
	