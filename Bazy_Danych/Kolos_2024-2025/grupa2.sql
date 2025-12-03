--zadanie 1
select c.CompanyName, e.FirstName ,e.LastName from Customers c 
join Orders o on c.CustomerID  = o.CustomerID 
join Employees e on o.EmployeeID = e.EmployeeID
group by c.CustomerID, c.CompanyName, e.FirstName ,e.LastName
having (Select count(distinct o.EmployeeID) from orders o where o.CustomerID = c.CustomerID) = 1

--zapytanie sprawdzajace
Select c.CompanyName  ,count(distinct o.EmployeeID) from orders o
join Customers c on c.CustomerID  = o.CustomerID 
group by o.CustomerID,c.CompanyName 

--zadanie 2
-- left join jest po to zeby bylo widac ludzi bez zamowien
SELECT e.EmployeeID,  COUNT(DISTINCT o.OrderID), 
    ISNULL(ROUND(SUM(od.Quantity * od.UnitPrice * (1 - od.Discount)) + SUM(DISTINCT o.Freight), 2), 0)
FROM Employees e
LEFT JOIN Orders o ON e.EmployeeID = o.EmployeeID AND YEAR(o.OrderDate) = 1997 AND MONTH(o.OrderDate) = 2
LEFT JOIN [Order Details] od ON o.OrderID = od.OrderID 
GROUP BY e.EmployeeID

--zapytania sprawdzajace
select e.EmployeeID , count(*), round(sum(od.Quantity * od.UnitPrice * (1-od.Discount )) ,2) from Employees e 
join Orders o on e.EmployeeID = o.EmployeeID and year(o.OrderDate) = 1997 and month(o.OrderDate ) = 2
join [Order Details] od on o.OrderID = od.OrderID 
group by e.EmployeeID 

select * from Orders o where o.EmployeeID = 5 and year(o.OrderDate) = 1997 and month(o.OrderDate ) = 2

select o.EmployeeID ,sum(o.Freight) from Orders o where year(o.OrderDate) = 1997 and month(o.OrderDate ) = 2 group by o.EmployeeID 

-- zadanie 3

select m.member_no ,m.firstname ,m.lastname , (select count(*) from juvenile j where j.adult_member_no = m.member_no),
((select count(*) from loan l 
	where l.member_no = m.member_no or l.member_no in 
	(select j.member_no from juvenile j where j.adult_member_no = m.member_no))
+(select count(*) from loanhist l 
where l.member_no = m.member_no or l.member_no in (select j.member_no from juvenile j where j.adult_member_no = m.member_no)))
from adult a 
join [member] m on a.member_no = m.member_no
group by m.member_no,m.firstname ,m.lastname 

select * from loanhist l 
where l.member_no = 3 or l.member_no in (select j.member_no  from juvenile j where j.adult_member_no = 3)
