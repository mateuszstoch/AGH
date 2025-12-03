--zadanie 1

select m.firstname, m.lastname, count(l.member_no), iif(m.member_no in (select a.member_no from adult a), 'Dorosły', 'Dziecko')
from [member] m join loanhist l on m.member_no = l.member_no and year(l.in_date) = 2001 
GROUP by m.member_no ,m.lastname ,m.firstname having count(l.member_no) >= 1 

-- zapytanie sprawdzajace 
select * from loanhist l where year(l.in_date) = 2001 and l.member_no = 9

-- zadanie 2

select p.ProductName, c.CategoryName, o.ShipName from Products p 
join [Order Details] od on p.ProductID = od.ProductID
join orders o on od.OrderID = o.OrderID and o.OrderDate BETWEEN '1997.02.20' and '1997.02.25'
join Categories c on p.CategoryID = c.CategoryID and c.CategoryName != 'Beverages'

-- zapytanie sprawdzajace 
select * from Orders o join [Order Details] od on o.OrderID = od.OrderID and od.ProductID = 28 and year(o.OrderDate) = 1997 and month(o.OrderDate) = 2



-- zadanie 3
-- 1 sposob
select distinct c.CompanyName ,c.Phone  from Customers c join Orders o on c.CustomerID = o.CustomerID 
where c.CustomerID not in 
(Select o.CustomerID from Orders o join Shippers s  on s.ShipperID = o.ShipVia where year(o.ShippedDate) = 1997 and s.CompanyName = 'United Package')
order by c.CompanyName 

-- 2 sposob
select distinct c.CompanyName ,c.Phone  from Customers c join Orders o on c.CustomerID = o.CustomerID 
except 
select distinct c.CompanyName ,c.Phone  from Customers c join Orders o on c.CustomerID = o.CustomerID 
join Shippers s on s.ShipperID  = o.ShipVia 
where year(o.ShippedDate) = 1997 and s.CompanyName = 'United Package'
order by c.CompanyName 

-- zapytanie sprawdzajace 
Select distinct c.CompanyName ,s.CompanyName from Orders o 
join Shippers s  on s.ShipperID = o.ShipVia 
join Customers c on c.CustomerID = o.CustomerID 
where year(o.ShippedDate) = 1997 and c.CompanyName = 'Drachenblut Delikatessen'
