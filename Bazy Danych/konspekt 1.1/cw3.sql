-- zadanie 1
select * from Products p where p.UnitPrice not between 10 and 20

-- zadanie 2
select p.ProductName , p.UnitPrice  from Products p where p.UnitPrice between 20 and 30

-- zadanie 3
select * from Orders o where year(o.OrderDate) = 1997