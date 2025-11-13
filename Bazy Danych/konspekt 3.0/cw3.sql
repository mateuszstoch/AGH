--zadanie 1
SELECT p.ProductName ,p.UnitPrice, s.Address  from Products p 
inner join Categories c on p.CategoryID = c.CategoryID 
inner join Suppliers s on p.SupplierID = s.SupplierID 
where p.UnitPrice BETWEEN 20 and 30 and c.CategoryName = 'Meat/Poultry'

--zadanie 2
SELECT p.ProductName ,p.UnitPrice, s.CompanyName   from Products p 
inner join Categories c on p.CategoryID = c.CategoryID 
inner join Suppliers s on p.SupplierID = s.SupplierID 
where c.CategoryName = 'Confections'

--zadanie 3
SELECT c.CompanyName , count(o.OrderID ) from Customers c 
LEFT JOIN Orders o on o.CustomerID = c.CustomerID GROUP BY c.CompanyName, c.CustomerID 

--zadanie 4
SELECT c.CompanyName,count(o.OrderID)  from Customers c 
left JOIN Orders o on o.CustomerID = c.CustomerID 
and year(o.OrderDate)=1997 and month(o.OrderDate)=3
group by c.CompanyName, c.CustomerID 